#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "message.h"

int send_string(int fd, char* str) {
    size_t len = strlen(str);
    size_t wlen = write(fd, &len, sizeof(len));
    if(wlen == (size_t)(-1)) {
        fprintf(stderr, "Error: write to %d failed, %s\n", fd, strerror(errno));
        return -1;
    }
    wlen = 0;
    while (wlen < len) {
        size_t w = write(fd, &str[wlen], len - wlen);
        if(w == (size_t)(-1)) {
            fprintf(stderr, "Error: write to %d failed, %s\n", fd, strerror(errno));
            return -1;
        }
        wlen += w;
    }
    return wlen;
}

char* recv_string(int fd) {
    size_t len;
    size_t rlen = read(fd, &len, sizeof(len));
    if(rlen == (size_t)(-1)) {
        fprintf(stderr, "Error: read from %d failed, %s\n", fd, strerror(errno));
        return NULL;
    } else if (rlen == 0) {
        fprintf(stderr, "Error: unexpected EOF reading from %d\n", fd);
        return NULL;
    } else if (rlen != sizeof(len)) {
        fprintf(stderr, "Error: expected %lu bytes from %d but only %lu found\n", sizeof(len), fd, rlen);
        return NULL;
    }
    char* buf = malloc(len + 1);
    if(!buf) {
        fprintf(stderr, "Error: failed alloc of %lu bytes, %s\n", len + 1, strerror(errno));
        return NULL;
    }
    rlen = 0;
    while (rlen < len) {
        size_t r = read(fd, &buf[rlen], len - rlen);
        if(r == (size_t)(-1)) {
            fprintf(stderr, "Error: read from %d failed, %s\n", fd, strerror(errno));
            return NULL;
        } else if (r == 0) {
            fprintf(stderr, "Error: unexpected EOF reading from %d\n", fd);
            return NULL;
        }
        rlen += r;
    }
    buf[len] = '\0';
    return  buf;
}

int send_argv(int fd, char* argv[]) {
    size_t len = 0;
    while(argv[len]) ++len;
    size_t wlen = write(fd, &len, sizeof(len));
    if(wlen == (size_t)(-1)) {
        fprintf(stderr, "Error: write to %d failed, %s\n", fd, strerror(errno));
        return -1;
    }
    for(size_t i=0; i < len; ++i) {
        size_t w = send_string(fd, argv[i]);
        wlen += w;
    }
    return wlen;
}

char** recv_argv(int fd){
    size_t len;
    size_t rlen = read(fd, &len, sizeof(len));
    if(rlen == (size_t)(-1)) {
        fprintf(stderr, "Error: read from %d failed, %s\n", fd, strerror(errno));
        return NULL;
    } else if (rlen == 0) {
        fprintf(stderr, "Error: unexpected EOF reading from %d\n", fd);
        return NULL;
    } else if (rlen != sizeof(len)) {
        fprintf(stderr, "Error: expected %lu bytes from %d but only %lu found\n", sizeof(len), fd, rlen);
        return NULL;
    }
    char** argv = calloc(len + 1, sizeof(char*));
    if(!argv) {
        fprintf(stderr, "Error: failed alloc of %lu bytes, %s\n", (len + 1)*sizeof(char*), strerror(errno));
        return NULL;
    }
    for(size_t i=0; i < len; ++i) {
        argv[i] = recv_string(fd);
    }
    return argv;
}

