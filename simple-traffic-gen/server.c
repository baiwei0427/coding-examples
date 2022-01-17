#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <arpa/inet.h>
#include "common.h"

static void print_usage(char *app);
static bool parse_args(int argc, char **argv);

static uint16_t server_port = DEFAULT_SERVER_PORT;

int main(int argc, char **argv)
{
    int sockfd = -1;
    int connection = -1;

    if (!parse_args(argc, argv)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    //print("Listen on 0.0.0.0:%hu\n", server_port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "TCP socket creation error\n");
        goto err;
    }

    // To allow reuse of local addresses
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt))) {
        fprintf(stderr, "Set socket option error\n");
        goto err;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(server_port);
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Bind error\n");
        goto err;
    }

    if (listen(sockfd, 5) < 0) {
        fprintf(stderr, "Listen error\n");
        goto err;
    }

    printf("Listen on 0.0.0.0:%hu\n", server_port);

    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    connection = accept(sockfd, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
    if (connection < 0) {
        fprintf(stderr, "Fail to accept an connection\n");
        goto err;
    }

    char write_buf[DEFAULT_BUF_SIZE] = {0};
    unsigned int iters = 0;
    while (1) {
        unsigned int msg_size;
        char *read_buf = (char*)&msg_size;
        size_t read_buf_size = sizeof(msg_size);

        if (read_exact(connection, read_buf, read_buf_size, read_buf_size, false) != read_buf_size) {
            fprintf(stderr, "Fail to read message size from the client\n");
            goto err;
        }

        if (msg_size == 0) {
            break;
        }

        if (write_exact(connection, write_buf, msg_size, sizeof(write_buf), true) != msg_size) {
            fprintf(stderr, "Fail to send a %u-bytes message\n", msg_size);
            goto err;
        }
        iters++;
    }

    printf("Send %u messages in total\n", iters);
    close(connection);
    close(sockfd);
    return EXIT_SUCCESS;

err:
    if (connection >= 0) {
        close(connection);
    }
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
    fprintf(stderr, "  -h, --help                   show this help screen\n");
}

static bool parse_args(int argc, char **argv)
{
    while (1) {
        static struct option long_options[] = {
            { .name = "port",            .has_arg = 1, .val = 'p' },
            { .name = "help",            .has_arg = 0, .val = 'h' },
            {}
        };

        int c = getopt_long(argc, argv, "p:h", long_options, NULL);

        if (c == -1) {
            break;
        }

        switch (c) {
            case 'p':
                server_port = (uint16_t)strtoul(optarg, NULL, 0);
                break;

            case 'h':
            default:
                print_usage(argv[0]);
                return false;
        }
    }

    return true;
}
