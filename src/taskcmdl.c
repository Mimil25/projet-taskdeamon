#include "taskcmdl.h"

#include <bits/time.h>
#include <bits/types/time_t.h>
#include <stddef.h>
#include <time.h>
#include <stdlib.h>

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

time_t taskcmd_next_exec(const struct taskcmd* self, time_t now) {
    if(now < self->start) {
        return self->start;
    } else if (self->period == 0) {
        return (time_t)-1;
    } else {
        return  now + self->period - (now - self->start)%self->period;
    }
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
    self->size = 0;
    self->alloc = TASKCMDL_BASEALLOC;
    self->tasks = calloc(self->alloc, sizeof(struct taskcmd));
    if(!self->tasks) {
        self->alloc = 0;
        return -1;
    }
    return 0;
}

void taskcmdl_destroy(struct taskcmdl* self) {
    for(int i=0; i < self->size; ++i) {
        taskcmd_destroy(&self->tasks[i]);
    }
    free(self->tasks);
    self->tasks = NULL;
    self->size = 0;
    self->alloc = 0;
}

size_t taskcmdl_size(const struct taskcmdl* self) {
    return self->size;
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
    ++self->size;
    return 0;
}

struct taskcmd* taskcmdl_next(struct taskcmdl* self) {
    time_t now = time(CLOCK_REALTIME);
begin:
    if(self->size == 0) {
        return NULL;
    }
    struct taskcmd* min_task = &self->tasks[0];
    time_t min_time = taskcmd_next_exec(min_task, now);
    if(min_time == (time_t)-1) { // remove the tasks that wont execute anymore
        taskcmdl_remove(self, 0);
        goto begin;
    }
    for(int i=1; i < self->size; ++i) {
        time_t t = taskcmd_next_exec(&self->tasks[i], now);
        if(t == (time_t)-1) { // remove the tasks that wont execute anymore
            taskcmdl_remove(self, i);
            --i;
            continue;
        }
        if(t < min_time) {
            min_time = t;
            min_task = &self->tasks[i];
        }
    }
    return min_task;
}
