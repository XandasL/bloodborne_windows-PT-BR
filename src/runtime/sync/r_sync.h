/* SPDX-License-Identifier: MIT
 * PS4 HLE synchronization services internal header.
 * Single responsibility: Sync subsystem declarations and exports. (~35 LOC)
 */
#ifndef R_SYNC_H
#define R_SYNC_H

#include "bb_common.h"
#include "platform/sync.h"
#include "platform/time.h"
#include "platform/memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GuestMutexAttr {
    int type;
    int protocol;
} GuestMutexAttr;

uintptr_t r_sync_mutex_attr_resolve(const char* name);
uintptr_t r_sync_mutex_resolve(const char* name);
uintptr_t r_sync_cond_resolve(const char* name);
uintptr_t r_sync_rwlock_resolve(const char* name);
uintptr_t r_sync_sema_resolve(const char* name);

void r_sync_mutex_report(void);
void r_sync_rwlock_report(void);
void r_sync_sema_report(void);

#ifdef __cplusplus
}
#endif

#endif /* R_SYNC_H */
