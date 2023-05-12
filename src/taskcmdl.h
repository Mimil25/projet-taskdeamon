#ifndef TASKCMDL
#define TASKCMDL

#define TASKCMDL_BASEALLOC 16
#define TASKCMDL_GROWRATE 2
#define TASKCMDL_FREETHRESHOLD 3 // the alloc space must be TASKCMDL_FREETHRESHOLD times biger than size to free some space

#include <bits/types/time_t.h>
#include <stddef.h>
#include <time.h>

struct taskcmd {
    time_t start;
    time_t period;
    char** argv;
};

void taskcmd_destroy(struct taskcmd* self);
time_t taskcmd_next_exec(const struct taskcmd* self, time_t now);

struct taskcmdl {
    size_t size;
    size_t alloc;
    struct taskcmd* tasks;
};

int taskcmdl_init(struct taskcmdl* self);
void taskcmdl_destroy(struct taskcmdl* self);
size_t taskcmdl_size(const struct taskcmdl* self);

int taskcmdl_add(struct taskcmdl* self, const struct taskcmd* task);
struct taskcmd* taskcmdl_next(struct taskcmdl* self);

#endif // TASKCMDL
