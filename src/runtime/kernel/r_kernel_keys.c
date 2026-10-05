/* SPDX-License-Identifier: MIT
 * PS4 Dynamic Once and Thread-Specific Data Keys.
 * Single responsibility: Pthread once dispatch and TLS key slots. (~68 LOC)
 */
#include "r_kernel_types.h"
#include "platform/threads.h"

#define KEYS 256
static struct { int used; KeyDestructor destructor; } keys[KEYS];
static BbMutex *key_lock = NULL;
static _Thread_local void *key_values[KEYS];

static void ensure_lock(void) {
    if (!key_lock) {
        BbMutex *m = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
        if (!__sync_bool_compare_and_swap(&key_lock, NULL, m)) bb_platform_mutex_destroy(m);
    }
}

ABI int32_t thread_once(int32_t *once, void (ABI *routine)(void)) {
    if (!once || !routine) return ERR(22);
    for (;;) {
        int32_t s = __atomic_load_n(once, __ATOMIC_ACQUIRE);
        if (s == 1) return 0;
        if (s == 0 && __atomic_compare_exchange_n(once, &s, 2, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
            routine(); __atomic_store_n(once, 1, __ATOMIC_RELEASE); return 0;
        }
        bb_platform_thread_yield();
    }
}
ABI int32_t posix_once(int32_t *once, void (ABI *routine)(void)) { return thread_once(once, routine) ? 22 : 0; }

ABI int32_t key_create(uint32_t *key, KeyDestructor destructor) {
    if (!key) return ERR(22);
    ensure_lock(); bb_platform_mutex_lock(key_lock);
    for (uint32_t i = 1; i < KEYS; ++i) if (!keys[i].used) {
        keys[i].used = 1; keys[i].destructor = destructor;
        bb_platform_mutex_unlock(key_lock);
        *key = i; return 0;
    }
    bb_platform_mutex_unlock(key_lock);
    return ERR(35);
}

ABI int32_t key_delete(uint32_t key) {
    if (!key || key >= KEYS) return ERR(22);
    ensure_lock(); bb_platform_mutex_lock(key_lock);
    int32_t r = keys[key].used ? 0 : ERR(22);
    keys[key].used = 0; keys[key].destructor = NULL;
    bb_platform_mutex_unlock(key_lock);
    return r;
}

ABI int32_t key_set(uint32_t key, void *value) {
    if (!key || key >= KEYS || !keys[key].used) return ERR(22);
    key_values[key] = value; return 0;
}
ABI void *key_get(uint32_t key) { return (key && key < KEYS) ? key_values[key] : NULL; }

void runtime_thread_keys_cleanup(void) {
    for (int round = 0; round < 4; ++round) {
        int any = 0;
        for (uint32_t i = 1; i < KEYS; ++i) {
            void *val = key_values[i];
            if (!val || !keys[i].used || !keys[i].destructor) continue;
            key_values[i] = NULL; any = 1; keys[i].destructor(val);
        }
        if (!any) break;
    }
}

ABI int32_t posix_key_create(uint32_t *k, KeyDestructor d) { int32_t r = key_create(k, d); return r ? r & 0xffff : 0; }
ABI int32_t posix_key_delete(uint32_t k) { int32_t r = key_delete(k); return r ? r & 0xffff : 0; }
ABI int32_t posix_key_set(uint32_t k, void *v) { int32_t r = key_set(k, v); return r ? r & 0xffff : 0; }
