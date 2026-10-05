/* SPDX-License-Identifier: MIT
 * PS4 HLE ajm subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_ajm implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/ajm/r_ajm_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_ajm_resolve(const char *name);
void runtime_ajm_report(void);
