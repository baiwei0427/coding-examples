#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
struct timespec time_start={0, 0},time_end={0, 0};

int main()
{
        if ((sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER)) < 0) {
                return -1;
        }

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid();     /* self pid */
        bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0;   /* For Linux Kernel */
        dest_addr.nl_groups = 0;        /* unicast */

        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;

        strcpy(NLMSG_DATA(nlh), "Hello");
        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        clock_gettime(CLOCK_REALTIME, &time_start);
        sendmsg(sock_fd, &msg, 0);
        recvmsg(sock_fd, &msg, 0);
        clock_gettime(CLOCK_REALTIME, &time_end);
        printf("duration:%llus %lluns\n", time_end.tv_sec - time_start.tv_sec, time_end.tv_nsec - time_start.tv_nsec);
        close(sock_fd);

        return 0;
}
