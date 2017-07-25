/*
 * Queue.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: user
 */

#include "Queue.h"
#include "SyncUtils.h"

void init_rpc_queue(queue_rpc* q)
{
	q->front =0;
	q->rear = 0;
	for (int i=0;i<RPC_QUEUE_SIZE;i++)
        {
                q->q[i] = 0;
        }
}
/*
int enqueue(queue_rpc* q, int ocall_index, void* ms)
{
	rpc_spin_lock(&q->_lock);

	if(q->rear-q->front == RPC_QUEUE_SIZE)
	{
		rpc_spin_unlock(&q->_lock);
		return -1;
	}

	request* req = q->q[q->rear % RPC_QUEUE_SIZE];
	req->ocall_index = ocall_index;
	req->ms = ms;
	req->is_done = 1;
	q->rear++;

	rpc_spin_unlock(&q->_lock);
	return 0;
}
*/
request* dequeue(queue_rpc* q)
{
	rpc_spin_lock(&q->_lock);

	if(q->front == q->rear)
	{
		rpc_spin_unlock(&q->_lock);
		return 0;
	}

	request* result = q->q[q->front % RPC_QUEUE_SIZE];
	q->front++;

	rpc_spin_unlock(&q->_lock);

	return result;
}

