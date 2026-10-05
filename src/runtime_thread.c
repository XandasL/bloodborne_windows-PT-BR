/* SPDX-License-Identifier: MIT
 * PS4 HLE thread subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_thread implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/threads/r_thread_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_thread_resolve(const char *name);
void runtime_thread_report(void);
void runtime_thread_attach_main(void);
void runtime_set_main_tls(const void *d, uint64_t f, uint64_t m, uint64_t a);
