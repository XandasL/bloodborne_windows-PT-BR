/* SPDX-License-Identifier: MIT
 * PS4 Core Static Initialization Guards and Memory Wrappers.
 * Single responsibility: Thread-safe C++ dynamic guards and memory primitive counting. (~70 LOC)
 */
#include "r_core_types.h"

_Atomic size_t g_runtime_guards_acquired = 0;
_Atomic size_t g_runtime_guards_released = 0;
_Atomic size_t g_runtime_memory_calls = 0;
static _Atomic uint32_t next_guard_owner = 1;
static _Thread_local uint32_t guard_owner = 0;

ABI int guard_acquire(uint64_t *guard) {
    _Atomic uint64_t *state = (_Atomic uint64_t *)guard;
    if (!guard_owner) guard_owner = atomic_fetch_add(&next_guard_owner, 1);
    for (;;) {
        uint64_t expected = 0;
        if (atomic_load_explicit(state, memory_order_acquire) & 1) return 0;
        if (atomic_compare_exchange_strong_explicit(state, &expected, (uint64_t)guard_owner << 32,
                                                    memory_order_acq_rel, memory_order_acquire)) break;
        if (expected & 1) return 0;
        if ((uint32_t)(expected >> 32) == guard_owner) {
            fputs("STOP: recursive/concurrent static initialization is not supported yet\n", stderr);
            exit(21);
        }
        bb_platform_thread_yield();
    }
    atomic_fetch_add(&g_runtime_guards_acquired, 1);
    return 1;
}

ABI void guard_release(uint64_t *guard) {
    atomic_store_explicit((_Atomic uint64_t *)guard, 1, memory_order_release);
    atomic_fetch_add(&g_runtime_guards_released, 1);
}

ABI void guard_abort(uint64_t *guard) {
    atomic_fetch_and_explicit((_Atomic uint64_t *)guard, UINT64_C(0xffffffff), memory_order_release);
}

ABI __attribute__((noreturn)) void stack_fail(void) {
    fputs("STOP: guest stack protector detected corruption\n", stderr);
    exit(22);
}

ABI void *guest_memset(void *dst, int value, size_t size) {
    atomic_fetch_add_explicit(&g_runtime_memory_calls, 1, memory_order_relaxed);
    return memset(dst, value, size);
}

ABI void *guest_memcpy(void *dst, const void *src, size_t size) {
    atomic_fetch_add_explicit(&g_runtime_memory_calls, 1, memory_order_relaxed);
    return memcpy(dst, src, size);
}

ABI void *guest_memmove(void *dst, const void *src, size_t size) {
    atomic_fetch_add_explicit(&g_runtime_memory_calls, 1, memory_order_relaxed);
    return memmove(dst, src, size);
}

ABI int guest_memcmp(const void *a, const void *b, size_t size) {
    atomic_fetch_add_explicit(&g_runtime_memory_calls, 1, memory_order_relaxed);
    return memcmp(a, b, size);
}

ABI size_t guest_strlen(const char *text) { return strlen(text); }
