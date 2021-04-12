#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        void *p = malloc(atoi(argv[i]));
        free(p);
    }

    return 0;
}
