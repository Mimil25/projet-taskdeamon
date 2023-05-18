#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
    pid_t pid = fork();
    if(pid == 0) {
        setsid();
        pid_t pid2 = fork();
        if(pid2 == 0) {
            chdir("/");
            umask(0);
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            
            execvp(argv[1], &argv[1]);
            exit(1);
        }
        exit(0);
    }
    waitpid(pid, NULL, 0);
    return 0;
}
