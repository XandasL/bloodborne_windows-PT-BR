/* SPDX-License-Identifier: MIT
 * Linux real-time clock and timezone offset implementation.
 * Single responsibility: CLOCK_REALTIME, tm_gmtoff, and secure random. (~45 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/time.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t bb_platform_time_realtime_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

int64_t bb_platform_time_tz_offset_us(int64_t epoch_sec) {
    time_t seconds = (time_t)epoch_sec;
    struct tm local;
    localtime_r(&seconds, &local);
    return (int64_t)local.tm_gmtoff * 1000000LL;
}

uint64_t bb_platform_random_u64(void) {
    uint64_t val = 0;
    FILE *f = fopen("/dev/urandom", "rb");
    if (f) {
        (void)fread(&val, sizeof(val), 1, f);
        fclose(f);
    }
    return val;
}
#endif
