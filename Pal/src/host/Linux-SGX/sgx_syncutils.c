/*
 * SyncUtils.cpp
 *
 *  Created on: Jun 23, 2016
 *      Author: user
 */

#include "SyncUtils.h"

void rpc_spin_lock(int volatile *p)
{
    while(!__sync_bool_compare_and_swap(p, 0, 1))
    {
        while(*p) __asm__("pause");
    }
}

void rpc_spin_unlock(int volatile *p)
{
    asm volatile (""); // acts as a memory barrier.
    *p = 0;
}

