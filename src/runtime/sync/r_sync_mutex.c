/* SPDX-License-Identifier: MIT
 * PS4 HLE pthread mutex services.
 * Single responsibility: mutex lifecycle, locking, and NID bindings.
 */
#include "r_sync.h"
#include <stdlib.h>
#include <string.h>

typedef struct { BbMutex* native; uint32_t held; } GuestMutex;

static enum BbMutexType native_type(int type) {
    if (type == 2) return BB_MUTEX_TYPE_RECURSIVE;
    if (type == 3) return BB_MUTEX_TYPE_NORMAL;
    return BB_MUTEX_TYPE_ERRORCHECK;
}

static BB_SYSV_ABI int32_t mutex_init(GuestMutex** out, GuestMutexAttr** attr, const char* name) {
    BB_UNUSED(name);
    if (!out || (attr && !*attr)) return BB_ORBIS_ERROR(22);
    int type = attr ? (*attr)->type : 1;
    if (type < 1 || type > 4) return BB_ORBIS_ERROR(22);
    GuestMutex* m = (GuestMutex*)calloc(1, sizeof(*m));
    if (!m) return BB_ORBIS_ERROR(12);
    m->native = bb_platform_mutex_create(native_type(type));
    if (!m->native) { free(m); return BB_ORBIS_ERROR(12); }
    *out = m;
    return 0;
}

static int32_t ensure_mutex(GuestMutex** slot) {
    if (!slot || (uintptr_t)*slot == 2) return BB_ORBIS_ERROR(22);
    if ((uintptr_t)*slot >= 2) return 0;
    return mutex_init(slot, NULL, NULL);
}

static BB_SYSV_ABI int32_t mutex_lock(GuestMutex** m) {
    int32_t e = ensure_mutex(m);
    if (!e && !(e = bb_platform_mutex_lock((*m)->native))) ++(*m)->held;
    return e;
}
static BB_SYSV_ABI int32_t mutex_trylock(GuestMutex** m) {
    int32_t e = ensure_mutex(m);
    if (!e && !(e = bb_platform_mutex_trylock((*m)->native))) ++(*m)->held;
    return e;
}
static BB_SYSV_ABI int32_t mutex_timedlock(GuestMutex** m, uint32_t usec) {
    int32_t e = ensure_mutex(m);
    if (!e && !(e = bb_platform_mutex_timedlock((*m)->native, usec))) ++(*m)->held;
    return e;
}
static BB_SYSV_ABI int32_t mutex_unlock(GuestMutex** m) {
    if (!m || (uintptr_t)*m == 2) return BB_ORBIS_ERROR(22);
    if ((uintptr_t)*m < 2) return BB_ORBIS_ERROR(1);
    int32_t e = bb_platform_mutex_unlock((*m)->native);
    if (!e && (*m)->held) --(*m)->held;
    return e;
}
static BB_SYSV_ABI int32_t mutex_destroy(GuestMutex** m) {
    if (!m || (uintptr_t)*m == 2) return BB_ORBIS_ERROR(22);
    if ((uintptr_t)*m < 2) return 0;
    if ((*m)->held) return BB_ORBIS_ERROR(16);
    bb_platform_mutex_destroy((*m)->native);
    free(*m);
    *m = (GuestMutex*)(uintptr_t)2;
    return 0;
}

static int32_t posix_result(int32_t r) { return r ? (int32_t)((uint32_t)r & 0xffff) : 0; }
static BB_SYSV_ABI int32_t posix_init(GuestMutex** m, GuestMutexAttr** a) { return posix_result(mutex_init(m, a, NULL)); }
static BB_SYSV_ABI int32_t posix_destroy(GuestMutex** m) { return posix_result(mutex_destroy(m)); }
static BB_SYSV_ABI int32_t posix_lock(GuestMutex** m) { return posix_result(mutex_lock(m)); }
static BB_SYSV_ABI int32_t posix_trylock(GuestMutex** m) { return posix_result(mutex_trylock(m)); }
static BB_SYSV_ABI int32_t posix_unlock(GuestMutex** m) { return posix_result(mutex_unlock(m)); }

uintptr_t r_sync_mutex_resolve(const char* name) {
    uintptr_t attr = r_sync_mutex_attr_resolve(name);
    if (attr) return attr;
    if (!strcmp(name, "cmo1RIYva9o#p#J")) return (uintptr_t)mutex_init;
    if (!strcmp(name, "9UK1vLZQft4#p#J")) return (uintptr_t)mutex_lock;
    if (!strcmp(name, "upoVrzMHFeE#p#J")) return (uintptr_t)mutex_trylock;
    if (!strcmp(name, "tn3VlD0hG60#p#J")) return (uintptr_t)mutex_unlock;
    if (!strcmp(name, "2Of0f+3mhhE#p#J")) return (uintptr_t)mutex_destroy;
    if (!strcmp(name, "IafI2PxcPnQ#p#J")) return (uintptr_t)mutex_timedlock;
    if (!strcmp(name, "ttHNfU+qDBU#I#J")) return (uintptr_t)posix_init;
    if (!strcmp(name, "ltCfaGr2JGE#I#J")) return (uintptr_t)posix_destroy;
    if (!strcmp(name, "7H0iTOciTLo#I#J")) return (uintptr_t)posix_lock;
    if (!strcmp(name, "K-jXhbt2gn4#I#J")) return (uintptr_t)posix_trylock;
    if (!strcmp(name, "2Z+PpY6CaJg#I#J")) return (uintptr_t)posix_unlock;
    return 0;
}
