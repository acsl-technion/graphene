#ifndef _DATA_STRUCTURES_HPP_INCLUDED_
#define	_DATA_STRUCTURES_HPP_INCLUDED_

// First version - can use custom sizes for each item in future...
#define PAGE_SIZE 4096
#define EPC_SIZE 134217728 // 128 MB
#define CACHE_SIZE 73400320 // Try set to 64 MB of the 128 MB (since enclave driver should take about 30 MB) - still no mem-pining
#define CACHE_CAPACITY (CACHE_SIZE / PAGE_SIZE)
#define NONCE_BYTE_SIZE 12

//typedef struct _CryptoMetadata
//{
//	bool is_initialized;
//	size_t size;
//	uint8_t nonce[NONCE_BYTE_SIZE];
//	sgx_aes_gcm_128bit_tag_t mac;
//	uint8_t ref_count;
//
//} CryptoMetadata;
//
//typedef struct _PageMetadata
//{
//	bool is_dirty;
//	void* prm_pointer;
//	std::list<void*>* active_pointer_list;
//
//} PageMetadata;

#endif


