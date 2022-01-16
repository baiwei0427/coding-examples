#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#define DEFAULT_MSG_SIZE 32000

#define MAX_MSG_SIZE 1048576

#define DEFAULT_ITERS 100

#define DEFAULT_SERVER_PORT 5001

// Write exactly 'count' bytes storing in buffer 'buf' into
// the file descriptor 'fd'.
// Return the number of bytes sucsessfully written.
size_t write_exact(int fd, char *buf, size_t count);

// Read exactly 'count' bytes from the file descriptor 'fd'
// and store the bytes into buffer 'buf'.
// Return the number of bytes successfully read.
size_t read_exact(int fd, char *buf, size_t count);

#endif
