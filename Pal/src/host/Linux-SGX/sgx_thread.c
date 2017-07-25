/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

//#define _GNU_SOURCE
#include "pal_internal.h"
#include "sgx_internal.h"

#include <pthread.h>
#include <linux/futex.h>
#include <asm/prctl.h>

#include "sgx_enclave.h"
#include "debugger/sgx_gdb.h"

#include  "SyncUtils.h"
#include "request.h"
#include "Queue.h"

__thread struct pal_enclave * current_enclave;
__thread void * current_tcs;
__thread unsigned long debug_register;

struct _worker_params
{
	br_fn_t bridge_func;
	void* request_arg;
};

queue_rpc* rpc_queue;
pthread_mutex_t rpc_mutex;
rpc_worker_func* rpc_func;

typedef struct _worker_params worker_params;

void* async_worker(void* arg)
{
	worker_params* worker_arg = (worker_params*) arg;
	br_fn_t bridge = worker_arg->bridge_func;
	request* req = (request*)worker_arg->request_arg;
        req->result = bridge(req->buffer);
	rpc_spin_unlock(&req->is_done);
	
	// Finish, free worker params arg.
	INLINE_SYSCALL(munmap, 2, ALLOC_ALIGNDOWN(worker_arg),
                   ALLOC_ALIGNUP(worker_arg+sizeof(worker_params)) -
                   ALLOC_ALIGNDOWN(worker_arg));
	
	return NULL;	
}

int call_async(br_fn_t bridge, void* req)
{
	worker_params* worker_arg = (worker_params*) INLINE_SYSCALL(mmap, 6, NULL, sizeof(worker_params),
                                   PROT_READ|PROT_WRITE,
                                   MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	worker_arg->bridge_func = bridge;
	worker_arg->request_arg = req;

	pthread_t worker_thread;
        int res = pthread_create(&worker_thread, NULL, async_worker, (void*)worker_arg);

        if (res)
        {
        	return -1;
        }

	return 0;
}

int work_around = 0;

void* rpc_thread(void* state)
{
        struct pal_enclave * enclave = (struct pal_enclave *)state;
        current_enclave = enclave;
	bool mutex_taken = false;
        
	while (1)
        {
		if (!mutex_taken)
		{
                	pthread_mutex_lock(&rpc_mutex);
			mutex_taken = true;
		}

                request* req = dequeue(rpc_queue);

                if (req == NULL)
                {
                        __asm__("pause");
                }
                else
                {
		        int ocall_index = req->ocall_index;

			// Note depcrated mechanism for blocking calls using thread pool, design moved to support it in 
			// 'Pal/src/host/Linux-SGX/enclave_ocalls.c'
			// Simply choose which ocalls will run using RPC and which will result in OCALLS.
			// TODO: can optimize this with adaptive mechanisms like was done in other works.
			bool cond = false; //(ocall_index == 21 || ocall_index ==34);

			if (cond) // blocking call, release mutex before executing to allow other threads to handle untrusted calls.
			{
				pthread_mutex_unlock(&rpc_mutex);
	                        mutex_taken = false;
			}

                        rpc_func(req);
                }
	}
	
	return 0;
}

int start_rpc_worker(int num_of_threads, void* _rpc_queue, void* arg, rpc_worker_func* _func)
{

	rpc_queue = (queue_rpc*) _rpc_queue;
	rpc_func = _func;

        pal_printf("in start rpc thread\n");


	if (pthread_mutex_init(&rpc_mutex, NULL) != 0)
	{
		return -1;
	}

        for (int i=0;i<num_of_threads;i++)
        {
                pthread_t communication_thread;
                int res = pthread_create(&communication_thread, NULL, &rpc_thread, arg);


                if (res)
                {
		        pal_printf("failed in start rpc thread\n");
                        return -1;
                }
        }

        return 0;
}

unsigned long * get_debug_register (void)
{
    return &debug_register;
}

void print_debug_register (void)
{
    SGX_DBG(DBG_E, "debug = %016x\n", debug_register);
}

struct tcs_map {
    unsigned int     tid;
    sgx_arch_tcs_t * tcs;
};

static struct tcs_map * tcs_map;
static int tcs_num;

void create_tcs_mapper (void * tcs_base, unsigned int thread_num)
{
    sgx_arch_tcs_t * all_tcs = tcs_base;

    tcs_map = malloc(sizeof(struct tcs_map) * thread_num);
    for (int i = 0 ; i < thread_num ; i++) {
        tcs_map[i].tid = 0;
        tcs_map[i].tcs = &all_tcs[i];
    }

    tcs_num = thread_num;
}

volatile int dummy = 1;

void map_tcs (unsigned int tid)
{
    for (int i = 0 ; i < tcs_num ; i++)
        if (!tcs_map[i].tid) {
            tcs_map[i].tid = tid;
            current_tcs = tcs_map[i].tcs;
            ((struct enclave_dbginfo *) DBGINFO_ADDR)->thread_tids[i] = tid;
            break;
        }
}

void unmap_tcs (void)
{
    for (int i = 0 ; i < tcs_num ; i++)
        if (tcs_map[i].tcs == current_tcs) {
            SGX_DBG(DBG_I, "unmap TCS at 0x%08lx\n", tcs_map[i].tcs);
            tcs_map[i].tid = 0;
            current_tcs = NULL;
            ((struct enclave_dbginfo *) DBGINFO_ADDR)->thread_tids[i] = 0;
            break;
        }
}

struct thread_arg {
    struct pal_enclave * enclave;
    pthread_t thread;
    void (*func) (void *, void *);
    void * arg;
    unsigned int * child_tid;
    unsigned int tid;
};

static void * thread_start (void * arg)
{
    struct thread_arg * thread_arg = (struct thread_arg *) arg;
    struct thread_arg local_arg = *thread_arg;
    local_arg.tid = thread_arg->tid = INLINE_SYSCALL(gettid, 0);

    INLINE_SYSCALL(futex, 6, &thread_arg->tid, FUTEX_WAKE, 1, NULL, NULL, 0);

    current_enclave = local_arg.enclave;
    map_tcs(local_arg.tid);
    if (!current_tcs) {
        SGX_DBG(DBG_E, "Cannot attach to any TCS!\n");
        return NULL;
    }

    ecall_thread_start(local_arg.func,
                       local_arg.arg,
                       local_arg.child_tid,
                       local_arg.tid);

    unmap_tcs();
    return NULL;
}

int clone_thread(void (*func) (void *, void *), void * arg,
                 unsigned int * child_tid, unsigned int * tid)
{
    int ret;
    struct thread_arg new_arg;

    new_arg.enclave = current_enclave;
    new_arg.func = func;
    new_arg.arg = arg;
    new_arg.child_tid = child_tid;
    new_arg.tid = 0;

    ret = pthread_create(&new_arg.thread, NULL, thread_start, &new_arg);

    if (ret < 0)
        return ret;

    INLINE_SYSCALL(futex, 6, &new_arg.tid, FUTEX_WAIT, 0, NULL, NULL, 0);

    if (tid)
        *tid = new_arg.tid;

    return ret;
}
