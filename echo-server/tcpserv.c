#include "unp.h"

void sig_chld(int signo)
{
        pid_t pid;
        int stat;

        while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                printf("child %d terminated\n", pid);
}

void str_echo(int sockfd)
{
        ssize_t n;
        char buf[MAXLINE];

        while (1) {
                n = read(sockfd, buf, MAXLINE);
                if (n > 0) {
                        writen(sockfd, buf, n);
                } else if (n == 0 || (n < 0 && errno != EINTR)) {
                        break;
                }
        }
}

int main(int argc, char **argv)
{
        int listenfd, connfd;
        pid_t childpid;
        socklen_t clilen;
        struct sockaddr_in cliaddr, servaddr;

        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("Error: socket()\n");
                return 0;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(SERV_PORT);

        if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                printf("Error: bind()\n");
                return 0;
        }

        if (listen(listenfd, LISTENQ) < 0) {
                printf("Error: listen()\n");
                return 0;
        }

        if (signal(SIGCHLD, sig_chld) == SIG_ERR) {
                printf("Error: signal()\n");
                return 0;
        }

        while (1) {
                clilen = sizeof(cliaddr);
                if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                        if (errno == EINTR)     /* call accept() again */
                                continue;
                        else
                                return 0;
                }

                if ((childpid = fork()) == 0) { /* child process */
                        close(listenfd);
                        str_echo(connfd);       /* process the request */
                        close(connfd);
                        exit(0);
                }
                printf("start child process %d\n", childpid);
                close(connfd);
        }
}
