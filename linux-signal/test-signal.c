#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
        if (signo == SIGINT)
                printf("received SIGINT\n");
}

int main(void)
{
        int i = 0;
        if (signal(SIGINT, sig_handler) == SIG_ERR) {
                printf("Can't catch SIGINT\n");
                return 0;
        }

        while (i++ < 30) {
                printf("%d\n", i);
                sleep(1);
        }

        return 0;
}
