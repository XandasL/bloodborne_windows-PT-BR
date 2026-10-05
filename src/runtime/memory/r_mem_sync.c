/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory Runtime Synchronization.
 * Single responsibility: RWLock and table generation tracking. (~50 LOC)
 */
#include "r_mem_types.h"

static BbRwlock *mem_lock = NULL;
static _Thread_local unsigned exclusive_depth;
static uint64_t table_generation;

static void ensure_lock(void) {
    if (!mem_lock) {
        BbRwlock *l = bb_platform_rwlock_create();
        if (!__sync_bool_compare_and_swap(&mem_lock, NULL, l)) {
            bb_platform_rwlock_destroy(l);
        }
    }
}

void r_mem_write_lock(void) {
    ensure_lock();
    if (!exclusive_depth++) {
        bb_platform_rwlock_wrlock(mem_lock);
        __atomic_add_fetch(&table_generation, 1, __ATOMIC_ACQ_REL);
    }
}

void r_mem_write_unlock(void) {
    if (!--exclusive_depth) {
        __atomic_add_fetch(&table_generation, 1, __ATOMIC_RELEASE);
        bb_platform_rwlock_unlock(mem_lock);
    }
}

void r_mem_read_lock(void) {
    ensure_lock();
    if (!exclusive_depth) bb_platform_rwlock_rdlock(mem_lock);
}

void r_mem_read_unlock(void) {
    if (!exclusive_depth) bb_platform_rwlock_unlock(mem_lock);
}

uint64_t r_mem_generation(void) {
    return __atomic_load_n(&table_generation, __ATOMIC_ACQUIRE);
}
