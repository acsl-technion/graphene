/* a simple helloworld test */

#include <stdio.h>
#include <assert.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */


#define __NR_cosmix_untrusted_alloc (310)

int main(int argc, char** argv) {
    //printf("Hello world (%s)!\n", argv[0]);
    void* umem = NULL;
    int ret = syscall(__NR_cosmix_untrusted_alloc, 0x80000000, &umem);
    printf("Hello world, untrusted alloc ptr is %p ret is %d\n", umem, ret);

memset(umem, 0xff, 0x80000000);
    	
    return 0;
}
