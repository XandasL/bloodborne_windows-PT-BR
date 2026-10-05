/* SPDX-License-Identifier: MIT
 * PS4 HLE audio subsystem entry point (forwarder).
 * Single responsibility: Bridge to modular r_audio implementation. (~15 LOC)
 */
#include "runtime.h"
#include "runtime/audio/r_audio_types.h"

/* Forwarding declarations for runtime bridge */
uintptr_t runtime_audio_resolve(const char *name);
void runtime_audio_report(void);
