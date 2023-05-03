#include "message.h"

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
    
const char* LOCK_FILE = "/tmp/taskd.pid";
const char* PIPE_FILE = "/tmp/tasks.fifo";
const char* TEXT_FILE = "/tmp/tasks.txt";
const char* TASK_DIR = "/tmp/tasks/";

void exit_handler(int _, void* __) {
    unlink(LOCK_FILE);
}

void check_lock(void) {
    const size_t BUFLEN = 32;
    FILE* file = fopen(LOCK_FILE, "r");
    if(file) {
        char buf[BUFLEN];
        size_t len = fread(buf, sizeof(char), BUFLEN - 1, file);
        fclose(file);
        buf[len] = '\0';
        fprintf(stderr, "Error : taskd is already running with pid %s\n", buf);
        exit(1);
    }

    file = fopen(LOCK_FILE, "w");
    fprintf(file, "%d", getpid());
    fclose(file);
}

int main(int argc, char** argv) {
    check_lock();

    
    int pipe = open(PIPE_FILE, O_RDONLY | O_CREAT);
    if(pipe == -1) {
        fprintf(stderr, "Error : failed to open %s, %s\n", PIPE_FILE, strerror(errno));
        unlink(LOCK_FILE);
        return 1;
    }
    
    if(access(TEXT_FILE, W_OK)) {
        fprintf(stderr, "Error : failed to access %s\n", PIPE_FILE);
        unlink(LOCK_FILE);
        return 1;
    }
    

    return 0;
}
