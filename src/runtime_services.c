/* SPDX-License-Identifier: MIT
 * PS4 HLE system services subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_serv implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/services/r_serv_types.h"

uintptr_t runtime_services_resolve(const char *name);
