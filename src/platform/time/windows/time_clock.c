/* SPDX-License-Identifier: MIT
 * Windows real-time clock and timezone offset implementation.
 * Single responsibility: UTC wall clock, local time bias, and secure random. (~45 LOC)
 */
#ifdef _WIN32
#define _CRT_RAND_S
#include "platform/time.h"
#include <windows.h>
#include <stdlib.h>

uint64_t bb_platform_time_realtime_us(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    /* 116444736000000000 is 100ns intervals between 1601-01-01 and 1970-01-01 */
    uint64_t epoch_100ns = uli.QuadPart - 116444736000000000ULL;
    return epoch_100ns / 10ULL;
}

int64_t bb_platform_time_tz_offset_us(int64_t epoch_sec) {
    (void)epoch_sec;
    TIME_ZONE_INFORMATION tz;
    DWORD res = GetTimeZoneInformation(&tz);
    LONG bias = tz.Bias;
    if (res == TIME_ZONE_ID_DAYLIGHT) bias += tz.DaylightBias;
    else if (res == TIME_ZONE_ID_STANDARD) bias += tz.StandardBias;
    return -(int64_t)bias * 60LL * 1000000LL;
}

uint64_t bb_platform_random_u64(void) {
    unsigned int a = 0, b = 0;
    rand_s(&a);
    rand_s(&b);
    return ((uint64_t)a << 32) | b;
}
#endif
