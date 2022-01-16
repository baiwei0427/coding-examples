
#include "common.h"

// Read exactly 'count' bytes from the file descriptor 'fd'
// and store the bytes into buffer 'buf'.
// Return the number of bytes successfully read.
size_t write_exact(int fd, char *buf, size_t count)
{
    // current buffer loccation
    char *cur_buf = NULL;
    // # of bytes that have been written
    size_t bytes_wrt = 0;
    int n;

    if (!buf) {
        return 0;
    }

    cur_buf = buf;

    while (count > 0) {
        n = write(fd, cur_buf, count);

        if (n <= 0) {
            fprintf(stderr, "write error\n");
            break;

        } else {
            bytes_wrt += n;
            count -= n;
            cur_buf += n;
        }
    }

    return bytes_wrt;
}

// Write exactly 'count' bytes storing in buffer 'buf' into
// the file descriptor 'fd'.
// Return the number of bytes sucsessfully written.
size_t read_exact(int fd, char *buf, size_t count)
{
    // current buffer loccation
    char *cur_buf = NULL;
    // # of bytes that have been read
    size_t bytes_read = 0;
    int n;

    if (!buf) {
        return 0;
    }

    cur_buf = buf;

    while (count > 0) {
        n = read(fd, cur_buf, count);

        if (n <= 0) {
            fprintf(stderr, "read error\n");
            break;

        } else {
            bytes_read += n;
            count -= n;
            cur_buf += n;
        }
    }

    return bytes_read;
}
