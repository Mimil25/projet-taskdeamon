#include <bits/types/sigset_t.h>
#define _XOPEN_SOURCE 700

#include "message.h"
#include "taskcmdl.h"

#include <signal.h>
#include <asm-generic/errno-base.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bits/types/siginfo_t.h>
#include <stdbool.h>
#include <sys/wait.h>

const char* LOCK_FILE = "/tmp/taskd.pid";
const char* PIPE_FILE = "/tmp/tasks.fifo";
const char* TEXT_FILE = "/tmp/tasks.txt";
const char* TASK_DIR = "/tmp/tasks/";

/*
 * open TEXT_FILE with the proper lock
 * return a valid FILE* on a succes, NULL on failure
 */
FILE* open_text_file(char* mode) {
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    FILE* file = fopen(TEXT_FILE, mode);
    int fd = fileno(file);
    fcntl(fd, F_SETLKW, &lock);
    return file;
}

/*
 * close TEXT_FILE and remove the lock
 */
void close_text_file(FILE* file) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();
    int fd = fileno(file);
    fcntl(fd, F_SETLK, &lock);
    fclose(file);
}

/*
 * read a task from the pipe,
 * append it in the taskcmdl,
 * update tasks.txt
 */
void add_task(struct taskcmdl* tasks, int* nb_running) {
    struct taskcmd task;
    int pipe = open(PIPE_FILE, O_RDONLY);
    read(pipe, &task.start, sizeof(time_t));
    read(pipe, &task.period, sizeof(time_t));
    task.argv = recv_argv(pipe);
    close(pipe);
    task.id = taskcmdl_add(tasks, &task);
    if(task.id < 0) {
        fprintf(stderr, "Error : failed to add task, %s\n", strerror(errno));
        return;
    }

    FILE* file = open_text_file("a");
    taskcmd_frepr(&task, file);
    close_text_file(file);
    
    time_t now = time(NULL);
    if(taskcmd_now(&task, now)) {
        taskcmd_launch(&task);
        *nb_running += 1;
    }
    time_t next = taskcmdl_next(tasks, now);
    if(next == (time_t)-1) {
        alarm(0);
    } else {
        alarm(next - now);
    }
}

/*
 * launch the tasks that should be launched now
 */
void launch_tasks(struct taskcmdl* tasks, int* nb_running) {
    time_t now = time(NULL);
    
    *nb_running += taskcmdl_launch_now(tasks, now);

    size_t size = tasks->size;
    time_t next = taskcmdl_next(tasks, now);
    if(next != (time_t)-1) {
        alarm(next - now);
    }
    if(size != tasks->size) { // some tasks have been removed
        FILE* file = open_text_file("w");
        for(size_t i=0; i < tasks->size; ++i) {
            taskcmd_frepr(&tasks->tasks[i], file);
        }
        close_text_file(file);
    }
}

/*
 * handle the end of a child
 */
void child_end(int* nb_running) {
    int wstatus;
    pid_t pid = wait(&wstatus);
    if(WIFEXITED(wstatus)) {
        int exit_status = WEXITSTATUS(wstatus);
        fprintf(stderr, "process %d exited with status %d\n", pid, exit_status);
    } else if(WIFSIGNALED(wstatus)) { // the if statment is unnessecary
        int sig = WTERMSIG(wstatus);
        fprintf(stderr, "process %d killed by signal %d\n", pid, sig);
    }
    *nb_running -= 1;
}

/*
 * end the program properly
 */
void quit(struct taskcmdl* tasks, int* nb_running) {
    kill(0, SIGQUIT);
    while (*nb_running) {
        child_end(nb_running);
    }
    taskcmdl_destroy(tasks);
    unlink(LOCK_FILE);
    exit(EXIT_SUCCESS);
}

bool f_add_task = false;
bool f_launch_tasks = false;
bool f_child_end = false;
bool f_quit = false;
void handler(int sig) {
    switch(sig) {
        case SIGUSR1:
            f_add_task = true;
            break;
        case SIGALRM:
            f_launch_tasks = true;
            break;
        case SIGCHLD:
            f_child_end = true;
            break;
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            f_quit = true;
            break;
    }
}

/*
 * check that LOCK_FILE doesn't exist and create it
 * if LOCK_FILE exist, exit
 */
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

int main(void) {
    check_lock();
    
    freopen("/tmp/taskd.out", "a", stdout);
    freopen("/tmp/taskd.err", "a", stderr);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

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

    int nb_running = 0;
    struct taskcmdl tasks;
    taskcmdl_init(&tasks);
    
    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGCHLD);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGTERM);
    sigprocmask(SIG_BLOCK, &set, NULL);
    
    sigemptyset(&set);
    while (1) {
        sigsuspend(&set);
        if(f_add_task) {
            f_add_task = false;
            add_task(&tasks, &nb_running);
        }
        if(f_launch_tasks) {
            f_launch_tasks = false;
            launch_tasks(&tasks, &nb_running);
        }
        if(f_child_end) {
            f_child_end = false;
            child_end(&nb_running);
        }
        if(f_quit) {
            f_quit = false;
            quit(&tasks, &nb_running);
        }
    }

    return 0;
}
