/* SPDX-License-Identifier: MIT
 * Windows mutex implementation using Win32 CriticalSection.
 * Single responsibility: Mutex creation, lock/unlock, recursion. (~70 LOC)
 */
#ifdef _WIN32
#include "platform/sync.h"
#include <windows.h>
#include <stdlib.h>

struct BbMutex {
    CRITICAL_SECTION cs;
    enum BbMutexType type;
    DWORD owner_tid;
    uint32_t recursion;
};

BbMutex* bb_platform_mutex_create(enum BbMutexType type) {
    BbMutex* m = (BbMutex*)calloc(1, sizeof(BbMutex));
    if (!m) return NULL;
    InitializeCriticalSectionAndSpinCount(&m->cs, 4000);
    m->type = type;
    return m;
}

int32_t bb_platform_mutex_lock(BbMutex* m) {
    if (!m) return BB_ORBIS_ERROR(22); /* EINVAL */
    DWORD self = GetCurrentThreadId();
    if (m->owner_tid == self && m->type != BB_MUTEX_TYPE_RECURSIVE) {
        return BB_ORBIS_ERROR(11); /* EDEADLK */
    }
    EnterCriticalSection(&m->cs);
    m->owner_tid = self;
    m->recursion++;
    return 0;
}

int32_t bb_platform_mutex_trylock(BbMutex* m) {
    if (!m) return BB_ORBIS_ERROR(22);
    DWORD self = GetCurrentThreadId();
    if (m->owner_tid == self && m->type != BB_MUTEX_TYPE_RECURSIVE) {
        return BB_ORBIS_ERROR(16); /* EBUSY */
    }
    if (!TryEnterCriticalSection(&m->cs)) return BB_ORBIS_ERROR(16);
    m->owner_tid = self;
    m->recursion++;
    return 0;
}

int32_t bb_platform_mutex_unlock(BbMutex* m) {
    if (!m || m->owner_tid != GetCurrentThreadId()) {
        return BB_ORBIS_ERROR(1); /* EPERM */
    }
    if (--m->recursion == 0) m->owner_tid = 0;
    LeaveCriticalSection(&m->cs);
    return 0;
}

void bb_platform_mutex_destroy(BbMutex* m) {
    if (m) {
        DeleteCriticalSection(&m->cs);
        free(m);
    }
}
#endif
