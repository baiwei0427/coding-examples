#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int binary_search(int *array, int len, int target)
{
        int start = 0, end = len - 1, mid;

        while (start <= end) {
                mid = start + (end - start) / 2;

                #ifdef DO_PREFETCH
                // low path
                __builtin_prefetch(&array[mid + 1 + (end - mid - 1) / 2], 0, 1);
                // high path
                __builtin_prefetch(&array[start + (mid - start - 1) / 2], 0, 1);
                #endif

                if (array[mid] == target) {
                        return mid;
                } else if (array[mid] < target) {
                        start = mid + 1;
                } else {
                        end = mid - 1;
                }
        }

        return -1;
}

int main()
{
        int array_size = 1 << 28;
        int lookup_size = 1 << 23;
        int *array =  malloc(array_size * sizeof(int));
        int *lookups = malloc(lookup_size * sizeof(int));

        if (!array || !lookups) {
                free(array);
                free(lookups);
                return EXIT_FAILURE;
        }

        // build a sorted array
        for (int i = 0; i < array_size; i++) {
                array[i] = i;
        }

        srand(0);
        for (int i = 0; i < lookup_size; i++) {
                lookups[i] = random() % array_size;
        }

        for (int i = 0; i < lookup_size; i++) {
                binary_search(array, array_size, lookups[i]);
        }

        free(array);
        free(lookups);
        return EXIT_SUCCESS;
}