/* SPDX-License-Identifier: MIT
 * Windows RWLock implementation using Slim Reader/Writer (SRW) locks.
 * Single responsibility: RWLock creation, read/write locking. (~60 LOC)
 */
#ifdef _WIN32
#include "platform/sync.h"
#include <windows.h>
#include <stdlib.h>

struct BbRwlock {
    SRWLOCK lock;
    DWORD writer_tid;
};

BbRwlock* bb_platform_rwlock_create(void) {
    BbRwlock* rw = (BbRwlock*)calloc(1, sizeof(BbRwlock));
    if (rw) InitializeSRWLock(&rw->lock);
    return rw;
}

int32_t bb_platform_rwlock_rdlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    AcquireSRWLockShared(&rw->lock);
    return 0;
}

int32_t bb_platform_rwlock_wrlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    AcquireSRWLockExclusive(&rw->lock);
    rw->writer_tid = GetCurrentThreadId();
    return 0;
}

int32_t bb_platform_rwlock_tryrdlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    return TryAcquireSRWLockShared(&rw->lock) ? 0 : BB_ORBIS_ERROR(16);
}

int32_t bb_platform_rwlock_trywrlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    if (!TryAcquireSRWLockExclusive(&rw->lock)) return BB_ORBIS_ERROR(16);
    rw->writer_tid = GetCurrentThreadId();
    return 0;
}

int32_t bb_platform_rwlock_unlock(BbRwlock* rw) {
    if (!rw) return BB_ORBIS_ERROR(22);
    if (rw->writer_tid == GetCurrentThreadId()) {
        rw->writer_tid = 0;
        ReleaseSRWLockExclusive(&rw->lock);
    } else {
        ReleaseSRWLockShared(&rw->lock);
    }
    return 0;
}

void bb_platform_rwlock_destroy(BbRwlock* rw) {
    free(rw);
}
#endif
