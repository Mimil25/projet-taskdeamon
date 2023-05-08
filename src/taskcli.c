#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

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
    if (*end != "\0"){
        fprintf(stderr,"Invalid start : %s",argv[1]);
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    int period = strtol(argv[2],&end,10);
    if (*end != "\0"){
        fprintf(stderr,"Invalid period : %s",argv[2]);
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    int fd = open("/tmp/tasks.fifo",O_RDWR);
    if (fd == -1) {
        perror("Error opening tasks.fifo");
        return 1;
    }
    for(size_t i=1; i<argc; i++){
        send_argv(fd,argv[i]);
    }
    closed(fd);
    kill(pidTaskD, SIGUSR1);

    return 0;
}
