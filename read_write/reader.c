
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
    PMEMobjpool *pop = pmemobj_open(argv[1], LAYOUT_NAME);
    if (pop == NULL) {
        perror("pmemobj_open");
        return 1;
    }

    PMEMoid root = pmemobj_root(pop, sizeof (struct my_root));
    struct my_root *rootp = pmemobj_direct(root);

    if (rootp->len == strlen(rootp->buf))
        printf("%s\n", rootp->buf);

    rootp->len=10;
    pmemobj_close(pop);

    return 0;
}
