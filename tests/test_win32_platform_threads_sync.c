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
    bb_platform_mutex_lock(g_mutex);
    g_counter++;
    bb_platform_mutex_unlock(g_mutex);
    assert((int)(intptr_t)bb_platform_tls_get_guest() == id * 100);
    bb_platform_sema_signal(g_sema, 1);
    return (void*)(intptr_t)42;
}

static void test_time_and_cas(void) {
    uint64_t t0 = bb_platform_time_ns();
    bb_platform_sleep_ns(10 * 1000 * 1000);
    assert((bb_platform_time_ns() - t0) / 1000 >= 5000);
    void *val1 = (void*)0x1111, *val2 = (void*)0x2222;
    assert(bb_atomic_cas_ptr(&g_cas_target, NULL, val1));
    assert(!bb_atomic_cas_ptr(&g_cas_target, NULL, val2));
    assert(bb_atomic_cas_ptr(&g_cas_target, val1, val2) && g_cas_target == val2);
}

int main(void) {
    printf("[TEST] Starting Windows Threads, Sync, TLS & Timing Test...\n");
    test_time_and_cas();
    g_mutex = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
    g_sema = bb_platform_sema_create(0, 10);
    assert(g_mutex && g_sema);

    const int NUM = 4;
    BbThread* threads[4];
    for (int i = 0; i < NUM; i++) {
        threads[i] = bb_platform_thread_create(worker_thread, (void*)(intptr_t)(i + 1), 64 * 1024, "worker");
        assert(threads[i] != NULL);
    }
    uint32_t timeout_us = 5000000;
    for (int i = 0; i < NUM; i++) assert(bb_platform_sema_wait(g_sema, 1, &timeout_us) == 0);
    for (int i = 0; i < NUM; i++) {
        void* res = NULL; bb_platform_thread_join(threads[i], &res);
        assert((int)(intptr_t)res == 42);
    }
    assert(g_counter == NUM);
    bb_platform_sema_destroy(g_sema);
    bb_platform_mutex_destroy(g_mutex);
    printf("[SUCCESS] Windows Threads, Sync, TLS & Timing test PASSED!\n");
    return 0;
}
