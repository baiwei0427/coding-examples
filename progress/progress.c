#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    int progress = 0;
    int max_progress = 100;
    int bar_width = 20;
    int pos = 0;
    int i;

    while (progress <= max_progress)
    {
        printf("[");
        pos = bar_width * progress / max_progress;
        for (i = 0; i < bar_width; i++)
        {
            if (i < pos)
                printf("=");
            else
                printf(" ");
        }
        printf("] %d %\r", progress);
        fflush(stdout);

        progress += max_progress/bar_width;
        sleep(1);
    }
    printf("\n");

    return 0;
}
