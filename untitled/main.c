

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

    int srcfd;
    struct stat stbuf;
    char *pmemaddr;
    size_t mapped_len;
    int is_pmem;

    printf("Hello, World!\n");
    printf("%s\n",argv[1]);
    if (argc != 2) {
        fprintf(stderr,
                "usage: %s src-file\n",
                argv[0]);
        exit(1);
    }

    if ((pmemaddr = pmem_map_file(argv[2],
                                  stbuf.st_size,
                                  PMEM_FILE_CREATE,
                                  0666, &mapped_len, &is_pmem)) == NULL) {
        perror("pmem_map_file");
        exit(1);
    }

    printf("%s\n");




    return 0;
}



















