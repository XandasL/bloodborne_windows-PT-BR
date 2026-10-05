/* SPDX-License-Identifier: MIT
 * PS4 RTC tick and timestamp conversions.
 * Single responsibility: Converting between microsecond ticks and calendar time. (~55 LOC)
 */
#include "r_rtc_types.h"

uint64_t rtc_to_tick(const DateTime *t) {
    return (uint64_t)rtc_days_from_civil(t->year, t->month, t->day) * TICKS_PER_DAY +
           ((uint64_t)t->hour * 3600 + (uint64_t)t->minute * 60 + t->second) * 1000000 + t->microsecond;
}

void rtc_from_tick(uint64_t tick, DateTime *t) {
    int y, m, d;
    rtc_civil_from_days((int64_t)(tick / TICKS_PER_DAY), &y, &m, &d);
    uint64_t us = tick % TICKS_PER_DAY;
    t->year = (uint16_t)y;
    t->month = (uint16_t)m;
    t->day = (uint16_t)d;
    t->hour = (uint16_t)(us / 3600000000u);
    t->minute = (uint16_t)(us / 60000000u % 60);
    t->second = (uint16_t)(us / 1000000u % 60);
    t->microsecond = (uint32_t)(us % 1000000u);
}

uint64_t rtc_now_utc(void) {
    return UNIX_EPOCH_TICKS + bb_platform_time_realtime_us();
}

int64_t rtc_local_offset(uint64_t utc) {
    int64_t epoch_sec = (int64_t)(utc - UNIX_EPOCH_TICKS) / 1000000;
    return bb_platform_time_tz_offset_us(epoch_sec);
}
