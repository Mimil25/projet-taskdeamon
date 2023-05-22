#ifndef TASKCMDL
#define TASKCMDL

#include <bits/types/timer_t.h>
#include <stdio.h>
#define TASKCMDL_BASEALLOC 16
#define TASKCMDL_GROWRATE 2

#include <bits/types/time_t.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>

/*
 * represent a programed task
 */
struct taskcmd {
    size_t id;
    time_t start;
    time_t period;
    char** argv;
};

/*
 * destroy taskcmd and free it's argv
 */
void taskcmd_destroy(struct taskcmd* self);

/*
 * return true if the task should be executed 'now'
 */
bool taskcmd_now(const struct taskcmd* self, time_t now);

/*
 * return the time t of the next future execution of the task
 * t == -1 mean the task wont exeute anymore
 * t > now mean the task should excute at time t
 * 
 * t will never be inferior or equal to now
 */
time_t taskcmd_next(const struct taskcmd* self, time_t now);

/*
 * write a description of the task in file
 */
void taskcmd_frepr(const struct taskcmd* self, FILE* file);

/*
 * launch the task in a new thread
 * return the pid of the thread
 */
pid_t taskcmd_launch(const struct taskcmd* self);

/*
 * a list of taskcmd
 */
struct taskcmdl {
    size_t next_id;
    size_t size;
    size_t alloc;
    struct taskcmd* tasks;
};

/*
 * initialize a taskcmdl and alloc enouth memory for TASKCMDL_BASEALLOC tasks
 * 
 * return 0 on a succes, 1 on a failure
 */
int taskcmdl_init(struct taskcmdl* self);

/*
 * safely free all memory allocated in a taskcmdl
 * including tasks's argv
 */
void taskcmdl_destroy(struct taskcmdl* self);

/*
 * add a task to the list
 * return the atribuated id of the task on a succes, -1 on a failure
 */
int taskcmdl_add(struct taskcmdl* self, const struct taskcmd* task);

/*
 * return the time t of the next future execution execution
 * t == -1 mean no tasks are schedulled
 * t > now mean one or more tasks are schedulled for time t
 * 
 * t will never be inferior or equal to now
 * 
 * all tasks that wont execute anymore are removed from the list
 */
time_t taskcmdl_next(struct taskcmdl* self, time_t now);

/*
 * launch the tasks that should be launched now
 * return the number of launched tasks
 */
int taskcmdl_launch_now(const struct taskcmdl* self, time_t now);

#endif // TASKCMDL
