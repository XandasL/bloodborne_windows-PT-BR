/* SPDX-License-Identifier: MIT
 * PS4 HLE memory subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_mem implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/memory/r_mem_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_memory_resolve(const char *name);
void runtime_memory_report(void);
void *runtime_low_map(size_t size, int prot);
int runtime_memory_is_mapped(uintptr_t address, uint64_t size);
