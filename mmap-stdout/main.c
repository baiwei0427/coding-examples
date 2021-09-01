#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
 #include <unistd.h>

void mmapcopy(int fd, int size);

int main(int argc, char **argv)
{
    struct stat stat_buf;
    int fd;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <filenamee>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd <= 0) {
        fprintf(stderr, "open %s fails\n", argv[1]);
        return EXIT_FAILURE;
    }

    fstat(fd, &stat_buf);
    mmapcopy(fd, stat_buf.st_size);

    return EXIT_SUCCESS;
}

void mmapcopy(int fd, int size)
{
    void *bufp = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (bufp == (void*)(-1)) {
        fprintf(stderr, "mmap fails\n");
        return;
    }

    if (write(1, bufp, size) != size) {
        fprintf(stderr, "write fails\n");
        return;
    }

    if (munmap(bufp, size) == -1) {
        fprintf(stderr, "mumap fails\n");
        return;
    }
}
