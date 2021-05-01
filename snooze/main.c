#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void sighandler(int sig)
{
    return;
}

unsigned int snooze(unsigned int secs)
{
    unsigned int rc = sleep(secs);
    printf("Slept for %u of %u secs.\n", secs - rc, secs);
    return rc;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <secs>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, sighandler) == SIG_ERR) {
        fprintf(stderr, "signal error\n");
        return EXIT_FAILURE;
    }

    snooze((unsigned int)atoi(argv[1]));
    return EXIT_SUCCESS;
}
