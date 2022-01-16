#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "common.h"

static void print_usage(char *app);
static bool parse_args(int argc, char **argv);

static uint16_t server_port     = DEFAULT_SERVER_PORT;
static unsigned int iters       = DEFAULT_ITERS;
static unsigned int msg_size    = DEFAULT_MSG_SIZE;
static char *server_ip          = NULL;

int main(int argc, char **argv)
{
    int sockfd = -1;
    struct timeval tv_start, tv_end;

    if (!parse_args(argc, argv)) {
        return EXIT_FAILURE;
    }

    // Create socket file descriptor
    if ((sockfd =  socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "TCP socket creation error\n");
        goto err;
    }

    // Initialize server socket address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server address %s\n", server_ip);
        goto err;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Fail to connect to %s:%hu\n", server_ip, server_port);
        goto err;
    }

    char read_buf[MAX_MSG_SIZE];
    char *write_buf = (char*)&msg_size;
    size_t write_buf_size = sizeof(msg_size);

    if (gettimeofday(&tv_start, NULL)) {
		fprintf(stderr, "Cannot get start time\n");
		goto err;
	}

    for (unsigned i = 0; i < iters; i++) {
        if (write_exact(sockfd, write_buf, write_buf_size) != write_buf_size) {
            fprintf(stderr, "Fail to send message size to %s:%hu\n", server_ip, server_port);
            goto err;
        }

        if (read_exact(sockfd, read_buf, msg_size) != msg_size) {
            fprintf(stderr, "Fail to receive %u bytes data from %s:%hu\n", msg_size, server_ip, server_port);
            goto err;
        }
    }

    if (gettimeofday(&tv_end, NULL)) {
		fprintf(stderr, "Cannot get end time\n");
		goto err;
	}

    unsigned int complete_msg = 0;
    write_buf = (char*)&complete_msg;
    if (write_exact(sockfd, write_buf, write_buf_size) != write_buf_size) {
        fprintf(stderr, "Fail to send completion message to %s:%hu\n", server_ip, server_port);
        goto err;
    }

    printf("Request %u %u-bytes messages from %s:%hu\n", iters, msg_size, server_ip, server_port);
    float total_usec = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + (tv_end.tv_usec - tv_start.tv_usec);
    printf("Average message completion time: %.2f usec\n", total_usec / iters);

    close(sockfd);
    return EXIT_SUCCESS;

err:
    if (sockfd >= 0) {
        close(sockfd);
    }
    return EXIT_FAILURE;
}

static void print_usage(char *app)
{
    if (!app) {
        return;
    }

    fprintf(stderr, "Usage: %s [options] host\n", app);
    fprintf(stderr, "Options:\n");

    fprintf(stderr, "  -p, --port=<port>            listen on port <port> (default %d)\n", DEFAULT_SERVER_PORT);
    fprintf(stderr, "  -n, --iters=<iters>          number of messages (default %d)\n", DEFAULT_ITERS);
    fprintf(stderr, "  -s, --size=<size>            size of message (default %d)\n", DEFAULT_MSG_SIZE);
    fprintf(stderr, "  -h, --help                   show this help screen\n");
}

static bool parse_args(int argc, char **argv)
{
    while (1) {
        static struct option long_options[] = {
            { .name = "port",   .has_arg = 1, .val = 'p' },
            { .name = "iters",  .has_arg = 1, .val = 'n' },
            { .name = "size",   .has_arg = 1, .val = 's' },
            { .name = "help",   .has_arg = 0, .val = 'h' },
            {}
        };

        int c = getopt_long(argc, argv, "p:n:s:h", long_options, NULL);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                server_port = (uint16_t)strtoul(optarg, NULL, 0);
                break;

            case 'n':
                iters = (unsigned int)strtoul(optarg, NULL, 0);
                if (iters == 0) {
                    fprintf(stderr, "iters should be larger than 0\n");
                    print_usage(argv[0]);
                    return false;
                }
                break;

            case 's':
                msg_size = (unsigned int)strtoul(optarg, NULL, 0);
                if (msg_size > MAX_MSG_SIZE) {
                    fprintf(stderr, "Max message size is %u\n", MAX_MSG_SIZE);
                    print_usage(argv[0]);
                    return false;

                } else {
                    break;
                }

            case 'h':
            default:
                print_usage(argv[0]);
                return false;
        }
    }

    if (optind == argc - 1) {
        server_ip = argv[optind];
    }

    if (!server_ip) {
        fprintf(stderr, "Fail to get host\n");
        print_usage(argv[0]);
        return false;
    }

    return true;
}
