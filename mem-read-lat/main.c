#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef void *ptr;

static void usage(char *program);
static void test_cache(size_t mem_size, size_t iters);
static void shuffle(ptr *array, size_t n);
static double ns_since(const struct timespec t0);

#define ONE p = (ptr*)*p;
#define FIVE ONE ONE ONE ONE ONE
#define TEN FIVE FIVE
#define FIFTY TEN TEN TEN TEN TEN
#define HUNDRED FIFTY FIFTY

int main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    size_t mem_size, iters;

    if (sscanf(argv[1], "%zu", &mem_size) != 1) {
        fprintf(stderr, "Fail to get memory size\n");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (sscanf(argv[2], "%zu", &iters) != 1) {
        fprintf(stderr, "Fail to get the number of iterations");
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    test_cache(mem_size, iters);
    return EXIT_SUCCESS;
}

static void usage(char *program)
{
    printf("usage: %s memory_size number_of_iterations\n", program);
    printf("    memory_size: size of allocated memory in bytes\n");
    printf("    number_of_iterations: number of memory read iterations\n");
}

static void test_cache(size_t mem_size, size_t iters)
{
    unsigned int array_size  = mem_size / sizeof(ptr);
    ptr *array = (ptr*)malloc(array_size * sizeof(ptr));
    if (!array) {
        fprintf(stderr, "Fail to allocate %lu B memory\n", array_size * sizeof(ptr));
        return;
    }

    for (size_t i = 0; i < array_size; i++) {
        array[i] = (ptr)&array[i];
    }

    // Shuffle the array so that array[i] stores a random valid address
    shuffle(array, array_size);

    ptr *p = array;
    size_t rounds = iters / 100;
    size_t remain_iters = iters - rounds * 100;

    struct timespec bench_start;
    clock_gettime(CLOCK_REALTIME, &bench_start);

    for (size_t i = 0; i < rounds; i++) {
        HUNDRED;
    }

    for (size_t i = 0; i < remain_iters; i++) {
        ONE;
    }

    // We need to use the pointer to ensure that the
    // compiler does not considier p as a dead variable.
    volatile int result = (int)*p;

    double bench_ns = ns_since(bench_start);
    printf("Average read latency = %.3f ns\n", bench_ns / iters);
    free(array);
}

static void shuffle(ptr *array, size_t n)
{
    if (!array || n <= 1) {
        return;
    }

    // Use current time as seed for random generator
    srand(time(0));

    for (size_t i = 0; i < n - 1; i++) {
        // Generate a random number in [i, n-1]
        size_t j = i + rand() % (n - i);

        ptr tmp = array[i];
        array[i] = array[j];
        array[j] = tmp;
    }
}

static double ns_since(const struct timespec t0)
{
    struct timespec t1;
    clock_gettime(CLOCK_REALTIME, &t1);
    return (t1.tv_sec - t0.tv_sec) * 1000000000.0 + (t1.tv_nsec - t0.tv_nsec);
}
