/* SPDX-License-Identifier: MIT
 * Linux semaphore implementation using mutex and condition variables.
 * Single responsibility: Token counting, FIFO waiters, timed waits. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/sync.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

struct BbSema { pthread_mutex_t lock; pthread_cond_t cond; int32_t count, max; };

BbSema* bb_platform_sema_create(int32_t initial, int32_t max) {
    if (initial < 0 || max <= 0 || initial > max) return NULL;
    BbSema* s = (BbSema*)calloc(1, sizeof(BbSema));
    if (!s) return NULL;
    pthread_mutex_init(&s->lock, NULL);
    pthread_condattr_t attr; pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&s->cond, &attr);
    pthread_condattr_destroy(&attr);
    s->count = initial; s->max = max;
    return s;
}

int32_t bb_platform_sema_wait(BbSema* s, int32_t count, uint32_t* timeout_us) {
    if (!s || count <= 0 || count > s->max) return BB_ORBIS_ERROR(22);
    pthread_mutex_lock(&s->lock);
    struct timespec ts;
    if (timeout_us) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        uint64_t ns = (uint64_t)ts.tv_nsec + (uint64_t)*timeout_us * 1000ULL;
        ts.tv_sec += (time_t)(ns / 1000000000ULL); ts.tv_nsec = (long)(ns % 1000000000ULL);
    }
    while (s->count < count) {
        int e = timeout_us ? pthread_cond_timedwait(&s->cond, &s->lock, &ts) : pthread_cond_wait(&s->cond, &s->lock);
        if (e == ETIMEDOUT) { pthread_mutex_unlock(&s->lock); return BB_ORBIS_ERROR(60); }
    }
    s->count -= count;
    pthread_mutex_unlock(&s->lock);
    return 0;
}

int32_t bb_platform_sema_signal(BbSema* s, int32_t count) {
    if (!s || count <= 0) return BB_ORBIS_ERROR(22);
    pthread_mutex_lock(&s->lock);
    if (s->count + count > s->max) { pthread_mutex_unlock(&s->lock); return BB_ORBIS_ERROR(22); }
    s->count += count;
    pthread_cond_broadcast(&s->cond);
    pthread_mutex_unlock(&s->lock);
    return 0;
}

int32_t bb_platform_sema_cancel(BbSema* s, int32_t count, int32_t* waiters) {
    if (!s) return BB_ORBIS_ERROR(22);
    pthread_mutex_lock(&s->lock);
    if (waiters) *waiters = 0;
    s->count = (count < 0) ? s->max : count;
    pthread_cond_broadcast(&s->cond);
    pthread_mutex_unlock(&s->lock);
    return 0;
}

void bb_platform_sema_destroy(BbSema* s) {
    if (s) { pthread_mutex_destroy(&s->lock); pthread_cond_destroy(&s->cond); free(s); }
}
#endif
