
#include <stdio.h>
#include <string.h>
#include <libpmemobj.h>
#include "layout.h"

#define LAYOUT_NAME "intro_0"
#define MAX_BUF_LEN 10

struct my_root {
    size_t len;
    char buf[MAX_BUF_LEN];
};

int main(int argc,char *argv[]){
    PMEMobjpool *pop =pmemobj_create(argv[1],LAYOUT_NAME,PMEMOBJ_MIN_POOL,0666);
    if(pop==NULL){
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid root =pmemobj_root(pop,sizeof(my_root));
    struct my_root *rootp =pmemobj_direct(root);

    char buf[MAX_BUF_LEN];
    scanf_s("%9s",buf);



    rootp->len =strlen(buf);
    pmemobj_persist(pop,&rootp->len,sizeof(rootp->len) );
    pmemobj_memcpy_persist(pop,rootp->buf,my_buf,rootp->len);


    pmemobj_close(pop);
    return 0;
}