/*
 * C_warpper.c
 *
 *  Created on: Sep 5, 2016
 *      Author: user
 */


#include "C_warpper.h"

int AptrInit(void* pool, size_t pool_size)
{
    return initialize_aptr(pool, pool_size, NULL, NULL, NULL);
}

