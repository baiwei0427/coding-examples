#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

static void *(*mallocp)(size_t) = NULL;
static void (*freep)(void*) = NULL;

void* malloc(size_t size)
{
    char *error;

    if (!mallocp) {
        mallocp = dlsym(RTLD_NEXT, "malloc");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(EXIT_FAILURE);
        }
    }

    void *ptr = mallocp(size);
    fprintf(stderr, "malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

void free(void *ptr)
{
    char *error;

    if (!ptr)
        return;

    if (!freep) {
        freep = dlsym(RTLD_NEXT, "free");
        if ((error = dlerror()) != NULL) {
            fputs(error, stderr);
            exit(EXIT_FAILURE);
        }
    }

    freep(ptr);
    fprintf(stderr, "free(%p)\n", ptr);
}
