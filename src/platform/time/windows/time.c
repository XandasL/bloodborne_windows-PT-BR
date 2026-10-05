/* SPDX-License-Identifier: MIT
 * Windows high-resolution time, clock, and sleep implementation.
 * Single responsibility: Win32 QPC timing & sleep services. (~70 LOC)
 */
#include "platform/time.h"
#include <windows.h>
#include <intrin.h>

static uint64_t get_qpc_freq(void) {
    static uint64_t freq = 0;
    if (!freq) {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        freq = (uint64_t)f.QuadPart;
    }
    return freq;
}

uint64_t bb_platform_time_ns(void) {
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    uint64_t freq = get_qpc_freq();
    return (uint64_t)c.QuadPart * 1000000000ULL / freq;
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
        uint64_t t0 = __rdtsc();
        uint64_t q0 = bb_platform_time_ns();
        Sleep(20);
        uint64_t t1 = __rdtsc();
        uint64_t q1 = bb_platform_time_ns();
        uint64_t dt_ns = q1 - q0;
        tsc_hz = dt_ns ? (t1 - t0) * 1000000000ULL / dt_ns : 3000000000ULL;
    }
    return tsc_hz;
}

void bb_platform_sleep_ns(uint64_t ns) {
    if (ns >= 1000000ULL) {
        Sleep((DWORD)(ns / 1000000ULL));
    } else {
        uint64_t target = bb_platform_time_ns() + ns;
        while (bb_platform_time_ns() < target) {
            _mm_pause();
        }
    }
}

void bb_platform_sleep_ms(uint32_t ms) {
    Sleep((DWORD)ms);
}
