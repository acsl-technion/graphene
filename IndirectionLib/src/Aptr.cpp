/*
 * Aptr.cpp
 *
 *  Created on: Jul 17, 2016
 *      Author: user
 */

#include "Aptr.h"
#include "mem.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>       /* time */

#include "C_warpper.h"

unsigned char* untrusted_pool_ptr;


typedef	long word;		/* "word" used for optimal copy speed */

#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

void *memmove_reg_aptr(void *dst0, void *src0, size_t length)
{
	char* dsrc = (char*)((int64_t)src0 - (int64_t)untrusted_pool_ptr + (int64_t)base_pool_ptr);
	return memmove(dst0, dsrc, length);


//	fprintf(stderr, "reg entered %p %d\n", dst0, length);

	Aptr<char> src = Aptr<char>(src0, length, 0);
	int ctr = length;

	while (ctr > 0)
	{


		unsigned char* aptr_mem = deref(&src.m_aptr, src.m_base_page_index);
#ifdef APTR_RANDOM_ACCESS
		int left = SUB_PAGE_SIZE * (1 + src.m_aptr.sub_page_index) - src.m_aptr.offset;
#else
		int left = SUB_PAGE_SIZE - src.m_aptr.offset;
#endif

		if (ctr > left)
		{
			memmove((char*)dst0 + (length - ctr), aptr_mem, left);
			MOVE_APTR(&src.m_aptr, left);
			ctr -= left;
			//left = PAGE_SIZE;
		}
		else
		{
			memmove((char*)dst0 + (length - ctr), aptr_mem, ctr);
//			MOVE_APTR(&dst.m_aptr, ctr);
			ctr -= ctr;
		}
	}

//	fprintf(stderr, "reg leave %p %d\n", dst0,length);

	return dst0;
}

void *memmove_aptr_reg(void *dst0, void *src0, size_t length)
{
	unsigned char* dsrc = (unsigned char*)((int64_t)dst0 - (int64_t)untrusted_pool_ptr + (int64_t)base_pool_ptr);
        return memmove(dsrc, src0, length);


//	fprintf(stderr, "aptr enter %p %d\n", dst0,length);

	Aptr<char> dst = Aptr<char>(dst0, length, 0);
	dst.m_aptr.hint_write = true; // this is dirty state
	int ctr = length;
	while (ctr > 0)
	{
		unsigned char* aptr_mem = deref(&dst.m_aptr, dst.m_base_page_index);

#ifdef APTR_RANDOM_ACCESS
		int left = SUB_PAGE_SIZE * (1 + dst.m_aptr.sub_page_index) - dst.m_aptr.offset;
#else
		int left = SUB_PAGE_SIZE - dst.m_aptr.offset;
#endif

		if (ctr > left)
		{
			memmove(aptr_mem, (char*)src0 + (length-ctr), left);
			MOVE_APTR(&dst.m_aptr, left);
			ctr -= left;
//			left = PAGE_SIZE;
		}
		else
		{
			memmove(aptr_mem, (char*)src0 + (length-ctr), ctr);
			//MOVE_APTR(&dst.m_aptr, ctr);
			ctr -= ctr;
		}
	}

//	fprintf(stderr, "aptr leave %p %d\n", dst0,length);

	return dst0;
}

long long get_aptr_range()
{
	return (long long)untrusted_pool_ptr;
}

int memcmp_aptr_aptr(const void* s1, const void* s2, size_t n)
{

	char* ds1 = (char*)((unsigned long long)s1 - (unsigned long long)untrusted_pool_ptr + (unsigned long long)base_pool_ptr);
	char* ds2 = (char*)((unsigned long long)s2 - (unsigned long long)untrusted_pool_ptr + (unsigned long long)base_pool_ptr);

        return memcmp(ds1, ds2, n);

//	fprintf(stderr, "memcmp_aptr_aptr enter %p\n", s1);

    unsigned char u1, u2;
    Aptr<char> aptr_s1 = Aptr<char>((void*)s1, n, 0);
    Aptr<char> aptr_s2 = Aptr<char>((void*)s2, n, 0);

    for ( ; n-- ; MOVE_APTR(&aptr_s1.m_aptr,1), MOVE_APTR(&aptr_s2.m_aptr, 1)) {
		u1 = (unsigned char)*deref(&aptr_s1.m_aptr, aptr_s1.m_base_page_index);
		u2 = (unsigned char)*deref(&aptr_s2.m_aptr, aptr_s2.m_base_page_index);
		if ( u1 != u2) {
			return (u1-u2);
		}
    }

//    fprintf(stderr, "memcmp_aptr_aptr leave %p\n", s1);

    return 0;
}

int memcmp_reg_aptr(const void* s1, const void* s2, size_t n)
{

        char* ds2 = (char*)((int64_t)s2 - (int64_t)untrusted_pool_ptr + (int64_t)base_pool_ptr);

        return memcmp(s1, ds2, n);

    unsigned char u1, u2;
    char* aptr_s1 = (char*)s1;
    Aptr<char> aptr_s2 = Aptr<char>((void*)s2, n, 0);

    for ( ; n-- ; aptr_s1++, MOVE_APTR(&aptr_s2.m_aptr, 1)) {
		u1 = (unsigned char)*aptr_s1;
		u2 = (unsigned char)*deref(&aptr_s2.m_aptr, aptr_s2.m_base_page_index);
		if ( u1 != u2) {
			return (u1-u2);
		}
    }

//    fprintf(stderr, "memcmp_reg_aptr leave %p\n", s1);

    return 0;
}


void *memset_aptr(void *dst, int c, size_t n)
{
	char* ds1 = (char*)((unsigned long long)dst - (unsigned long long)untrusted_pool_ptr + (unsigned long long)base_pool_ptr);
        return memset(ds1, c, n);


	if (n != 0) {
			Aptr<char> aptr = Aptr<char>(dst, n, 0);
			aptr.m_aptr.hint_write = true;
			do
			{

					*deref(&aptr.m_aptr, aptr.m_base_page_index) = (unsigned char)c;
					MOVE_APTR(&aptr.m_aptr, 1);
			}
			while (--n != 0);
	}

//	fprintf(stderr, "memset leave %p\n", dst);
	return (dst);
}

void deref_set(void* ram_addr, int len, char* val)
{
	fprintf(stderr,"deref_set\n");
	
	char* dst1 = (char*)((unsigned long long)ram_addr - (unsigned long long)untrusted_pool_ptr + (unsigned long long)base_pool_ptr);

	while (len-- > 0)
	{
		*dst1 = *val;
		val++;
		dst1++;
	}

	return;
	

	Aptr<char> aptr = Aptr<char>(ram_addr, len, 0);
	aptr.m_aptr.hint_write = true;
	while (len-- > 0)
	{
		*deref(&aptr.m_aptr, aptr.m_base_page_index) = *val;
		MOVE_APTR(&aptr.m_aptr, 1);
		val++;
	}
}

char deref_get(void* ram_addr)
{
	fprintf(stderr,"deref_get called\n");
	Aptr<char> aptr = Aptr<char>(ram_addr, 1, 0);
	return *aptr;
}

int strncmp_aptr(char* s1, char *s2, size_t n)
{
	char* ds1 = (char*)((int64_t)s1 - (int64_t)untrusted_pool_ptr + (int64_t)base_pool_ptr);

        return strncmp(ds1, s2, n);

	
//	fprintf(stderr, "strncmp enter %p\n", s1);

	Aptr<char> aptr = Aptr<char>(s1, n, 0);
    for ( ; n > 0; MOVE_APTR(&aptr.m_aptr, 1), s2++, --n)
    {
    	unsigned char* val = deref(&aptr.m_aptr, aptr.m_base_page_index);
    	if (*val != *s2)
    	{
//    		fprintf(stderr, "strncmp leave %p\n", s1);
			return ((*val < *(unsigned char *)s2) ? -1 : +1);
    	}
		else if (*val == '\0')
		{
//			fprintf(stderr, "strncmp leave %p\n", s1);
			return 0;
		}
	}

//    fprintf(stderr, "strncmp leave %p\n", s1);
    return 0;
}

void * memcpy_internal(char *dst0, const char *src0, size_t length)
{
	if (length == 0 || dst0 == src0)		/* nothing to do */
		return (dst0);
	char* dst = (char*)dst0;
	char* src = (char*)src0;

	do
	{
			*dst = *src;
			dst++;
			src++;
	}
	while (--length != 0);
	return dst0;
}

void * memcpy_reg_aptr(char *dst0, const char *src0, size_t length)
{
	return memmove_reg_aptr(dst0, (void*)src0, length);

	if (length == 0 || dst0 == src0)		/* nothing to do */
		return (dst0);
	char* dst = (char*)dst0;
	Aptr<char> src = Aptr<char>((void*)src0, length, 0);

	do
	{
			*dst = *deref(&src.m_aptr, src.m_base_page_index);
			dst++;
			MOVE_APTR(&src.m_aptr, 1);
	}
	while (--length != 0);
	return dst0;
}

void * memcpy_aptr_reg(char *dst0, const char *src0, size_t length)
{
	return memmove_aptr_reg(dst0, (void*)src0, length);

	if (length == 0 || dst0 == src0)		/* nothing to do */
		return (dst0);
	Aptr<char> dst = Aptr<char>(dst0, length, 0);
	char* src = (char*)src0;

	do
	{
			*deref(&dst.m_aptr, dst.m_base_page_index) = *src;
			MOVE_APTR(&dst.m_aptr, 1);
			src++;
	}
	while (--length != 0);
	return dst0;
}

void * memcpy_aptr(char *dst0, const char *src0, size_t length)
{
	fprintf(stderr,"memcpy_aptr_aptr %ld\n", length);

//	return memcpy_internal(dst0, src0, length);

	if (length == 0 || dst0 == src0)		/* nothing to do */
		return (dst0);

	Aptr<char> dst = Aptr<char>(dst0, length, 0);
	Aptr<char> src = Aptr<char>((void*)src0, length, 0);
	dst.m_aptr.hint_write = true;

	do
	{
			*deref(&dst.m_aptr, dst.m_base_page_index) = *deref(&src.m_aptr, src.m_base_page_index);
			MOVE_APTR(&dst.m_aptr, 1);
			MOVE_APTR(&src.m_aptr, 1);
	}
	while (--length != 0);
	return dst0;
}

void debug(const char* fmt, ...) {}

//const uint8_t AUTH[] =
//{
//	0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
//	0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
//	0xab, 0xad, 0xda, 0xd2
//};

//void debug(const char *fmt, ...)
//{
//    char buf[BUFSIZ] = {'\0'};
//    va_list ap;
//    va_start(ap, fmt);
//    vsnprintf(buf, BUFSIZ, fmt, ap);
//    va_end(ap);
//    ocall_debug(buf);
//}
unsigned char DUMMY = 0;

sgx_aes_gcm_128bit_key_t eviction_key;
unsigned char* base_pool_ptr;
size_t pool_size;
bool is_initialized;

s_page_link_state* s_page_ref_count_arr;

std::unordered_map<int, unsigned char**,MyTemplatePointerHash1<int>> s_aptr_crypto_page_cache_l1(CACHE_CAPACITY); // cache that is always linked - fast access, aptr that has 0 ref_count will move to LLC
std::unordered_map<int, s_crypto_page,MyTemplatePointerHash2<int>> s_aptr_crypto_page_cache_llc(LLC_CAPACITY); // cache that contains all pages that are used can be very large (may be swapped out). Memory that is freed will be removed from llc
std::vector<unsigned char**> s_free_epc_pages;

volatile int allocated_pages;
volatile int allocated_pages_lock;
volatile int s_aptr_crypto_page_cache_l1_lock;
volatile int s_aptr_crypto_page_cache_llc_lock;
volatile int s_free_epc_pages_lock;

// assume cleanup is thread-unsafe - under user responsibility.
int cleanup_resources()
{
	for (auto it = s_free_epc_pages.begin(); it < s_free_epc_pages.end(); it++)
	{
		unsigned char** epc_page = *it;
		free(*epc_page);
	}

	// TODO: clean the arr...

	// clean data structures unused stack memory
	s_free_epc_pages.clear();
//	s_fifo_page_index_list.clear();
	s_aptr_crypto_page_cache_l1.clear();
	s_aptr_crypto_page_cache_llc.clear();

	return 0;
}

extern "C" sgx_status_t sgx_init_crypto_lib(uint64_t cpu_feature_indicator);
extern "C" uint64_t ippGetEnabledCpuFeatures (void);

// assume init is thread-unsafe - under user responsibility.
int initialize_aptr(void* ptr_pool, size_t _pool_size)
{
	if (is_initialized || ptr_pool == NULL)
	{
		abort(); // error
	}

        uint64_t mask = 0;
        mask |= 0x00000001;
        mask |= 0x00000002;
        mask |= 0x00000004;
        mask |= 0x00000008;
        mask |= 0x00000010;
        mask |= 0x00000020;
        mask |= 0x00000040;
        mask |= 0x00000200;
        mask |= 0x00000800;
        mask |= 0x00002000;
        mask |= 0x00004000;
        mask |= 0x00008000;
        mask |= 0x00010000;
        mask |= 0x00020000;
        mask |= 0x00040000;

        mask |= 0x00000400; //aes;
        mask |= 0x00000080; //sse4.2
        mask |= 0x00000100; //avx;
        fprintf(stderr,"%ld\n", mask);
        sgx_init_crypto_lib(mask);

	untrusted_pool_ptr = (unsigned char*)ptr_pool;
	pool_size = _pool_size;//1024 * 1024 * 1024; // 500MB
        base_pool_ptr = (unsigned char*)malloc(pool_size);
	return 0;
}

int run_clock(int* page_to_evict, unsigned char*** prm_addr_to_evict)
{
	*page_to_evict = -1;
	spin_lock(&s_aptr_crypto_page_cache_l1_lock);

//	auto it = s_fifo_page_index_list.front();
//	*page_to_evict = it->first;
//	*prm_addr_to_evict = it->second;
//	s_fifo_page_index_list.pop_front();

	for (auto it=s_aptr_crypto_page_cache_l1.begin(); it != s_aptr_crypto_page_cache_l1.end(); it++)
	{
//		debug("CLK LOCK: %d\n", it->first);
//		spin_lock(&s_page_ref_count_arr[it->first].lock);
		int ref_count = s_page_ref_count_arr[it->first].ref_count;
//		spin_unlock(&s_page_ref_count_arr[it->first].lock);
//		debug("CLK UNLOCK: %d\n", it->first);

		if (ref_count == 0)
		{
			*page_to_evict = it->first;
			*prm_addr_to_evict = it->second;
			break;
		}
	}

	if (*page_to_evict == -1)
	{
		debug("page to evict no found\n");
		abort(); // major error, abort the program
	}

	spin_unlock(&s_aptr_crypto_page_cache_l1_lock);
	return 0;
}

unsigned long g_evict_counter = 0, g_decrypt_counter = 0;
unsigned long g_dirty=0;

int try_evict_page(int page_index, unsigned char** prm_addr)
{
	spin_lock(&s_aptr_crypto_page_cache_l1_lock);
	auto l1_it = s_aptr_crypto_page_cache_l1.find(page_index);
	if (l1_it == s_aptr_crypto_page_cache_l1.end())
	{
		spin_unlock(&s_aptr_crypto_page_cache_l1_lock);
		return -1;
	}
	spin_unlock(&s_aptr_crypto_page_cache_l1_lock);

//	unsigned lowbits1 = (long long)prm_addr & (1 << 0);
//	debug("evicted %d\n", lowbits1);

	long long prm_addr_long = (long long)*prm_addr;
//	debug("remove: %d\n", prm_addr_long & 0x01);


	//spin_lock(&s_aptr_crypto_page_cache_llc_lock);
	//bool in_llc = s_aptr_crypto_page_cache_llc.find(page_index) != s_aptr_crypto_page_cache_llc.end();
	//spin_unlock(&s_aptr_crypto_page_cache_llc_lock);

	if (/*!in_llc ||*/ (prm_addr_long & 0x01) == 1)
	{
		prm_addr_long &= 0xFFFFFFFFFFFFFFFE;
		*prm_addr = (unsigned char*)prm_addr_long;

//		debug("evicting %p\n",*prm_addr);

		s_crypto_page evicted_pagemetadata;

		for (int i=0;i<NONCE_BYTE_SIZE;i++)
		{
			evicted_pagemetadata.nonce[i] = rand() % 256;
		}

		sgx_aes_gcm_128bit_tag_t& mac = evicted_pagemetadata.mac[0];

//		sgx_read_rand(evicted_pagemetadata.nonce, NONCE_BYTE_SIZE);
		unsigned char* ram_page_ptr = base_pool_ptr + page_index * PAGE_SIZE;

//		AES_GCM_encrypt(prm_addr,
//				ram_page_ptr,
//				AUTH,
//				evicted_pagemetadata.nonce,
//				evicted_pagemetadata.mac,
//				PAGE_SIZE,
//				sizeof(AUTH),
//				NONCE_BYTE_SIZE,
//				eviction_key,
//				10);

		sgx_status_t ret = sgx_rijndael128GCM_encrypt(&eviction_key,
				*prm_addr,
				PAGE_SIZE,
				ram_page_ptr,
				evicted_pagemetadata.nonce,
				NONCE_BYTE_SIZE,
				NULL,
				0,
				&mac);

		assert (ret == SGX_SUCCESS);

		// keep track of free'd addresses
		evicted_pagemetadata.is_initialized[0] = true;

		spin_lock(&s_aptr_crypto_page_cache_llc_lock);
		s_aptr_crypto_page_cache_llc[page_index] = evicted_pagemetadata;
		spin_unlock(&s_aptr_crypto_page_cache_llc_lock);
	}

	spin_lock(&s_aptr_crypto_page_cache_l1_lock);
	s_aptr_crypto_page_cache_l1.erase(page_index);
//	s_fifo_page_index_list.remove(page_index);
	spin_unlock(&s_aptr_crypto_page_cache_l1_lock);

	spin_lock(&s_free_epc_pages_lock);
	s_free_epc_pages.push_back(prm_addr);
	spin_unlock(&s_free_epc_pages_lock);

	// written back encrypted data - done
	return 0;
}


void page_fault(aptr_t* aptr, int base_page_index)
{
	// first remove ref count from aptr only if it was linked before...
	if (*aptr->prm_ptr != NULL)
	{
		int prev_page_index = base_page_index + (aptr->page);
		spin_lock_c(&s_page_ref_count_arr[prev_page_index].lock);
		s_page_ref_count_arr[prev_page_index].ref_count--;
/*
		if (s_page_ref_count_arr[prev_page_index].ref_count == 0 && aptr->hint_write)
		{
			// This will set as dirty
			long long prm_ptr_long = (long long)*aptr->prm_ptr;
			if ((prm_ptr_long & 0x01) == 0)
			{
				prm_ptr_long |= 0x01;
				*aptr->prm_ptr = (unsigned char*)prm_ptr_long;
			}
		}
*/
		spin_unlock_c(&s_page_ref_count_arr[prev_page_index].lock);
		//*base = NULL; - should do this but assume that it would be corrected next time
	}

	// now update aptr to its new state.
    int tmp_offset=(aptr->offset)&PAGE_BITS_MASK;
    int tmp_page=(aptr->offset)>>PAGE_SIZE_LOG;
    int real_page=aptr->page+tmp_page;

    // now deal with the page fault..
	int page_index = base_page_index + real_page;
	spin_lock_c(&s_page_ref_count_arr[page_index].lock);
	aptr->offset=tmp_offset;
	aptr->page=real_page;

	spin_lock(&s_aptr_crypto_page_cache_l1_lock);
	auto it = s_aptr_crypto_page_cache_l1.find(page_index);
	bool is_minor = it != s_aptr_crypto_page_cache_l1.end();
	spin_unlock(&s_aptr_crypto_page_cache_l1_lock);

	if (is_minor) // minor #PF
	{
		aptr->prm_ptr = it->second;
		s_page_ref_count_arr[page_index].ref_count++;
		spin_unlock_c(&s_page_ref_count_arr[page_index].lock);
		return;
	}

	// major page fault
	int pages_allocated_in_fault = 1;
	spin_lock(&allocated_pages_lock);
	int cache_size = allocated_pages;
	spin_unlock(&allocated_pages_lock);

	if (cache_size >= CACHE_CAPACITY)
	{
		/*
		if (cache_size >= CACHE_CAPACITY - LOAD_FACTOR)
		{
			//start async worker
			rpc_ocall(-1, NULL);
		}
		*/

		bool evicted = false;
		do
		{
			int page_to_evict;
			unsigned char** prm_addr_to_evict;
			run_clock(&page_to_evict, &prm_addr_to_evict);

			spin_lock_c(&s_page_ref_count_arr[page_to_evict].lock);

			if (s_page_ref_count_arr[page_to_evict].ref_count == 0)
			{
				evicted = try_evict_page(page_to_evict, prm_addr_to_evict) == 0;
			}

			spin_unlock_c(&s_page_ref_count_arr[page_to_evict].lock);

		} while (!evicted);

		pages_allocated_in_fault--;
	}

	s_crypto_page metadata;

	spin_lock(&s_aptr_crypto_page_cache_llc_lock);

	auto llc_it = s_aptr_crypto_page_cache_llc.find(page_index);
	bool is_not_in_llc = llc_it == s_aptr_crypto_page_cache_llc.end();
	if (is_not_in_llc) // first time
	{
		metadata.is_initialized[0] = false;
	}
	else
	{
		metadata = llc_it->second;
	}

	spin_unlock(&s_aptr_crypto_page_cache_llc_lock);

	spin_lock(&s_free_epc_pages_lock);
	auto free_epc_page_it = s_free_epc_pages.begin();
	if (free_epc_page_it == s_free_epc_pages.end())
	{
		debug("no free epc page found\n");
		abort(); // major problem abort immediately
	}

	unsigned char** prm_ptr = *free_epc_page_it;
	s_free_epc_pages.erase(free_epc_page_it); // used - remove from list
	spin_unlock(&s_free_epc_pages_lock);

	if (metadata.is_initialized[0])
	{
		unsigned char* ram_page_ptr = base_pool_ptr + page_index * PAGE_SIZE;
		sgx_aes_gcm_128bit_tag_t& mac = metadata.mac[0];

		sgx_status_t ret = sgx_rijndael128GCM_decrypt(&eviction_key,
				ram_page_ptr,
				PAGE_SIZE,
				(uint8_t*)*prm_ptr,
				metadata.nonce,
				NONCE_BYTE_SIZE,
				NULL,
				0,
				&mac);

		assert (ret == SGX_SUCCESS);
	}

	// Now add to cache
	//metadata.is_dirty |= !m_offset.is_read_only;

	spin_lock(&s_aptr_crypto_page_cache_l1_lock);
	s_aptr_crypto_page_cache_l1[page_index] = prm_ptr;
//	s_fifo_page_index_list.push_back(s_aptr_crypto_page_cache_l1.find(page_index));
	spin_unlock(&s_aptr_crypto_page_cache_l1_lock);

/*
	spin_lock(&s_aptr_crypto_page_cache_llc_lock);
	if (llc_it != s_aptr_crypto_page_cache_llc.end())
	{
		s_aptr_crypto_page_cache_llc.erase(llc_it);
	}

	spin_unlock(&s_aptr_crypto_page_cache_llc_lock);
*/

	if (pages_allocated_in_fault > 0)
	{
		spin_lock(&allocated_pages_lock);
		allocated_pages += pages_allocated_in_fault;
		spin_unlock(&allocated_pages_lock);
	}

	// finally, link
	aptr->prm_ptr = prm_ptr;

	s_page_ref_count_arr[page_index].ref_count++;
	spin_unlock_c(&s_page_ref_count_arr[page_index].lock);
}

#ifdef APTR_RANDOM_ACCESS

aptr_t last_aptr;

unsigned char* deref_direct(aptr_t* aptr, int base_page_index)
{
	// now update aptr to its new state.
	int tmp_offset=(aptr->offset)&PAGE_BITS_MASK;
	int tmp_page=(aptr->offset)>>PAGE_SIZE_LOG;
	int real_page=aptr->page+tmp_page;

	// now deal with the page fault..
	aptr->offset=tmp_offset;
	aptr->page=real_page;
	int sub_page_index = tmp_offset >> SUB_PAGE_SIZE_LOG;
	aptr->sub_page_index = sub_page_index;

	int page_index = base_page_index + aptr->page;

        spin_lock(&s_free_epc_pages_lock);
        unsigned char** res = s_free_epc_pages.back();
        s_free_epc_pages.pop_back();
        spin_unlock(&s_free_epc_pages_lock);


	if (last_aptr.page == page_index && last_aptr.sub_page_index == sub_page_index &&
		res == last_aptr.prm_ptr)
	{
		//fprintf(stderr,"optimize\n");
		aptr->prm_ptr = last_aptr.prm_ptr;
	        aptr->prm_linked_offset = last_aptr.prm_linked_offset;

        	return (unsigned char*)((uint64_t)*aptr->prm_ptr + aptr->offset);// - aptr->sub_page_index * SUB_PAGE_SIZE);
	}

//	spin_lock(&s_page_ref_count_arr[page_index].lock);

	s_crypto_page pagemetadata;

	spin_lock(&s_aptr_crypto_page_cache_llc_lock);
	auto it = s_aptr_crypto_page_cache_llc.find(page_index);
	if (it != s_aptr_crypto_page_cache_llc.end())
	{
		pagemetadata = it->second;
	}
	else
	{
		for (int i=0;i<SUB_PAGE_COUNT; i++)
		{
			pagemetadata.is_initialized[i] = false;
		}
	}
	spin_unlock(&s_aptr_crypto_page_cache_llc_lock);

	if (pagemetadata.is_initialized[sub_page_index])
	{
		unsigned char* ram_page_ptr = base_pool_ptr + page_index * PAGE_SIZE + sub_page_index * SUB_PAGE_SIZE;

		unsigned char* nonce = pagemetadata.nonce + sub_page_index * NONCE_BYTE_SIZE;
		sgx_aes_gcm_128bit_tag_t& mac = pagemetadata.mac[sub_page_index];

		sgx_status_t ret = sgx_rijndael128GCM_decrypt(&eviction_key,
				ram_page_ptr,
				SUB_PAGE_SIZE,
				*res + sub_page_index * SUB_PAGE_SIZE,
				nonce,
				NONCE_BYTE_SIZE,
				NULL,
				0,
				&mac);

		assert (ret == SGX_SUCCESS);
	}

	aptr->prm_ptr = res;
	aptr->prm_linked_offset = aptr->offset;

	// cache last accesed
	last_aptr.prm_ptr = res;
	last_aptr.page = page_index;
	last_aptr.sub_page_index = sub_page_index;
	last_aptr.prm_linked_offset = aptr->offset;
//	spin_unlock(&s_page_ref_count_arr[page_index].lock);

	return (unsigned char*)((uint64_t)*aptr->prm_ptr + aptr->offset);// - aptr->sub_page_index * SUB_PAGE_SIZE);
}

int aptr_fsync(aptr_t* aptr, int base_page_index)
{
	if (*aptr->prm_ptr == NULL)
	{
		// nothing to sync
		return -1;
	}

	if (!aptr->hint_write)
	{
		spin_lock(&s_free_epc_pages_lock);
		s_free_epc_pages.push_back(aptr->prm_ptr);
		spin_unlock(&s_free_epc_pages_lock);
		aptr->prm_ptr = (unsigned char**)&DUMMY;

		return 0;
	}

	int sub_page_index = aptr->sub_page_index ;
	int page_index = base_page_index + aptr->page;
	unsigned char* ram_page_ptr = base_pool_ptr + page_index * PAGE_SIZE + sub_page_index * SUB_PAGE_SIZE;

	s_crypto_page evicted_pagemetadata;
	spin_lock(&s_aptr_crypto_page_cache_llc_lock);
	auto it = s_aptr_crypto_page_cache_llc.find(page_index);
	if (it != s_aptr_crypto_page_cache_llc.end())
	{
		evicted_pagemetadata = it->second;
	}
	else
	{
		for (int i=0;i<SUB_PAGE_COUNT; i++)
		{
			evicted_pagemetadata.is_initialized[i] = false;
		}
	}
	spin_unlock(&s_aptr_crypto_page_cache_llc_lock);

	unsigned char* nonce = evicted_pagemetadata.nonce + sub_page_index * NONCE_BYTE_SIZE;
	sgx_aes_gcm_128bit_tag_t& mac = evicted_pagemetadata.mac[sub_page_index];

	for (int i=0;i<NONCE_BYTE_SIZE;i++)
	{
		nonce[i] = rand() % 256;
	}

	sgx_status_t ret = sgx_rijndael128GCM_encrypt(&eviction_key,
			*aptr->prm_ptr + sub_page_index * SUB_PAGE_SIZE,
			SUB_PAGE_SIZE,
			ram_page_ptr,
			nonce,
			NONCE_BYTE_SIZE,
			NULL,
			0,
			&mac);

	assert (ret == SGX_SUCCESS);

	// keep track of free'd addresses
	evicted_pagemetadata.is_initialized[sub_page_index] = true;

	// TODO: Not sure if this is needed...
	spin_lock(&s_aptr_crypto_page_cache_llc_lock);
	s_aptr_crypto_page_cache_llc[page_index] = evicted_pagemetadata;
	spin_unlock(&s_aptr_crypto_page_cache_llc_lock);

	spin_lock(&s_free_epc_pages_lock);
	s_free_epc_pages.push_back(aptr->prm_ptr);
	spin_unlock(&s_free_epc_pages_lock);

	aptr->prm_ptr = (unsigned char**)&DUMMY;

	return 0;
}

#endif
