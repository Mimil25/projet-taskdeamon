#include "../../src/message.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    char** a = recv_argv(fd);
    close(fd);
    char **p = a;
    while (*p) {
        printf("%s\n", *p);
        free(*p);
        ++p;
    }
    free(a);
    return 0;
}
