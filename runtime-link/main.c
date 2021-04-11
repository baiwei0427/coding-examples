#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int x[2] = {1, 2};
int y[2] = {3, 4};
int z[2];

int main(int argc, char **argv)
{
    void *handle;
    void (*addvec)(int *, int *, int *, int);
    void (*multvec)(int *, int *, int *, int);
    char *error;

    handle = dlopen("./libvector.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    addvec = dlsym(handle, "addvec");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    multvec = dlsym(handle, "multvec");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    addvec(x, y, z, 2);
    printf("z = x + y = [%d %d]\n", z[0], z[1]);

    multvec(x, y, z, 2);
    printf("z = x * y = [%d %d]\n", z[0], z[1]);

    if (dlclose(handle) < 0) {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    return 0;
}
