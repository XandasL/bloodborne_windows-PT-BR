/* SPDX-License-Identifier: MIT
 * PS4 HLE synchronization reporting and runtime dispatch.
 * Single responsibility: Diagnostic reports and resolver bridge. (~40 LOC)
 */
#include "r_sync.h"
#include <stdio.h>

void r_sync_mutex_report(void) {
    puts("Runtime: mutex & condition variable subsystem active");
}

void r_sync_rwlock_report(void) {
    puts("Runtime: rwlock subsystem active");
}

void r_sync_sema_report(void) {
    puts("Runtime: semaphore subsystem active");
}

uintptr_t runtime_mutex_resolve(const char* name) {
    uintptr_t m = r_sync_mutex_resolve(name);
    return m ? m : r_sync_cond_resolve(name);
}

void runtime_mutex_report(void) {
    r_sync_mutex_report();
}

uintptr_t runtime_rwlock_resolve(const char* name) {
    return r_sync_rwlock_resolve(name);
}

void runtime_rwlock_report(void) {
    r_sync_rwlock_report();
}

uintptr_t runtime_sema_resolve(const char* name) {
    return r_sync_sema_resolve(name);
}

void runtime_sema_report(void) {
    r_sync_sema_report();
}
