/* SPDX-License-Identifier: MIT
 * Windows Platform Threads, Mutex, Semaphore, TLS & Timing Test.
 */
#include "platform/threads.h"
#include "platform/sync.h"
#include "platform/time.h"
#include "platform/bb_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static BbMutex* g_mutex;
static BbSema*  g_sema;
static volatile int g_counter = 0;
static void* volatile g_cas_target = NULL;

static void* worker_thread(void* arg) {
    int id = (int)(intptr_t)arg;
    bb_platform_tls_set_guest((void*)(intptr_t)(id * 100));

    // Test mutex
    bb_platform_mutex_lock(g_mutex);
    g_counter++;
    bb_platform_mutex_unlock(g_mutex);

    // Verify Guest TLS
    void* val = bb_platform_tls_get_guest();
    assert((int)(intptr_t)val == id * 100);

    // Signal completion
    bb_platform_sema_signal(g_sema, 1);
    return (void*)(intptr_t)42;
}

int main(void) {
    printf("[TEST] Starting Windows Threads, Sync, TLS & Timing Test...\n");

    // 1. Time Test
    uint64_t t0 = bb_platform_time_ns();
    bb_platform_sleep_ns(10 * 1000 * 1000); // 10ms
    uint64_t t1 = bb_platform_time_ns();
    uint64_t elapsed_us = (t1 - t0) / 1000;
    printf("  [PASS] High-resolution timer: 10ms sleep measured as %llu us\n", (unsigned long long)elapsed_us);
    assert(elapsed_us >= 5000); // at least 5ms passed

    // 2. Portable CAS Test
    void* val1 = (void*)0x1111;
    void* val2 = (void*)0x2222;
    bool cas1 = bb_atomic_cas_ptr(&g_cas_target, NULL, val1);
    bool cas2 = bb_atomic_cas_ptr(&g_cas_target, NULL, val2); // should fail because it's now val1
    bool cas3 = bb_atomic_cas_ptr(&g_cas_target, val1, val2); // should succeed
    assert(cas1 && !cas2 && cas3 && g_cas_target == val2);
    printf("  [PASS] Portable atomic CAS (bb_atomic_cas_ptr) verified\n");

    // 3. Mutex & Semaphore
    g_mutex = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
    g_sema = bb_platform_sema_create(0, 10);
    assert(g_mutex && g_sema);

    const int NUM_THREADS = 4;
    BbThread* threads[4];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = bb_platform_thread_create(worker_thread, (void*)(intptr_t)(i + 1), 64 * 1024, "worker");
        assert(threads[i] != NULL);
    }
    printf("  [PASS] Spawned %d worker threads via _beginthreadex\n", NUM_THREADS);

    // Wait for all semaphores
    uint32_t timeout_us = 5000000; // 5 sec timeout
    for (int i = 0; i < NUM_THREADS; i++) {
        int w = bb_platform_sema_wait(g_sema, 1, &timeout_us);
        assert(w == 0);
    }
    printf("  [PASS] All %d threads signaled semaphore\n", NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) {
        void* res = NULL;
        bb_platform_thread_join(threads[i], &res);
        assert((int)(intptr_t)res == 42);
    }
    printf("  [PASS] Joined all threads cleanly with expected exit codes\n");

    assert(g_counter == NUM_THREADS);
    printf("  [PASS] Mutex protected counter matched: %d == %d\n", g_counter, NUM_THREADS);

    bb_platform_sema_destroy(g_sema);
    bb_platform_mutex_destroy(g_mutex);

    printf("[SUCCESS] Windows Threads, Sync, TLS & Timing test PASSED!\n");
    return 0;
}
