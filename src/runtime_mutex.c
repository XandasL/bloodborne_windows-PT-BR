/* SPDX-License-Identifier: MIT
 * PS4 HLE mutex subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_sync implementation. (~25 LOC)
 */
#include "runtime.h"
#include "runtime/sync/r_sync.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_mutex_resolve(const char* name);
void runtime_mutex_report(void);
