/* SPDX-License-Identifier: MIT
 * PS4 HLE pthread mutex services.
 * Single responsibility: scePthreadMutex* NID bindings and lifecycle. (~75 LOC)
 */
#include "r_sync.h"
#include <string.h>

typedef struct { BbMutex* native; int type; } GuestMutex;

static BB_SYSV_ABI int32_t mutex_init(GuestMutex** out, void** attr, const char* name) {
    BB_UNUSED(name);
    if (!out) return BB_ORBIS_ERROR(22);
    int type = (attr && *attr) ? *(int*)(*attr) : BB_MUTEX_TYPE_NORMAL;
    GuestMutex* m = (GuestMutex*)bb_platform_vm_reserve(0, sizeof(GuestMutex));
    if (!m) return BB_ORBIS_ERROR(12);
    bb_platform_vm_commit(m, sizeof(GuestMutex), BB_PROT_READ | BB_PROT_WRITE);
    m->native = bb_platform_mutex_create((enum BbMutexType)type);
    m->type = type;
    *out = m;
    return 0;
}

static BB_SYSV_ABI int32_t mutex_lock(GuestMutex** m) {
    return (m && *m) ? bb_platform_mutex_lock((*m)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t mutex_trylock(GuestMutex** m) {
    return (m && *m) ? bb_platform_mutex_trylock((*m)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t mutex_timedlock(GuestMutex** m, uint32_t usec) {
    return (m && *m) ? bb_platform_mutex_timedlock((*m)->native, usec) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t mutex_unlock(GuestMutex** m) {
    return (m && *m) ? bb_platform_mutex_unlock((*m)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t mutex_destroy(GuestMutex** m) {
    if (!m || !*m) return BB_ORBIS_ERROR(22);
    bb_platform_mutex_destroy((*m)->native);
    bb_platform_vm_release(*m, sizeof(GuestMutex));
    *m = NULL;
    return 0;
}

uintptr_t r_sync_mutex_resolve(const char* name) {
    if (!strcmp(name, "cmo1RIYva9o#p#J") || !strcmp(name, "ttHNfU+qDBU#I#J")) return (uintptr_t)mutex_init;
    if (!strcmp(name, "9UK1vLZQft4#p#J") || !strcmp(name, "7H0iTOciTLo#I#J")) return (uintptr_t)mutex_lock;
    if (!strcmp(name, "upoVrzMHFeE#p#J") || !strcmp(name, "K-jXhbt2gn4#I#J")) return (uintptr_t)mutex_trylock;
    if (!strcmp(name, "tn3VlD0hG60#p#J") || !strcmp(name, "2Z+PpY6CaJg#I#J")) return (uintptr_t)mutex_unlock;
    if (!strcmp(name, "2Of0f+3mhhE#p#J") || !strcmp(name, "ltCfaGr2JGE#I#J")) return (uintptr_t)mutex_destroy;
    if (!strcmp(name, "IafI2PxcPnQ#p#J")) return (uintptr_t)mutex_timedlock;
    return 0;
}
