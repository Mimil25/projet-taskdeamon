#include "taskcmdl.h"

#include <bits/time.h>
#include <bits/types/time_t.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static void free_argv(char** argv) {
    char** p = argv;
    while (*p) {
        free(*p);
        ++p;
    }
    free(argv);
}

void taskcmd_destroy(struct taskcmd* self) {
    free_argv(self->argv);
    self->argv = NULL;
}

bool taskcmd_now(const struct taskcmd *self, time_t now) {
    return now == self->start ||
        ( now > self->start &&
          self->period != 0 &&
          (self->start - now)%self->period == 0);
}

time_t taskcmd_next(const struct taskcmd* self, time_t now) {
    if(now < self->start) {
        return self->start;
    } else if (self->period == 0) {
        return (time_t)-1;
    } else {
        return  now + self->period - (now - self->start)%self->period;
    }
}

void taskcmd_frepr(const struct taskcmd* self, FILE* file) {
    fprintf(file, "%ld;%ld;%ld;", self->id, self->start, self->period);
    for(char** arg=self->argv; *arg; ++arg) {
        fprintf(file, "%s ", *arg);
    }
    fprintf(file, "\n");
}

pid_t taskcmd_launch(const struct taskcmd *self) {
    pid_t pid = fork();
    if(pid == 0) {
        execvp(self->argv[0], &self->argv[0]);
        exit(1);
    }
    return pid;
}

// TASKCMDL

static void taskcmdl_remove(struct taskcmdl* self, size_t i) {
    if(i >= self->size) return;
    taskcmd_destroy(&self->tasks[i]);
    --self->size;
    if(i != self->size) {
        self->tasks[i] = self->tasks[self->size];
    }
}

int taskcmdl_init(struct taskcmdl* self) {
    self->next_id = 1;
    self->size = 0;
    self->alloc = TASKCMDL_BASEALLOC;
    self->tasks = calloc(self->alloc, sizeof(struct taskcmd));
    if(!self->tasks) {
        self->alloc = 0;
        return 1;
    }
    return 0;
}

void taskcmdl_destroy(struct taskcmdl* self) {
    for(size_t i=0; i < self->size; ++i) {
        taskcmd_destroy(&self->tasks[i]);
    }
    free(self->tasks);
    self->tasks = NULL;
    self->size = 0;
    self->alloc = 0;
}

int taskcmdl_add(struct taskcmdl* self, const struct taskcmd* task) {
    if(self->size >= self->alloc) {
        struct taskcmd* new = realloc(self->tasks, self->alloc*TASKCMDL_GROWRATE * sizeof(struct taskcmd));
        if(!new) {
            return -1;
        }
        self->alloc *= TASKCMDL_GROWRATE;
        self->tasks = new;
    }
    self->tasks[self->size] = *task;
    self->tasks[self->size].id = self->next_id;
    ++self->size;
    return self->next_id++;
}

time_t taskcmdl_next(struct taskcmdl *self, time_t now) {
    if(self->size == 0) {
        return (time_t)-1;
    }

    time_t min = taskcmd_next(&self->tasks[0], now);
    if(min == (time_t)-1) { // remove the tasks that wont execute anymore
        taskcmdl_remove(self, 0);
        return taskcmdl_next(self, now);
    }
    for(size_t i=1; i < self->size; ++i) {
        time_t t = taskcmd_next(&self->tasks[i], now);
        if(t == (time_t)-1) { // remove the tasks that wont execute anymore
            taskcmdl_remove(self, i);
            --i;
            continue;
        }
        if(t < min) {
            min = t;
        }
    }
    return min;
}

int taskcmdl_launch_now(const struct taskcmdl *self, time_t now) {
    size_t n = 0;
    struct taskcmd* p = self->tasks;
    struct taskcmd* end = &self->tasks[self->size];
    
    for(; p != end; ++p) {
        if(taskcmd_now(p, now)) {
            if(taskcmd_launch(p)) n++;
        }
    }

    return n;
}
