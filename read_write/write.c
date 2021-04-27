/*
Copyright (c) 2018-2019 Intel Corporation

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 
SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * hello_libpmem.c -- an example for libpmem library
 */

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

/* using 1k of pmem for this example */
#define PMEM_LEN 1024

// Maximum length of our buffer
#define MAX_BUF_LEN 30

/****************************
 * This function writes the "Hello..." string to persistent-memory.
 *****************************/

static void
do_copy_to_pmem(char *pmemaddr, int srcfd, off_t len)
{
    char buf[BUF_LEN];
    int cc;

    /*
     * Copy the file,
     * saving the last flush & drain step to the end
     */
    while ((cc = read(srcfd, buf, BUF_LEN)) > 0) {
        pmem_memcpy_nodrain(pmemaddr, buf, cc);
        pmemaddr += cc;
    }

    if (cc < 0) {
        perror("read");
        exit(1);
    }

    /* Perform final flush step */
    pmem_drain();
}

/****************************
 * This main function gather from the command line and call the appropriate
 * functions to perform read and write persistently to memory.
 *****************************/
int main(int argc, char *argv[])
{
    char *path = argv[2];
    struct stat stbuf;
    char *pmemaddr;
    size_t mapped_len;
    int srcfd;
    int is_pmem;

    if (argc != 3) {
        fprintf(stderr,
                "usage: %s src-file dst-file\n",
                argv[0]);
        exit(1);
    }

    /* Open src-file */
    if ((srcfd = open(argv[1], O_RDONLY)) < 0) {
        perror(argv[1]);
        exit(1);
    }

    /* Find the size of the src-file */
    if (fstat(srcfd, &stbuf) < 0) {
        perror("fstat");
        exit(1);
    }

    /* create a pmem file and memory map it */
    if ((pmemaddr = pmem_map_file(argv[2],
                                  stbuf.st_size,
                                  PMEM_FILE_CREATE|PMEM_FILE_EXCL,
                                  0666, &mapped_len, &is_pmem)) == NULL) {
        perror("pmem_map_file");
        exit(1);
    }

    /*
  * Determine if range is true pmem,
  * call appropriate copy routine
  * */
    if (is_pmem)
        do_copy_to_pmem(pmemaddr, srcfd,
                        stbuf.st_size);

    close(srcfd);
    pmem_unmap(pmemaddr, mapped_len);

    exit(0);

}
