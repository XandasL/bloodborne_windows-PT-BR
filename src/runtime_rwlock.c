/* SPDX-License-Identifier: MIT
 * PS4 HLE rwlock subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_sync implementation. (~25 LOC)
 */
#include "runtime.h"
#include "runtime/sync/r_sync.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_rwlock_resolve(const char* name);
void runtime_rwlock_report(void);
