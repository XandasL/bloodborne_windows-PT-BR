/* SPDX-License-Identifier: MIT
 * PS4 HLE kernel subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_kernel implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/kernel/r_kernel_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_kernel_resolve(const char *name);
int32_t runtime_guest_errno(int e);
void runtime_thread_keys_cleanup(void);
