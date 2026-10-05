/* SPDX-License-Identifier: MIT
 * PS4 HLE semaphore subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_sync implementation. (~25 LOC)
 */
#include "runtime.h"
#include "runtime/sync/r_sync.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_sema_resolve(const char* name);
void runtime_sema_report(void);
unsigned runtime_sema_waiters(uint32_t id) {
    BB_UNUSED(id);
    return 0;
}
