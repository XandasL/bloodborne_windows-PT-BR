/* SPDX-License-Identifier: MIT
 * PS4 HLE pthread rwlock services.
 * Single responsibility: scePthreadRwlock* NID bindings and lifecycle. (~70 LOC)
 */
#include "r_sync.h"
#include <string.h>

typedef struct { BbRwlock* native; } GuestRwlock;

static BB_SYSV_ABI int32_t rw_init(GuestRwlock** out, const void* attr, const char* name) {
    BB_UNUSED(attr); BB_UNUSED(name);
    if (!out) return BB_ORBIS_ERROR(22);
    GuestRwlock* rw = (GuestRwlock*)bb_platform_vm_reserve(0, sizeof(GuestRwlock));
    if (!rw) return BB_ORBIS_ERROR(12);
    bb_platform_vm_commit(rw, sizeof(GuestRwlock), BB_PROT_READ | BB_PROT_WRITE);
    rw->native = bb_platform_rwlock_create();
    *out = rw;
    return 0;
}

static BB_SYSV_ABI int32_t rw_rdlock(GuestRwlock** rw) {
    return (rw && *rw) ? bb_platform_rwlock_rdlock((*rw)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t rw_wrlock(GuestRwlock** rw) {
    return (rw && *rw) ? bb_platform_rwlock_wrlock((*rw)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t rw_tryrdlock(GuestRwlock** rw) {
    return (rw && *rw) ? bb_platform_rwlock_tryrdlock((*rw)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t rw_trywrlock(GuestRwlock** rw) {
    return (rw && *rw) ? bb_platform_rwlock_trywrlock((*rw)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t rw_unlock(GuestRwlock** rw) {
    return (rw && *rw) ? bb_platform_rwlock_unlock((*rw)->native) : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t rw_destroy(GuestRwlock** rw) {
    if (!rw || !*rw) return BB_ORBIS_ERROR(22);
    bb_platform_rwlock_destroy((*rw)->native);
    bb_platform_vm_release(*rw, sizeof(GuestRwlock));
    *rw = NULL;
    return 0;
}

uintptr_t r_sync_rwlock_resolve(const char* name) {
    if (!strcmp(name, "6ULAa0fq4jA#p#J")) return (uintptr_t)rw_init;
    if (!strcmp(name, "BB+kb08Tl9A#p#J")) return (uintptr_t)rw_destroy;
    if (!strcmp(name, "Ox9i0c7L5w0#p#J")) return (uintptr_t)rw_rdlock;
    if (!strcmp(name, "mqdNorrB+gI#p#J")) return (uintptr_t)rw_wrlock;
    if (!strcmp(name, "XD3mDeybCnk#p#J")) return (uintptr_t)rw_tryrdlock;
    if (!strcmp(name, "bIHoZCTomsI#p#J")) return (uintptr_t)rw_trywrlock;
    if (!strcmp(name, "+L98PIbGttk#p#J")) return (uintptr_t)rw_unlock;
    return 0;
}
