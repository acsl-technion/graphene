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
int enqueue(queue_rpc* q, request* elem)
{
	rpc_spin_lock(&q->_lock);

	if(q->rear-q->front == RPC_QUEUE_SIZE)
	{
		rpc_spin_unlock(&q->_lock);
		return -1;
	}

	q->q[q->rear % RPC_QUEUE_SIZE] = elem;
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

