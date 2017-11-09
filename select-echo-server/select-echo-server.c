#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define LISTENQ 5
#define BUF_SIZE 1024

// write "n" bytes to a descriptor 
ssize_t writen(int fd, char *ptr, size_t n)
{
        ssize_t nleft = n, nwritten;

        while (nleft > 0) {
                if ((nwritten = write(fd, ptr, nleft)) <= 0 ) {
                        if (nwritten < 0 && errno == EINTR) {
                                nwritten = 0;   /* and call write() again */
                        } else {
                                return -1;
                        }
                }

                nleft -= nwritten;
                ptr += nwritten;
        }

        return n;
}

void usage(char *program)
{
        fprintf(stderr, "usage: %s port\n", program);
}

int main(int argc, char **argv)
{
        int nready, i, maxi, port, maxfd, listenfd, connfd, sockfd;
        socklen_t clilen;
        struct sockaddr_in cliaddr, servaddr;
        int clients[FD_SETSIZE];
        char buf[BUF_SIZE];
        fd_set rset, allset;
        ssize_t n;

        if (argc != 2) {
                usage(argv[0]);
                return EXIT_FAILURE;
        }

        // Get TCP port number 
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
                fprintf(stderr, "Invalid port number %d\n", port);
                return EXIT_FAILURE;
        }

        // Create listen socket 
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                fprintf(stderr, "Error: socket\n");
                return EXIT_FAILURE;
        } else {
                printf("Create listen socket %d\n", listenfd);
        }

        // Initialize server socket address
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(port);

        // Bind socket to an address
        if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                fprintf(stderr, "Error: bind\n");
                return EXIT_FAILURE;
        }

        // Listen
        if (listen(listenfd, LISTENQ) < 0) {
                fprintf(stderr, "Error: listen\n");
                return EXIT_FAILURE;
        }

        // Maximum file descriptor so far
        maxfd = listenfd;
        // index into clients[] array
        maxi = -1;

        for (i = 0; i < FD_SETSIZE; i++) {
                clients[i] = -1;        // -1 indicates available entry
        }

        FD_ZERO(&allset);
        FD_SET(listenfd, &allset);

        while (1) {
                rset = allset;
                nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

                if (nready <= 0) {
                        continue;
                } 

                // Check new connection
                if (FD_ISSET(listenfd, &rset)) {
                        clilen = sizeof(cliaddr);
                        if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0) {
                                fprintf(stderr, "Error: accept\n");
                                return EXIT_FAILURE;
                        }

                        printf("Accept socket %d (%s : %hu)\n", 
                               connfd, 
                               inet_ntoa(cliaddr.sin_addr),
                               ntohs(cliaddr.sin_port));

                        // Save client socket into clients array
                        for (i = 0; i < FD_SETSIZE; i++) {
                                if (clients[i] < 0) {
                                        clients[i] = connfd;
                                        break;
                                }
                        }

                        // No enough space in clients array
                        if (i == FD_SETSIZE) {
                                fprintf(stderr, "Error: too many clients\n");
                                close(connfd);
                        }

                        // Add new descriptor to set 
                        FD_SET(connfd, &allset);

                        if (connfd > maxfd) {
                                maxfd = connfd;
                        }

                        if (i > maxi) {
                                maxi = i;
                        }

                        // No more readable file descriptors
                        if (--nready <= 0) {
                                continue;
                        }
                } 

                // Check all clients to read data
                for (i = 0; i <= maxi; i++) {
                        if ((sockfd = clients[i]) < 0) {
                                continue;
                        }

                        // If the client is readable
                        if (FD_ISSET(sockfd, &rset)) {
                                n = read(sockfd, buf, BUF_SIZE);
                                
                                if (n < 0) {
                                        fprintf(stderr, "Error: read from socket %d\n", sockfd);
                                        close(sockfd);
                                        FD_CLR(sockfd, &allset);
                                        clients[i] = -1;
                                } else if (n == 0) {    // connection closed by client
                                        printf("Close socket %d\n", sockfd);
                                        close(sockfd);
                                        FD_CLR(sockfd, &allset);
                                        clients[i] = -1;                                        
                                } else {
                                        printf("Read %zu bytes from socket %d\n", n, sockfd);
                                        writen(sockfd, buf, n);
                                }

                                // No more readable file descriptors
                                if (--nready <= 0) {
                                        break;
                                }
                        }
                }

        }
        
        close(listenfd);
        return EXIT_SUCCESS;
}