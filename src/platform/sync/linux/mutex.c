/* SPDX-License-Identifier: MIT
 * Linux POSIX mutex implementation.
 * Single responsibility: pthread_mutex lifecycle, recursive mapping. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/sync.h"
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

struct BbMutex {
    pthread_mutex_t handle;
};

BbMutex* bb_platform_mutex_create(enum BbMutexType type) {
    BbMutex* m = (BbMutex*)malloc(sizeof(BbMutex));
    if (!m) return NULL;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int native_type = (type == BB_MUTEX_TYPE_RECURSIVE) ? PTHREAD_MUTEX_RECURSIVE :
                      (type == BB_MUTEX_TYPE_ERRORCHECK) ? PTHREAD_MUTEX_ERRORCHECK :
                      PTHREAD_MUTEX_NORMAL;
    pthread_mutexattr_settype(&attr, native_type);
    if (pthread_mutex_init(&m->handle, &attr) != 0) {
        pthread_mutexattr_destroy(&attr);
        free(m);
        return NULL;
    }
    pthread_mutexattr_destroy(&attr);
    return m;
}

int32_t bb_platform_mutex_lock(BbMutex* m) {
    if (!m) return BB_ORBIS_ERROR(22);
    int e = pthread_mutex_lock(&m->handle);
    return e ? BB_ORBIS_ERROR(e == EDEADLK ? 11 : e == EBUSY ? 16 : 22) : 0;
}

int32_t bb_platform_mutex_trylock(BbMutex* m) {
    if (!m) return BB_ORBIS_ERROR(22);
    int e = pthread_mutex_trylock(&m->handle);
    return e ? BB_ORBIS_ERROR(16) : 0;
}

int32_t bb_platform_mutex_unlock(BbMutex* m) {
    if (!m) return BB_ORBIS_ERROR(22);
    int e = pthread_mutex_unlock(&m->handle);
    return e ? BB_ORBIS_ERROR(e == EPERM ? 1 : 22) : 0;
}

void bb_platform_mutex_destroy(BbMutex* m) {
    if (m) {
        pthread_mutex_destroy(&m->handle);
        free(m);
    }
}
#endif
