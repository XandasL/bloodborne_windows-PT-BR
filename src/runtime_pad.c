/* SPDX-License-Identifier: MIT
 * PS4 HLE pad subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_pad implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/pad/r_pad_types.h"

uintptr_t runtime_pad_resolve(const char *name);
void runtime_pad_report(void);
