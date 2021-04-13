#include <malloc.h>

int main(int argc, char *argv[])
{
    void *p = malloc(32);
    free(p);

    return 0;
}
