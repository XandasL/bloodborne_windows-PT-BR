/* SPDX-License-Identifier: MIT
 * Linux POSIX RWLock implementation.
 * Single responsibility: pthread_rwlock lifecycle, reader/writer locks. (~60 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/sync.h"
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

struct BbRwlock {
    pthread_rwlock_t handle;
};

BbRwlock* bb_platform_rwlock_create(void) {
    BbRwlock* rw = (BbRwlock*)malloc(sizeof(BbRwlock));
    if (!rw) return NULL;
    if (pthread_rwlock_init(&rw->handle, NULL) != 0) {
        free(rw);
        return NULL;
    }
    return rw;
}

int32_t bb_platform_rwlock_rdlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    int e = pthread_rwlock_rdlock(&rw->handle);
    return e ? BB_ORBIS_ERROR(22) : 0;
}

int32_t bb_platform_rwlock_wrlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    int e = pthread_rwlock_wrlock(&rw->handle);
    return e ? BB_ORBIS_ERROR(22) : 0;
}

int32_t bb_platform_rwlock_tryrdlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    int e = pthread_rwlock_tryrdlock(&rw->handle);
    return e ? BB_ORBIS_ERROR(16) : 0;
}

int32_t bb_platform_rwlock_trywrlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    int e = pthread_rwlock_trywrlock(&rw->handle);
    return e ? BB_ORBIS_ERROR(16) : 0;
}

int32_t bb_platform_rwlock_unlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    int e = pthread_rwlock_unlock(&rw->handle);
    return e ? BB_ORBIS_ERROR(22) : 0;
}

void bb_platform_rwlock_destroy(BbRwlock* rw) {
    if (rw) {
        pthread_rwlock_destroy(&rw->handle);
        free(rw);
    }
}
#endif
