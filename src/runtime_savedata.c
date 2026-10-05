/* SPDX-License-Identifier: MIT
 * PS4 HLE savedata subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_savedata implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/savedata/r_savedata_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_savedata_resolve(const char *name);
void runtime_savedata_report(void);
void runtime_savedata_configure(const char *title);
