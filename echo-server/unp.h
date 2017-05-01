#ifndef __UNP_H
#define __UNP_H

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

#define SERV_PORT 12345
#define LISTENQ 5
#define MAXLINE 1024

/* read "n" bytes from a descriptor, return # of bytes that have been read */
ssize_t readn(int fd, char *ptr, size_t n)
{
        ssize_t nleft = n, nread;

        while (nleft > 0) {
                nread = read(fd, ptr, nleft);
                if (nread < 0) {
                        if (errno = EINTR) {
                                nread = 0;      /* and call read() again */
                        } else {
                                return -1;
                        }
                } else if (nread == 0) {
                        break;
                }
                nleft -= nread;
                ptr += nread;
        }

        return n - nleft;
}

/* write "n" bytes to a descriptor */
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

/* read a line (end up with '\n') from a descriptor */
ssize_t readline(int fd, char *ptr, size_t maxlen)
{
        ssize_t n = 0, rc;

        while (n < maxlen - 1) {
                rc = read(fd, ptr, maxlen - n - 1);
                if (rc > 0) {
                        ptr += rc;
                        n += rc;
                        if (*(ptr - 1) == '\n')
                                break;  /* newline is stored */
                } else if (rc == 0) {
                        break;
                } else if (errno != EINTR) {
                        return -1;
                }
        }

        *ptr = 0;
        return n;
}

#endif
