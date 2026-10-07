#pragma once
#include "test_sema_ops.h"

static void concurrency(void) {
    uint32_t id;
    assert(create(&id, "fifo", 1, 0, 3, NULL) == 0);
    Task a = {id, 1, 99, 5000000}, b = {id, 1, 99, 5000000};
    pthread_t first, second;
    assert(pthread_create(&first, NULL, waiter, &a) == 0); enrolled(id, 1);
    assert(pthread_create(&second, NULL, waiter, &b) == 0); enrolled(id, 2);
    assert(signal_sem(id, 1) == 0);
    assert(pthread_join(first, NULL) == 0 && a.result == 0 && a.timeout <= 5000000);
    assert(runtime_sema_waiters(id) == 1 && (uint32_t)poll_sem(id, 1) == 0x80020010);
    assert(signal_sem(id, 1) == 0);
    assert(pthread_join(second, NULL) == 0 && b.result == 0);
    a = (Task){id, 2, 99, 5000000}; b = (Task){id, 1, 99, 5000000};
    assert(pthread_create(&first, NULL, waiter, &a) == 0); enrolled(id, 1);
    assert(pthread_create(&second, NULL, waiter, &b) == 0); enrolled(id, 2);
    assert(signal_sem(id, 1) == 0);
    assert(pthread_join(second, NULL) == 0 && b.result == 0);
    assert(runtime_sema_waiters(id) == 1);
    assert(signal_sem(id, 2) == 0);
    assert(pthread_join(first, NULL) == 0 && a.result == 0);
    assert(delete_sem(id) == 0);
}

static void cancellation(void) {
    uint32_t id;
    assert(create(&id, "cancel", 1, 0, 2, NULL) == 0);
    Task a = {id, 1, 99, 5000000}, b = {id, 1, 99, 5000000};
    pthread_t first, second;
    assert(pthread_create(&first, NULL, waiter, &a) == 0); enrolled(id, 1);
    assert(pthread_create(&second, NULL, waiter, &b) == 0); enrolled(id, 2);
    int32_t n = -1;
    assert(cancel_sem(id, 1, &n) == 0 && n == 2);
    assert(pthread_join(first, NULL) == 0 && (uint32_t)a.result == 0x80020055);
    assert(pthread_join(second, NULL) == 0 && (uint32_t)b.result == 0x80020055);
    assert(a.timeout == 0 && b.timeout == 0 && poll_sem(id, 1) == 0);
    a = (Task){id, 1, 99, 5000000}; b = (Task){id, 1, 99, 5000000};
    assert(pthread_create(&first, NULL, waiter, &a) == 0); enrolled(id, 1);
    assert(pthread_create(&second, NULL, waiter, &b) == 0); enrolled(id, 2);
    assert(delete_sem(id) == 0);
    assert(pthread_join(first, NULL) == 0 && (uint32_t)a.result == 0x8002000d);
    assert(pthread_join(second, NULL) == 0 && (uint32_t)b.result == 0x8002000d);
    assert((uint32_t)poll_sem(id, 1) == 0x80020003);
}
