/* SPDX-License-Identifier: MIT
 * PS4 HLE content subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_content implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/content/r_content_types.h"

uintptr_t runtime_content_resolve(const char *name);
void runtime_content_report(void);
