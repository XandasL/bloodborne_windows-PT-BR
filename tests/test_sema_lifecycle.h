#pragma once
#include "test_sema_ops.h"

static void lifecycle(void) {
    struct { uint32_t handle, canary; } s = {0, 0xdeadbeef};
    assert((uint32_t)create(NULL, "test", 1, 0, 1, NULL) == 0x80020016);
    assert((uint32_t)create(&s.handle, "test", 1, 2, 1, NULL) == 0x80020016);
    assert(create(&s.handle, "test", 2, 2, 3, NULL) == 0 && s.canary == 0xdeadbeef);
    uint32_t old = s.handle, timeout = 1000000;
    assert(wait_sem(s.handle, 2, &timeout) == 0 && timeout == 1000000);
    assert((uint32_t)poll_sem(s.handle, 1) == 0x80020010);
    timeout = 0;
    assert((uint32_t)wait_sem(s.handle, 1, &timeout) == 0x8002003c);
    assert((uint32_t)signal_sem(s.handle, 4) == 0x80020016);
    assert((uint32_t)poll_sem(s.handle, 0) == 0x80020016);
    assert((uint32_t)signal_sem(s.handle, -1) == 0x80020016);
    assert(signal_sem(s.handle, 3) == 0);
    assert((uint32_t)signal_sem(s.handle, INT_MAX) == 0x80020016);
    assert(poll_sem(s.handle, 3) == 0);
    int32_t n = -1;
    assert(cancel_sem(s.handle, -1, &n) == 0 && n == 0);
    assert(poll_sem(s.handle, 2) == 0);
    assert(delete_sem(s.handle) == 0);
    assert((uint32_t)delete_sem(s.handle) == 0x80020003);
    assert((uint32_t)signal_sem(s.handle, 1) == 0x80020003);
    assert(create(&s.handle, "test", 1, 0, 1, NULL) == 0 && s.handle != old);
    assert((uint32_t)poll_sem(old, 1) == 0x80020003);
    timeout = 1000;
    assert((uint32_t)wait_sem(s.handle, 1, &timeout) == 0x8002003c && timeout == 0);
    assert(runtime_sema_waiters(s.handle) == 0);
    assert(signal_sem(s.handle, 1) == 0 && poll_sem(s.handle, 1) == 0);
    assert(delete_sem(s.handle) == 0);
}
