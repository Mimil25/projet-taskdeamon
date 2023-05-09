#define _POSIX_SOURCE
#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    if(argc!=1 && argc<4){
        fprintf(stderr,"Wrong number of argument");
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    if(argc == 1){
        //lire fichier /tmp/tasks.txt
    }

    FILE * fPidTaskD = fopen("/tmp/taskd.pid", "r");
    if (fPidTaskD==NULL){
        fprintf(stderr,"taskd have to be launch to use ./taskcli");
        return 1;
    }
    pid_t pidTaskD;
    fscanf(fPidTaskD, "%d", &pidTaskD);
    fclose(fPidTaskD);

    char *end = NULL;

    int start = strtol(argv[1],&end,10);
    if (strcmp(end,"\0")!=0){
        fprintf(stderr,"Invalid start : %s",argv[1]);
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    int period = strtol(argv[2],&end,10);
    if (strcmp(end,"\0")!=0){
        fprintf(stderr,"Invalid period : %s",argv[2]);
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    int fd = open("/tmp/tasks.fifo",O_APPEND);
    if (fd == -1) {
        perror("Error opening tasks.fifo");
        return 1;
    }

    write(fd,&start,sizeof(start));
    write(fd,&period,sizeof(period));

    for(int i=3; i<argc; i++){
        send_argv(fd,&argv[i]);
    }
    close(fd);
    kill(pidTaskD, SIGUSR1);

    return 0;
}
