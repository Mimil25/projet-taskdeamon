#include <time.h>
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
        static struct flock lock;
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        
        int fd = open("/tmp/tasks.txt",O_RDONLY);
        if (fd == -1) {
            perror("Error opening tasks.txt");
            return 1;
        }
        fcntl ( fd , F_SETLKW , &lock );

        char buf[256];

        int taille = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);
        
        if(taille==0){
            fprintf(stdout,"No command to run.\n");
            lock.l_type = F_UNLCK;
            fcntl ( fd , F_SETLKW , &lock );
            close(fd);
            return 0;
        }

        size_t sz=-1;

        fprintf(stdout,"Commands to run :\n");
        while(sz!=0){
            sz=read(fd,&buf,256*sizeof(char));
            write(1,&buf,sz*sizeof(char));
        }

        lock.l_type = F_UNLCK;
        fcntl ( fd , F_SETLKW , &lock );
        close(fd);
        return 0;
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

    time_t start = strtol(argv[1],&end,10);
    if (strcmp(end,"\0")!=0){
        fprintf(stderr,"Invalid start : %s",argv[1]);
        fprintf(stderr,"Usage : ./taskcli START PERIOD CMD [ARG]...");
        fprintf(stderr,"Usage : ./taskcli");
        return 1;
    }

    time_t period = strtol(argv[2],&end,10);
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

    send_argv(fd,&argv[3]);
    
    close(fd);
    kill(pidTaskD, SIGUSR1);

    return 0;
}
