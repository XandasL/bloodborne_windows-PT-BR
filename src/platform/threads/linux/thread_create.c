/* SPDX-License-Identifier: MIT
 * Linux POSIX thread creation and lifecycle implementation.
 * Single responsibility: pthread_create with stack limits and naming. (~70 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/threads.h"
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

struct BbThread {
    pthread_t handle;
    BbThreadEntry entry;
    void* arg;
    void* result;
};

static void* ThreadProxy(void* param) {
    BbThread* t = (BbThread*)param;
    t->result = t->entry(t->arg);
    return t->result;
}

BbThread* bb_platform_thread_create(BbThreadEntry entry, void* arg, size_t stack_size, const char* name) {
    BbThread* t = (BbThread*)calloc(1, sizeof(BbThread));
    if (!t) return NULL;
    t->entry = entry;
    t->arg = arg;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size) pthread_attr_setstacksize(&attr, stack_size);
    if (pthread_create(&t->handle, &attr, ThreadProxy, t) != 0) {
        pthread_attr_destroy(&attr);
        free(t);
        return NULL;
    }
    pthread_attr_destroy(&attr);
    if (name) {
        char thread_name[16] = {0};
        strncpy(thread_name, name, sizeof(thread_name) - 1);
        pthread_setname_np(t->handle, thread_name);
    }
    return t;
}

int32_t bb_platform_thread_join(BbThread* t, void** result) {
    if (!t) return BB_ORBIS_ERROR(3);
    int e = pthread_join(t->handle, result ? result : &t->result);
    free(t);
    return e ? BB_ORBIS_ERROR(22) : 0;
}

int32_t bb_platform_thread_detach(BbThread* t) {
    if (!t) return BB_ORBIS_ERROR(3);
    int e = pthread_detach(t->handle);
    return e ? BB_ORBIS_ERROR(22) : 0;
}

void bb_platform_thread_yield(void) {
    sched_yield();
}

uint64_t bb_platform_thread_id(void) {
    return (uint64_t)pthread_self();
}

void* bb_platform_alloc_low_stack(size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
#endif
