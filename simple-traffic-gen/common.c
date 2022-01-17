
#include "common.h"

size_t write_exact(int fd, char *buf, size_t count, size_t max_per_write, bool dummy_buf)
{
    // total # of bytes that have been written
    size_t bytes_written = 0;
    // max # of bytes to write in next write() call
    size_t bytes_to_write = 0;
    // current buffer loccation
    char *cur_buf = NULL;
    int n;

    if (unlikely(!buf)) {
        return 0;
    }

    while (count > 0) {
        bytes_to_write = (count > max_per_write) ? max_per_write : count;
        cur_buf = (dummy_buf) ? buf : (buf + bytes_written);
        n = write(fd, cur_buf, bytes_to_write);

        if (n <= 0) {
            fprintf(stderr, "write error\n");
            break;

        } else {
            bytes_written += n;
            count -= n;
        }
    }

    return bytes_written;
}

size_t read_exact(int fd, char *buf, size_t count, size_t max_per_read, bool dummy_buf)
{
    // total # of bytes that have been read
    size_t bytes_read = 0;
    // max # of bytes to readin next read() call
    size_t bytes_to_read = 0;
    // current buffer loccation
    char *cur_buf = NULL;
    int n;

    if (unlikely(!buf)) {
        return 0;
    }

    while (count > 0) {
        bytes_to_read = (count > max_per_read) ? max_per_read : count;
        cur_buf = (dummy_buf) ? buf : (buf + bytes_read);
        n = read(fd, cur_buf, bytes_to_read);

        if (n <= 0) {
            fprintf(stderr, "read error\n");
            break;

        } else {
            bytes_read += n;
            count -= n;
        }
    }

    return bytes_read;
}
