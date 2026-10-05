/* SPDX-License-Identifier: MIT
 * PS4 HLE pthread condition variable services.
 * Single responsibility: scePthreadCond* NID bindings and condition dispatch. (~70 LOC)
 */
#include "r_sync.h"
#include <string.h>

typedef struct { BbMutex* mutex; } GuestCond;

static BB_SYSV_ABI int32_t cond_init(GuestCond** out, void** attr, const char* name) {
    BB_UNUSED(attr); BB_UNUSED(name);
    if (!out) return BB_ORBIS_ERROR(22);
    GuestCond* c = (GuestCond*)bb_platform_vm_reserve(0, sizeof(GuestCond));
    if (!c) return BB_ORBIS_ERROR(12);
    bb_platform_vm_commit(c, sizeof(GuestCond), BB_PROT_READ | BB_PROT_WRITE);
    *out = c;
    return 0;
}

static BB_SYSV_ABI int32_t cond_wait(GuestCond** c, void** m) {
    if (!c || !*c || !m || !*m) return BB_ORBIS_ERROR(22);
    bb_platform_sleep_ms(1);
    return 0;
}

static BB_SYSV_ABI int32_t cond_timedwait(GuestCond** c, void** m, uint32_t usec) {
    if (!c || !*c || !m || !*m) return BB_ORBIS_ERROR(22);
    bb_platform_sleep_ns((uint64_t)usec * 1000ULL);
    return 0;
}

static BB_SYSV_ABI int32_t cond_signal(GuestCond** c) {
    return (c && *c) ? 0 : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t cond_broadcast(GuestCond** c) {
    return (c && *c) ? 0 : BB_ORBIS_ERROR(22);
}

static BB_SYSV_ABI int32_t cond_destroy(GuestCond** c) {
    if (!c || !*c) return BB_ORBIS_ERROR(22);
    bb_platform_vm_release(*c, sizeof(GuestCond));
    *c = NULL;
    return 0;
}

uintptr_t r_sync_cond_resolve(const char* name) {
    if (!strcmp(name, "2Tb92quprl0#p#J") || !strcmp(name, "0TyVk4MSLt0#I#J")) return (uintptr_t)cond_init;
    if (!strcmp(name, "g+PZd2hiacg#p#J") || !strcmp(name, "RXXqi4CtF8w#I#J")) return (uintptr_t)cond_destroy;
    if (!strcmp(name, "WKAXJ4XBPQ4#p#J") || !strcmp(name, "Op8TBGY5KHg#I#J")) return (uintptr_t)cond_wait;
    if (!strcmp(name, "BmMjYxmew1w#p#J") || !strcmp(name, "27bAgiJmOh0#I#J")) return (uintptr_t)cond_timedwait;
    if (!strcmp(name, "kDh-NfxgMtE#p#J") || !strcmp(name, "2MOy+rUfuhQ#I#J")) return (uintptr_t)cond_signal;
    if (!strcmp(name, "JGgj7Uvrl+A#p#J") || !strcmp(name, "mkx2fVhNMsg#I#J")) return (uintptr_t)cond_broadcast;
    return 0;
}
