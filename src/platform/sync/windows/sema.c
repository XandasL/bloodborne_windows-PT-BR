/* SPDX-License-Identifier: MIT
 * Windows semaphore implementation using Win32 Semaphore kernel objects.
 * Single responsibility: Counted tokens, timed waits, signaling. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/sync.h"
#include <windows.h>
#include <stdlib.h>

struct BbSema {
    HANDLE handle;
    int32_t max;
};

BbSema* bb_platform_sema_create(int32_t initial, int32_t max) {
    if (initial < 0 || max <= 0 || initial > max) return NULL;
    BbSema* s = (BbSema*)calloc(1, sizeof(BbSema));
    if (!s) return NULL;
    s->handle = CreateSemaphoreW(NULL, initial, max, NULL);
    s->max = max;
    if (!s->handle) { free(s); return NULL; }
    return s;
}

int32_t bb_platform_sema_wait(BbSema* s, int32_t count, uint32_t* timeout_us) {
    if (!s || count <= 0 || count > s->max) return BB_ORBIS_ERROR(22);
    DWORD timeout_ms = timeout_us ? (DWORD)(*timeout_us / 1000) : INFINITE;
    for (int32_t i = 0; i < count; ++i) {
        DWORD res = WaitForSingleObject(s->handle, timeout_ms);
        if (res == WAIT_TIMEOUT) {
            if (i > 0) ReleaseSemaphore(s->handle, i, NULL);
            return BB_ORBIS_ERROR(60); /* ETIMEDOUT */
        }
        if (res != WAIT_OBJECT_0) {
            if (i > 0) ReleaseSemaphore(s->handle, i, NULL);
            return BB_ORBIS_ERROR(3);
        }
    }
    return 0;
}

int32_t bb_platform_sema_signal(BbSema* s, int32_t count) {
    if (!s || count <= 0) return BB_ORBIS_ERROR(22);
    if (!ReleaseSemaphore(s->handle, count, NULL)) return BB_ORBIS_ERROR(22);
    return 0;
}

int32_t bb_platform_sema_cancel(BbSema* s, int32_t count, int32_t* waiters) {
    if (!s) return BB_ORBIS_ERROR(22);
    if (waiters) *waiters = 0;
    if (count > 0) ReleaseSemaphore(s->handle, count, NULL);
    return 0;
}

void bb_platform_sema_destroy(BbSema* s) {
    if (s) {
        if (s->handle) CloseHandle(s->handle);
        free(s);
    }
}
#endif
