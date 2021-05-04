
#include <stdio.h>
#include <string.h>
#include <libpmemobj.h>

#define LAYOUT_NAME "intro_0"
#define MAX_BUF_LEN 10

struct my_root {
    size_t len;
    int buf[MAX_BUF_LEN];
};

int main(int argc,char *argv[]){
    PMEMobjpool *pop =pmemobj_create(argv[1],LAYOUT_NAME,PMEMOBJ_MIN_POOL,0666);
    if(pop==NULL){
        perror("pmemobj_create");
        return 1;
    }

    PMEMoid root =pmemobj_root(pop,sizeof(struct my_root));
    struct my_root *rootp =pmemobj_direct(root);

    int buf[MAX_BUF_LEN]={1,2,3,4,5,6,7,8,9,10};


    rootp->len =strlen(buf);
    pmemobj_persist(pop,&rootp->len,sizeof(rootp->len) );
    pmemobj_memcpy_persist(pop,rootp->buf,buf,rootp->len);


    pmemobj_close(pop);
    return 0;
}