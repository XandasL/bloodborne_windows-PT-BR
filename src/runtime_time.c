/* SPDX-License-Identifier: MIT
 * PS4 HLE gettimeofday service.
 * Single responsibility: Guest gettimeofday layout & platform dispatch. (~35 LOC)
 */
#include "runtime.h"
#include "platform/time.h"
#include <string.h>

typedef struct { int64_t seconds, microseconds; } GuestTimeval;
typedef struct { int32_t minuteswest, dsttime; } GuestTimezone;

static BB_SYSV_ABI int guest_gettimeofday(GuestTimeval* value, GuestTimezone* zone) {
    uint64_t us = bb_platform_time_us();
    if (value) {
        value->seconds = (int64_t)(us / 1000000ULL);
        value->microseconds = (int64_t)(us % 1000000ULL);
    }
    if (zone) {
        zone->minuteswest = 0;
        zone->dsttime = 0;
    }
    return 0;
}

uintptr_t runtime_time_resolve(const char* name) {
    if (!strcmp(name, "n88vx3C5nW8#I#J")) return (uintptr_t)guest_gettimeofday;
    return 0;
}
