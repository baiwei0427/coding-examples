#include "unp.h"

void str_cli(FILE *fp, int sockfd)
{
        char sendline[MAXLINE], recvline[MAXLINE];

        while (fgets(sendline, MAXLINE, fp)) {
                writen(sockfd, sendline, strlen(sendline));
                if (readline(sockfd, recvline, MAXLINE) > 0) {
                        fputs(recvline, stdout);
                } else {
                        break;
                }
        }
}

int main(int argc, char **argv)
{
        int sockfd;
        struct sockaddr_in servaddr;

        if (argc != 2) {
                printf("usage: %s <IP address>\n", argv[0]);
                return 0;
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("Error: socket()\n");
                return 0;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERV_PORT);
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
                printf("Error: inet_pton()\n");
                return 0;
        }

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
                printf("Error: connect()\n");
                return 0;
        }

        str_cli(stdin, sockfd);

        return 0;
}
