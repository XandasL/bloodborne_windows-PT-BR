/* SPDX-License-Identifier: MIT
 * PS4 Thread Subsystem Types and Internal Signatures.
 * Single responsibility: Core thread records and synchronization. (~65 LOC)
 */
#ifndef R_THREAD_TYPES_H
#define R_THREAD_TYPES_H

#include "runtime.h"
#include "platform/bb_common.h"
#include "platform/threads.h"
#include "platform/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#define ERR(n) ((int32_t)(UINT32_C(0x80020000) | (n)))
#define ATTR_MAGIC UINT32_C(0x41545452)
#define STACK_MARGIN (256 * 1024)
#define MIN_STACK (64 * 1024)
#define DEFAULT_STACK (1024 * 1024)
#define DEFAULT_PRIO 700

typedef void *(ABI *GuestEntry)(void *);

typedef struct ThreadAttr {
    uint32_t magic;
    int detached, policy, prio, inherit;
    uint64_t stack, guard, affinity;
    struct ThreadAttr *next;
} ThreadAttr;

typedef struct GuestThread {
    uint64_t *tcb;
    unsigned char *tls_block;
    BbThread *host;
    GuestEntry entry;
    void *argument, *result;
    ThreadAttr attr;
    char name[32];
    int detached, finished, joined, host_owned;
    jmp_buf exit_jump;
    struct GuestThread *next;
} GuestThread;

void r_thread_lock(void);
void r_thread_unlock(void);
GuestThread *r_thread_find(void *handle);
ThreadAttr *r_thread_find_attr(ThreadAttr **slot);
void r_thread_publish(GuestThread *t);
GuestThread *r_thread_new(void);
void r_thread_attach(GuestThread *t);
GuestThread *runtime_thread_current(void);

extern size_t g_threads_created, g_threads_joined, g_threads_exited;

#endif /* R_THREAD_TYPES_H */
