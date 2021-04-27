

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include <string.h>
#include <libpmem.h>

/* Copying 4K at a time to pmem for this example */
#define BUF_LEN 4096

int main(int argc, char *argv[]) {

    printf("Hello, World!\n");
    printf("%s",argv[0]);
    return 0;
}
