/* SPDX-License-Identifier: MIT
 * PS4 HLE RTC subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_rtc implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/rtc/r_rtc_types.h"

uintptr_t runtime_rtc_resolve(const char *name);
