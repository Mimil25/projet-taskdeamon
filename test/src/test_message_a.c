#include "../../src/message.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char** argv) {
    if(argc < 2) {
        return 1;
    }
    int fd = open(argv[1], O_WRONLY);
    send_argv(fd, &argv[2]);
    close(fd);
    return 0;
}
