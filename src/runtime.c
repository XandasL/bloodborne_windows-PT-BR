/* SPDX-License-Identifier: MIT
 * PS4 HLE Core Runtime Entry Point (forwarder).
 * Single responsibility: Bridge to modular src/runtime/core/ units. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/core/r_core_types.h"

void runtime_start(uint64_t flags);
void runtime_report(void);
uintptr_t runtime_resolve(const char *name, int is_data);
