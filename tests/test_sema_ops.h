#pragma once
#define _GNU_SOURCE
#include "runtime.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

void runtime_restart(void) { abort(); }

typedef int32_t (ABI *Create)(uint32_t *, const char *, uint32_t, int32_t, int32_t, const void *);
typedef int32_t (ABI *CountOp)(uint32_t, int32_t);
typedef int32_t (ABI *Wait)(uint32_t, int32_t, uint32_t *);
typedef int32_t (ABI *Delete)(uint32_t);
typedef int32_t (ABI *Cancel)(uint32_t, int32_t, int32_t *);
#define GET(t, n) ((t)runtime_resolve(n, 0))

static Create create;
static CountOp poll_sem, signal_sem;
static Wait wait_sem;
static Delete delete_sem;
static Cancel cancel_sem;

static void setup(void) {
    runtime_start(1);
    create = GET(Create, "188x57JYp0g#p#J");
    poll_sem = GET(CountOp, "12wOHk8ywb0#p#J");
    signal_sem = GET(CountOp, "4czppHBiriw#p#J");
    wait_sem = GET(Wait, "Zxa0VhQVTsk#p#J");
    delete_sem = GET(Delete, "R1Jvn8bSCW8#p#J");
    cancel_sem = GET(Cancel, "4DM06U2BNEY#p#J");
    assert(create && poll_sem && signal_sem && wait_sem && delete_sem && cancel_sem);
    assert(!runtime_resolve("188x57JYp0g#I#J", 0));
    assert(!runtime_resolve("188x57JYp0g#p#J", 1));
}

typedef struct { uint32_t id; int32_t need, result; uint32_t timeout; } Task;
static void *waiter(void *p) { Task *t = p; t->result = wait_sem(t->id, t->need, &t->timeout); return NULL; }
static void enrolled(uint32_t id, unsigned count) {
    const struct timespec nap = {0, 1000000};
    for (int i = 0; i < 2000; ++i) {
        if (runtime_sema_waiters(id) == count) return;
        nanosleep(&nap, NULL);
    }
    assert(!"waiter enrollment timed out");
}
