/* SPDX-License-Identifier: MIT
 * PS4 HLE kernel semaphore services.
 * Single responsibility: sceKernelSem* NID bindings and table dispatch. (~75 LOC)
 */
#include "r_sync.h"
#include <string.h>

#define MAX_SEMAS 256
static BbSema* g_semas[MAX_SEMAS];

static BB_SYSV_ABI int32_t sem_create(uint32_t* out, const char* name, uint32_t attr,
                                      int32_t init, int32_t max, const void* opt) {
    BB_UNUSED(name); BB_UNUSED(attr); BB_UNUSED(opt);
    if (!out) return BB_ORBIS_ERROR(22);
    for (uint32_t i = 1; i < MAX_SEMAS; ++i) {
        if (!g_semas[i]) {
            g_semas[i] = bb_platform_sema_create(init, max);
            if (!g_semas[i]) return BB_ORBIS_ERROR(12);
            *out = i;
            return 0;
        }
    }
    return BB_ORBIS_ERROR(28); /* ENOSPC */
}

static BB_SYSV_ABI int32_t sem_wait(uint32_t id, int32_t count, uint32_t* timeout) {
    if (!id || id >= MAX_SEMAS || !g_semas[id]) return BB_ORBIS_ERROR(3);
    return bb_platform_sema_wait(g_semas[id], count, timeout);
}

static BB_SYSV_ABI int32_t sem_poll(uint32_t id, int32_t count) {
    uint32_t zero = 0;
    return sem_wait(id, count, &zero);
}

static BB_SYSV_ABI int32_t sem_signal(uint32_t id, int32_t count) {
    if (!id || id >= MAX_SEMAS || !g_semas[id]) return BB_ORBIS_ERROR(3);
    return bb_platform_sema_signal(g_semas[id], count);
}

static BB_SYSV_ABI int32_t sem_cancel(uint32_t id, int32_t count, int32_t* waiters) {
    if (!id || id >= MAX_SEMAS || !g_semas[id]) return BB_ORBIS_ERROR(3);
    return bb_platform_sema_cancel(g_semas[id], count, waiters);
}

static BB_SYSV_ABI int32_t sem_delete(uint32_t id) {
    if (!id || id >= MAX_SEMAS || !g_semas[id]) return BB_ORBIS_ERROR(3);
    bb_platform_sema_destroy(g_semas[id]);
    g_semas[id] = NULL;
    return 0;
}

uintptr_t r_sync_sema_resolve(const char* name) {
    if (!strcmp(name, "188x57JYp0g#p#J")) return (uintptr_t)sem_create;
    if (!strcmp(name, "Zxa0VhQVTsk#p#J")) return (uintptr_t)sem_wait;
    if (!strcmp(name, "4czppHBiriw#p#J")) return (uintptr_t)sem_signal;
    if (!strcmp(name, "12wOHk8ywb0#p#J")) return (uintptr_t)sem_poll;
    if (!strcmp(name, "4DM06U2BNEY#p#J")) return (uintptr_t)sem_cancel;
    if (!strcmp(name, "R1Jvn8bSCW8#p#J")) return (uintptr_t)sem_delete;
    return 0;
}
