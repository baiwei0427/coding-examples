#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>

static void *ibv_handle = NULL;
static int (*real_ibv_modify_qp)(struct ibv_qp *qp, struct ibv_qp_attr *attr, int mask) = NULL;
static struct ibv_context* (*real_ibv_open_device)(struct ibv_device *device) = NULL;

int ibv_modify_qp(struct ibv_qp *qp, struct ibv_qp_attr *attr, int mask) {
    if (!ibv_handle) {
        ibv_handle = dlopen("libibverbs.so.1", RTLD_NOW);
        if (!ibv_handle) {
            printf("libibverbs.so.1 not found\n");
            return -1;
        } else {
            printf("libibverbs.so.1 found\n");
        }
    }

    if (!real_ibv_modify_qp) {
        real_ibv_modify_qp = dlsym(ibv_handle, "ibv_modify_qp");

        if (!real_ibv_modify_qp) {
            printf("ibv_modify_qp not found\n");
            return -1;
        }
    }

    printf("ibv_modify_qp intercepted\n");
    int ret = real_ibv_modify_qp(qp, attr, mask);
    printf("ibv_modify_qp returns %d\n", ret);

    return ret;
}

struct ibv_context *ibv_open_device(struct ibv_device *device)
{
    if (!ibv_handle) {
        ibv_handle = dlopen("libibverbs.so.1", RTLD_NOW);
        if (!ibv_handle) {
            printf("libibverbs.so.1 not found\n");
            return NULL;
        } else {
            printf("libibverbs.so.1 found\n");
        }
    }

    if (!real_ibv_open_device) {
        real_ibv_open_device = dlsym(ibv_handle, "ibv_open_device");

        if (!real_ibv_open_device) {
            printf("ibv_open_device not found\n");
            return NULL;
        }
    }

    printf("ibv_open_device intercepted\n");
    struct ibv_context *ret = real_ibv_open_device(device);
    printf("ibv_open_device returns %p\n", ret);

    return ret;
}
