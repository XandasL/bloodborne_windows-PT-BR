/* SPDX-License-Identifier: MIT
 * PS4 HLE pthread mutex attributes.
 * Single responsibility: mutexattr lifecycle, validation, and POSIX wrappers.
 */
#include "r_sync.h"
#include <stdlib.h>
#include <string.h>

static BB_SYSV_ABI int32_t attr_init(GuestMutexAttr** out) {
    if (!out) return BB_ORBIS_ERROR(22);
    GuestMutexAttr* a = (GuestMutexAttr*)calloc(1, sizeof(*a));
    if (!a) return BB_ORBIS_ERROR(12);
    a->type = 1;
    *out = a;
    return 0;
}

static BB_SYSV_ABI int32_t attr_type(GuestMutexAttr** attr, int type) {
    if (!attr || !*attr || type < 1 || type > 4) return BB_ORBIS_ERROR(22);
    (*attr)->type = type;
    return 0;
}

static BB_SYSV_ABI int32_t attr_protocol(GuestMutexAttr** attr, int protocol) {
    if (!attr || !*attr || protocol < 0 || protocol > 2) return BB_ORBIS_ERROR(22);
    (*attr)->protocol = protocol;
    return 0;
}

static BB_SYSV_ABI int32_t attr_destroy(GuestMutexAttr** attr) {
    if (!attr || !*attr) return BB_ORBIS_ERROR(22);
    free(*attr);
    *attr = NULL;
    return 0;
}

static int32_t posix_result(int32_t r) {
    return r ? (int32_t)((uint32_t)r & 0xffff) : 0;
}
static BB_SYSV_ABI int32_t posix_attr_init(GuestMutexAttr** a) { return posix_result(attr_init(a)); }
static BB_SYSV_ABI int32_t posix_attr_type(GuestMutexAttr** a, int t) { return posix_result(attr_type(a, t)); }
static BB_SYSV_ABI int32_t posix_attr_destroy(GuestMutexAttr** a) { return posix_result(attr_destroy(a)); }

uintptr_t r_sync_mutex_attr_resolve(const char* name) {
    if (!strcmp(name, "F8bUHwAG284#p#J")) return (uintptr_t)attr_init;
    if (!strcmp(name, "iMp8QpE+XO4#p#J")) return (uintptr_t)attr_type;
    if (!strcmp(name, "smWEktiyyG0#p#J")) return (uintptr_t)attr_destroy;
    if (!strcmp(name, "1FGvU0i9saQ#p#J")) return (uintptr_t)attr_protocol;
    if (!strcmp(name, "dQHWEsJtoE4#I#J")) return (uintptr_t)posix_attr_init;
    if (!strcmp(name, "mDmgMOGVUqg#I#J")) return (uintptr_t)posix_attr_type;
    if (!strcmp(name, "HF7lK46xzjY#I#J")) return (uintptr_t)posix_attr_destroy;
    return 0;
}
