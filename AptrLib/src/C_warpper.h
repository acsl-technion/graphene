/*
 * C_warpper.h
 *
 *  Created on: Sep 5, 2016
 *      Author: user
 */

#ifndef SRC_C_WARPPER_H_
#define SRC_C_WARPPER_H_

#include <stdlib.h>

int initialize_aptr(void* ptr_pool, size_t _pool_size, unsigned char** ptr_to_pin, size_t* size_to_pin, unsigned long long* untrusted_counters);

#ifdef __cplusplus
extern "C" {
#endif
/*
#define APTR_RANDOM_ACCESS
#undef APTR_RANDOM_ACCESS

struct __attribute__ ((aligned (8))) _aptr_t
{
	unsigned char** prm_ptr;
	int page;
	int offset;
#ifdef APTR_RANDOM_ACCESS
	unsigned short prm_linked_offset;
	char sub_page_index;
#endif
	bool hint_write;
};

typedef struct _aptr_t aptr_t;
*/
	int AptrInit(void* pool, size_t pool_size);
	// returns aptr pointer
	//void* InitAptr(void* ram_ptr, size_t size, int access_flags);
	//void dispose(void* aptrObj);
	// TODO: copy ctor? assignment-operator?

	// TODO: inline this
	void deref_set(void* ram_addr, int len, char* val);
	char deref_get(void* ram_addr);

	int strncmp_aptr(char* s1, char *s2, size_t n);
	void * memcpy_aptr(char *dst0, const char *src0, size_t length);
	void * memcpy_aptr_reg(char *dst0, const char *src0, size_t length);
	void * memcpy_reg_aptr(char *dst0, const char *src0, size_t length);

	void *memset_aptr(void *dst, int c, size_t n);

	int memcmp_aptr_aptr(const void* s1, const void* s2, size_t n);
	int memcmp_reg_aptr(const void* s1, const void* s2, size_t n);

	long long get_aptr_range();

	void *memmove_aptr_reg(void *dst0, void *src0, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SRC_C_WARPPER_H_ */

