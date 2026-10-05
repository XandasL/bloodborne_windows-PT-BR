/* SPDX-License-Identifier: MIT
 * PS4 HLE filesystem subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_fs implementation. (~25 LOC)
 */
#include "runtime.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_file_resolve(const char* name);
void runtime_file_report(void);
void runtime_file_configure(const char* app0, const char* user);
