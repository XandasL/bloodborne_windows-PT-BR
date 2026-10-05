/* SPDX-License-Identifier: MIT
 * Linux POSIX time, clock, and sleep implementation.
 * Single responsibility: POSIX monotonic clock & nanosleep. (~60 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/time.h"
#include <time.h>
#include <x86intrin.h>

uint64_t bb_platform_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

uint64_t bb_platform_time_us(void) {
    return bb_platform_time_ns() / 1000ULL;
}

uint64_t bb_platform_time_ms(void) {
    return bb_platform_time_ns() / 1000000ULL;
}

uint64_t bb_platform_tsc_read(void) {
    return __rdtsc();
}

uint64_t bb_platform_tsc_frequency(void) {
    static uint64_t tsc_hz = 0;
    if (!tsc_hz) {
        struct timespec nap = {0, 20000000};
        uint64_t t0 = __rdtsc();
        uint64_t q0 = bb_platform_time_ns();
        nanosleep(&nap, NULL);
        uint64_t t1 = __rdtsc();
        uint64_t q1 = bb_platform_time_ns();
        uint64_t dt_ns = q1 - q0;
        tsc_hz = dt_ns ? (t1 - t0) * 1000000000ULL / dt_ns : 3000000000ULL;
    }
    return tsc_hz;
}

void bb_platform_sleep_ns(uint64_t ns) {
    struct timespec req = {
        .tv_sec = (time_t)(ns / 1000000000ULL),
        .tv_nsec = (long)(ns % 1000000000ULL)
    };
    while (nanosleep(&req, &req) != 0) {}
}

void bb_platform_sleep_ms(uint32_t ms) {
    bb_platform_sleep_ns((uint64_t)ms * 1000000ULL);
}
#endif
