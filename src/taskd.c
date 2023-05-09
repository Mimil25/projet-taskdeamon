#include "message.h"

#include <asm-generic/errno-base.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

const char* LOCK_FILE = "/tmp/taskd.pid";
const char* PIPE_FILE = "/tmp/tasks.fifo";
const char* TEXT_FILE = "/tmp/tasks.txt";
const char* TASK_DIR = "/tmp/tasks/";

void check_lock(void) {
    const size_t BUFLEN = 32;
    FILE* file = fopen(LOCK_FILE, "r");
    if(file) {
        char buf[BUFLEN];
        size_t len = fread(buf, sizeof(char), BUFLEN - 1, file);
        fclose(file);
        buf[len] = '\0';
        fprintf(stderr, "Error : taskd is already running with pid %s\n", buf);
        exit(EXIT_FAILURE);
    } else if(errno == ENOENT) {
        file = fopen(LOCK_FILE, "w");
        fprintf(file, "%d", getpid());
        fclose(file);
    } else {
        fprintf(stderr, "Error : failed to open %s, %s\n", LOCK_FILE, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    check_lock();

    if(mkfifo(PIPE_FILE, 0666) && errno != EEXIST) {
        fprintf(stderr, "Error : failed to create %s, %s\n", PIPE_FILE, strerror(errno));
        unlink(LOCK_FILE);
        return 1;
    }

    int text_file = open(TEXT_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if(text_file == -1) {
        fprintf(stderr, "Error : failed to open %s, %s\n", TEXT_FILE, strerror(errno));
        unlink(LOCK_FILE);
        return 1;
    }
    close(text_file);

    if(mkdir(TASK_DIR, 0755) && errno != EEXIST) {
        fprintf(stderr, "Error : failed to create %s, %s\n", TASK_DIR, strerror(errno));
        unlink(LOCK_FILE);
        return 1;
    }
    
    return 0;
}
