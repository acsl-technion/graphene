/*
 * Queue.h
 *
 *  Created on: Jun 24, 2016
 *      Author: user
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include "request.h"

#define RPC_QUEUE_SIZE 1024

typedef struct queue_t {
        unsigned long front, rear;
        request* q[RPC_QUEUE_SIZE];
        int volatile _lock;
} queue_rpc;

void init_rpc_queue(queue_rpc* q);
//int enqueue(queue_rpc* q, request* elem);
//int enqueue(queue_rpc* q, int ocall_index, void* ms);
request* dequeue(queue_rpc* q);

#endif /* QUEUE_H_ */
