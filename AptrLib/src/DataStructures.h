#ifndef _DATA_STRUCTURES_HPP_INCLUDED_
#define	_DATA_STRUCTURES_HPP_INCLUDED_

#include <stdlib.h>
#include <string.h>
#include <sgx_tcrypto.h>

//#define APTR_RANDOM_ACCESS
#undef APTR_RANDOM_ACCESS


// First version - can use custom sizes for each item in future...
#define PAGE_SIZE 4096
#define EPC_SIZE 134217728 // 128 MB
#define CACHE_SIZE 41943040 // Can only set to 64 MB of the 128 MB (since enclave driver should take about 30 MB) - still no mem-pining
//#define CACHE_SIZE 31457280 // 30 M
//===========MOVED TO common/consts.h
//#define CACHE_SIZE (1<<6)*(1<<12)

//#define CACHE_SIZE 104857600 // 100 M
#define LLC_CAPACITY 262144 //65536
#define CACHE_CAPACITY (CACHE_SIZE / PAGE_SIZE)
#define NONCE_BYTE_SIZE 12

#ifndef APTR_RANDOM_ACCESS
#warning "**********************RANDOM_IS_DISABLED ************************"
#define SUB_PAGE_COUNT_LOG 0
#else
#warning "**********************RANDOM_IS_ENABLED ************************"
#define SUB_PAGE_COUNT_LOG 2
#endif

#define SUB_PAGE_COUNT (1 << SUB_PAGE_COUNT_LOG)
#define SUB_PAGE_SIZE_LOG (PAGE_SIZE_LOG - SUB_PAGE_COUNT_LOG)
#define SUB_PAGE_SIZE  (1 << SUB_PAGE_SIZE_LOG)

#endif


