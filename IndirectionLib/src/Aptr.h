/*
 * Aptr.h
 *
 *  Created on: Jul 15, 2016
 *      Author: user
 */

#ifndef TRUSTEDLIB_LIB_SERVICES_STATIC_TRUSTED_APTR_H_
#define TRUSTEDLIB_LIB_SERVICES_STATIC_TRUSTED_APTR_H_

#include <stdio.h>
#include "mem.h"
#include "common_utils.h"
#include "DataStructures.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "SyncUtils.h"
#include <vector>
#include <unordered_map>
#include <list>
#include  <math.h>


#include "C_warpper.h"
#include <sgx_tcrypto.h>

void debug(const char *fmt, ...);

#define LLC_CAPACITY 65536

#define MN_REQ 16
#define PAGE_SIZE_LOG 12
#define LOAD_FACTOR 32
//#define PAGE_SIZE 1 << PAGE_SIZE_LOG
#define PAGE_BITS_MASK (PAGE_SIZE-1)

#ifndef APTR_RANDOM_ACCESS
#define SUB_PAGE_COUNT_LOG 0
#else
#define SUB_PAGE_COUNT_LOG 2
#endif

#define SUB_PAGE_COUNT (1 << SUB_PAGE_COUNT_LOG)
#define SUB_PAGE_SIZE_LOG (PAGE_SIZE_LOG - SUB_PAGE_COUNT_LOG)
#define SUB_PAGE_SIZE  (1 << SUB_PAGE_SIZE_LOG)

#define VALID_BITS 1
#define ACCESS_BITS 1
#define OFFSET_PTR_BITS 64
#define PAGE_BITS (OFFSET_PTR_BITS - PAGE_SIZE_LOG - VALID_BITS - ACCESS_BITS)
#define VALID_MASK 0xFFFFF000

#define ISVALID(X) (((X) < PAGE_SIZE) && ((X)>=0))
//#define ISVALID(X) ((X)&VALID_MASK == 0)
#define MOVE_APTR(aptr,val) (aptr)->offset+=val
//#define APTR2ADDR(X, BASE) (((int64_t)BASE)+(int64_t)(((int64_t)(X).page)<<PAGE_SIZE_LOG)+(X).offset)
#define APTR2ADDR(APTR) (unsigned char*)(((int64_t)*(APTR).prm_ptr & 0xFFFFFFFFFFFFFFFE)+(APTR).offset)

//struct __attribute__ ((aligned (8))) aptr_t
//{
//	unsigned char** prm_ptr;
//	int page;
//	int offset;
//	bool hint_write;
//};

struct s_page_link_state
{
	uint8_t ref_count;
	volatile uint8_t lock;
};

typedef uint8_t sgx_aes_gcm_128bit_tag_t[16];

struct s_crypto_page
{
	uint8_t nonce[NONCE_BYTE_SIZE * SUB_PAGE_COUNT];
	sgx_aes_gcm_128bit_tag_t mac[SUB_PAGE_COUNT];
//	sgx_aes_gcm_128bit_tag_t mac;
	bool is_initialized[SUB_PAGE_COUNT];
} __attribute__ ((aligned (8)));

template<typename Tval>
struct MyTemplatePointerHash1 {
    size_t operator()(const Tval val) const {
    	return val & (CACHE_CAPACITY -1);
        //static const size_t shift = (size_t)log2(1 + sizeof(Tval));
        //return (size_t)(val) >> shift;
    }
};

template<typename Tval>
struct MyTemplatePointerHash2 {
    size_t operator()(const Tval val) const {
    	return val & (LLC_CAPACITY -1);
        //static const size_t shift = (size_t)log2(1 + sizeof(Tval));
        //return (size_t)(val) >> shift;
    }
};

// Should be initialized to zeroes
extern s_page_link_state* s_page_ref_count_arr;
extern std::unordered_map<int, unsigned char**,MyTemplatePointerHash1<int>> s_aptr_crypto_page_cache_l1; // cache that is always linked - fast access, aptr that has 0 ref_count will move to LLC
extern std::unordered_map<int, s_crypto_page,MyTemplatePointerHash2<int>> s_aptr_crypto_page_cache_llc; // cache that contains all pages that are used can be very large (may be swapped out). Memory that is freed will be removed from llc
extern std::vector<unsigned char**> s_free_epc_pages;

extern sgx_aes_gcm_128bit_key_t eviction_key;
extern unsigned char* base_pool_ptr;
extern size_t pool_size;

int initialize_aptr(void* ptr_pool, size_t _pool_size);
int cleanup_resources();
int run_clock(int* page_to_evict, unsigned char*** prm_addr_to_evict);
int try_evict_page(int page_index, unsigned char** prm_addr);

void remove_entries_pcache(int num_entries);
void page_fault(aptr_t* aptr, int base_page_index);
inline unsigned char* deref(aptr_t* aptr, int base_page_index) __attribute__ ((always_inline));

#ifdef APTR_RANDOM_ACCESS
int aptr_fsync(aptr_t* aptr, int base_page_index);
unsigned char* deref_direct(aptr_t* aptr, int base_page_index);
#endif

extern unsigned char DUMMY;

template<typename T>
class Aptr {
public:
	// TODO: get memory from outside.
	// int* temp_ptr = malloc(sizeof(int)*64);
	// Aptr<int>(temp_ptr,sizeof(int)*64
	// .....
	// when the ptr is dead
	// free(temp_ptr);
	// only create ref_count,etcmetadata for first page
	// when moving->reset should delete page if it was the last one
	// otherwise->just dec ref_count and move to next one...'
	// so ref_count will only count PRM ptrs!
	// the same goes for the rest of the metadata....
	void init(){
		m_size=0, m_base_page_index=-1;

	}
	Aptr()  
	{
		init();	// null aptr
	}

	Aptr(void* ram_ptr, size_t size, int notUsed) : m_size(size)
	{
		if (ram_ptr == NULL)
		{
			return;
		}

		//m_offset.is_read_only = !access_flags; // TODO
		m_aptr.offset = ((((__int64_t)ram_ptr - (__int64_t)base_pool_ptr)) & PAGE_BITS_MASK) + PAGE_SIZE;
		m_aptr.page = -1;
		m_aptr.prm_ptr = (unsigned char**)&DUMMY; // DUMMY VALUE
		m_aptr.hint_write = false;
#ifdef APTR_RANDOM_ACCESS
		m_aptr.sub_page_index = 0;
		m_aptr.prm_linked_offset = 0;
#endif
		m_base_page_index = ((((__int64_t)ram_ptr - (__int64_t)base_pool_ptr)) >> PAGE_SIZE_LOG);

		assert(m_base_page_index >= 0);
	}

	Aptr(const Aptr& aptr)
	{
		do_copy(aptr);
	}

	//virtual ~Aptr()
	~Aptr()
	{
		reset();
	}


	inline Aptr& operator=(const Aptr<T>& aptr)  __attribute__ ((always_inline))
	{
		reset();
		do_copy(aptr);

		return *this;
	}

#define INC_APTRS
	//Aptr& moveTo(size_t offset);

	inline Aptr& move(size_t offset) __attribute__ ((always_inline))
	{
		offset *= sizeof(T);
		MOVE_APTR(&m_aptr, offset);
		return *this;
	}

	inline T& operator *()  __attribute__ ((always_inline))
	{
		void* pRet = deref(&m_aptr, m_base_page_index);
		return *((T*)pRet);
	}

	inline bool is_not_null()  __attribute__ ((always_inline))
	{
		return m_base_page_index > 0;
	}

	inline bool operator!()  __attribute__ ((always_inline))
	{
		return m_base_page_index < 0;
	}

	inline bool operator==(const Aptr& lhs)  __attribute__ ((always_inline))
	{
		return lhs.m_base_page_index == m_base_page_index;//TODO && lhs.m_aptr.offset + (lhs.m_aptr.page >> PAGE_SIZE_LOG) == m_aptr.offset + m_aptr.page >> PAGE_SIZE_LOG;
	}

	inline bool operator!=(const Aptr& lhs)  __attribute__ ((always_inline))
	{
		return lhs.m_base_page_index != m_base_page_index;//TODO || lhs.m_aptr.offset != m_aptr.offset || lhs.m_aptr.page != m_aptr.page;
	}

	// Postfix
	inline Aptr& operator ++(int)  __attribute__ ((always_inline))
	{
		return move(1);
	}

	// Postfix
	inline Aptr& operator --(int)  __attribute__ ((always_inline))
	{
		return move(-1);
	}

	// Prefix
	inline Aptr& operator ++()  __attribute__ ((always_inline))
	{
		return move(1);
	}

	// Prefix
	inline Aptr& operator --()  __attribute__ ((always_inline))
	{
	return move(-1);
	}


	inline Aptr& operator +=(size_t offset)  __attribute__ ((always_inline))
	{
		return move(offset);
	}

	inline Aptr& operator -=(size_t offset)  __attribute__ ((always_inline))
	{
		return move(-offset);
	}

	inline void reset()  __attribute__ ((always_inline))
	{
		if (m_base_page_index < 0 || m_aptr.prm_ptr == (unsigned char**)&DUMMY)
		{
			return;
		}

#ifndef APTR_RANDOM_ACCESS
		if (*m_aptr.prm_ptr != NULL)
		{
			int page_index = m_base_page_index + (m_aptr.page);
			spin_lock_c(&s_page_ref_count_arr[page_index].lock);
			s_page_ref_count_arr[page_index].ref_count--;
			//if (m_aptr.hint_write && g_page_ref_count_arr[page_index].ref_count == 0)
		/*	if (m_aptr.hint_write)
			{
				long long prm_ptr_long = (long long)*m_aptr.prm_ptr;
				if ((prm_ptr_long & 0x01) == 0)
				{
					// This will set as dirty
					prm_ptr_long |= 0x01;
					*m_aptr.prm_ptr = (unsigned char*)prm_ptr_long;

				}
			}*/ // Marina deleted this
			spin_unlock_c(&s_page_ref_count_arr[page_index].lock);
		}

#else
		aptr_fsync(&m_aptr, m_base_page_index);
#endif

		//debug("reset\n");
		m_base_page_index = -1; // for double dtor's case
		m_aptr.prm_ptr = (unsigned char**)&DUMMY;
	}

	inline void do_copy(const Aptr& aptr)  __attribute__ ((always_inline))
	{
		
		m_base_page_index = aptr.m_base_page_index;
	//	if (ISVALID(aptr.m_aptr.offset)){
			m_aptr.offset = aptr.m_aptr.offset+PAGE_SIZE;
			m_aptr.page = aptr.m_aptr.page - 1;
	//	}else{
	//		m_aptr.offset = aptr.m_aptr.offset;
	//		m_aptr.page = aptr.m_aptr.page ;
//
//		}
		m_aptr.prm_ptr = (unsigned char**)&DUMMY; // DUMMY VALUE- not valid by def
		m_aptr.hint_write = aptr.m_aptr.hint_write;
		m_size = aptr.m_size;
		//debug("do_copy: offset: %p, page: %p  \n", m_aptr.offset,m_aptr.page);
	}

//private:
	aptr_t m_aptr;
	int m_base_page_index;
	size_t m_size;
};

extern unsigned long g_dirty;

inline unsigned char* deref(aptr_t* aptr, int base_page_index)
{
#ifndef APTR_RANDOM_ACCESS
	int b=ISVALID(aptr->offset);
	
		
	unsigned char* pRet;
	if (likely(b)){
	
		pRet = APTR2ADDR(*aptr);
	}else{
			// first - get the correct aptr page & offset
			page_fault(aptr, base_page_index);
			pRet = APTR2ADDR(*aptr);
			


	}
	
	if (aptr->hint_write)
	{
		long long prm_ptr_long = (long long)*(aptr->prm_ptr);
		prm_ptr_long |= 0x01;
		g_dirty++;
//		debug("%lu\n",g_dirty);
		*(aptr->prm_ptr) = (unsigned char*)prm_ptr_long;
	}
	return pRet;

#else
	int b = aptr->offset >= aptr->sub_page_index * SUB_PAGE_SIZE &&  aptr->offset < (aptr->sub_page_index + 1) * SUB_PAGE_SIZE; //ISVALID_SUB_PAGES(aptr->offset);

	if (likely(b)){
		return (unsigned char*)((uint64_t)*aptr->prm_ptr + aptr->offset);// - aptr->sub_page_index * SUB_PAGE_SIZE);
	}else{
		// first - get the correct aptr page & offset
		aptr_fsync(aptr, base_page_index);
		return deref_direct(aptr, base_page_index);
	}
#endif
}

#endif /* TRUSTEDLIB_LIB_SERVICES_STATIC_TRUSTED_APTR_H_ */
