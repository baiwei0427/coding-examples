#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    double progress = 0;
    int bar_width = 20;
    int pos = 0;
    int i;

    while (progress <= 1.0)
    {
        printf("[");
        pos = bar_width * progress;
        for (i = 0; i < bar_width; i++)
        {
            if (i < pos)
                printf("=");
            else
                printf(" ");
        }
        printf("] %d %\r", (int)(progress * 100));
        fflush(stdout);

        progress += 1.0/bar_width;
        sleep(1);
    }
    printf("\n");

    return 0;
}
