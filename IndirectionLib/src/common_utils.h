/*
 * common_utils.h
 *
 *  Created on: Jul 23, 2016
 *      Author: user
 */

#ifndef TRUSTEDLIB_LIB_SERVICES_COMMON_COMMON_UTILS_H_
#define TRUSTEDLIB_LIB_SERVICES_COMMON_COMMON_UTILS_H_

#include "SyncUtils.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#if !defined (ALIGN16)
    #if defined (__GNUC__)
        #define ALIGN16 __attribute__ ( (aligned (16)))
    #elif defined(_MSC_VER)
        /* disable align warning, we want alignment ! */
        #pragma warning(disable: 4324)
        #define ALIGN16 __declspec (align (16))
    #else
        #define ALIGN16
    #endif
#endif

#define cpuid(reg, func)\
	__asm__ __volatile__ ("cpuid":\
		 "=a" (reg[0]), "=b" (reg[1]), "=c" (reg[2]), "=d" (reg[3]) :\
		 "a" (func));

#define XASM_LINK(f) asm(f)

#endif /* TRUSTEDLIB_LIB_SERVICES_COMMON_COMMON_UTILS_H_ */
