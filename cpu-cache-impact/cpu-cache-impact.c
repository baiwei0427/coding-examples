#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int array_size = 64 * 1024 * 1024;

int main()
{
        struct timespec tstart, tend;
        int *array, i, k;
        double time;

        if (!(array = malloc(array_size * sizeof(int)))) {
                fprintf(stderr, "Error: malloc\n");
                return EXIT_FAILURE;
        }

        // initialize array to all zeros
        memset(array, 0, array_size * sizeof(int));

        for (k = 1; k <= 1024; k <<= 1) {
                clock_gettime(CLOCK_MONOTONIC, &tstart);
                for (i = 0; i < array_size; i += k) {
                        array[i] += 1;
                }
                clock_gettime(CLOCK_MONOTONIC, &tend);
                
                // calculate elapsed time in millisecond
                time = (tend.tv_sec - tstart.tv_sec) * 1000.0 + (tend.tv_nsec - tstart.tv_nsec) / 1000000.0;
                printf("%d : %.3f ms\n", k, time);
        }

        free(array);
        return EXIT_SUCCESS;
}