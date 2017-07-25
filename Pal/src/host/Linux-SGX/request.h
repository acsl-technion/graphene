#ifndef _REQUEST_H
#define _REQUEST_H

typedef struct request_t
{
	int result;
        int ocall_index;
        void* buffer;
        int volatile is_done;
} request;

#endif

