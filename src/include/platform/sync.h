/* SPDX-License-Identifier: MIT
 * Platform synchronization primitives interface.
 * Single responsibility: Opaque mutex, rwlock, and semaphore contracts. (~55 LOC)
 */
#ifndef BB_PLATFORM_SYNC_H
#define BB_PLATFORM_SYNC_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BbMutex BbMutex;
typedef struct BbRwlock BbRwlock;
typedef struct BbSema BbSema;

enum BbMutexType {
    BB_MUTEX_TYPE_NORMAL = 1,
    BB_MUTEX_TYPE_RECURSIVE = 2,
    BB_MUTEX_TYPE_ERRORCHECK = 3
};

BbMutex* bb_platform_mutex_create(enum BbMutexType type);
int32_t  bb_platform_mutex_lock(BbMutex* mutex);
int32_t  bb_platform_mutex_trylock(BbMutex* mutex);
int32_t  bb_platform_mutex_timedlock(BbMutex* mutex, uint64_t timeout_us);
int32_t  bb_platform_mutex_unlock(BbMutex* mutex);
void     bb_platform_mutex_destroy(BbMutex* mutex);

BbRwlock* bb_platform_rwlock_create(void);
int32_t   bb_platform_rwlock_rdlock(BbRwlock* rwlock);
int32_t   bb_platform_rwlock_wrlock(BbRwlock* rwlock);
int32_t   bb_platform_rwlock_tryrdlock(BbRwlock* rwlock);
int32_t   bb_platform_rwlock_trywrlock(BbRwlock* rwlock);
int32_t   bb_platform_rwlock_unlock(BbRwlock* rwlock);
void      bb_platform_rwlock_destroy(BbRwlock* rwlock);

BbSema* bb_platform_sema_create(int32_t initial, int32_t max);
int32_t bb_platform_sema_wait(BbSema* sema, int32_t count, uint32_t* timeout_us);
int32_t bb_platform_sema_signal(BbSema* sema, int32_t count);
int32_t bb_platform_sema_cancel(BbSema* sema, int32_t count, int32_t* waiters);
void    bb_platform_sema_destroy(BbSema* sema);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_SYNC_H */
