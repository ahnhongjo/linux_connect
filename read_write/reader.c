
#include <stdio.h>
#include <string.h>
#include <libpmemobj.h>

#define LAYOUT_NAME "intro_0"
#define MAX_BUF_LEN 10

struct my_root {
    size_t len;
    char buf[MAX_BUF_LEN];
};

int main(int argc, char *argv[])
{

    char* path=argv[1]
    int n=strlen(argv[1]);

    for(int i=0;i<n;i++){
        char* tmp=""
        if (path[i]=="/"){
            printf("%s",tmp);
            tmp="";
        }
        else{
            strcat(tmp,path[i]);
        }
    }

    PMEMobjpool *pop = pmemobj_open(path, LAYOUT_NAME);
    if (pop == NULL) {
        perror("pmemobj_open");
        return 1;
    }

    PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);

    if (rootp->len == strlen(rootp->buf))
        printf("%s\n", rootp->buf);

    pmemobj_close(pop);

    return 0;
}