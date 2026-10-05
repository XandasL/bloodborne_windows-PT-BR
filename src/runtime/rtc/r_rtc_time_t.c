/* SPDX-License-Identifier: MIT
 * PS4 RTC time_t and POSIX time conversions.
 * Single responsibility: Converting between unix time_t and Orbis ticks. (~60 LOC)
 */
#include "r_rtc_types.h"

int32_t rtc_day_of_week(int32_t year, int32_t month, int32_t day) {
    if (year < 1 || year > 9999) return ERR_INVALID_YEAR;
    if (month < 1 || month > 12) return ERR_INVALID_MONTH;
    if (day < 1 || day > rtc_days_in_month(year, month)) return ERR_INVALID_DAY;
    return (int32_t)((rtc_days_from_civil(year, month, day) + 1) % 7);
}

ABI int32_t rtc_get_tick(const DateTime *t, uint64_t *tick) {
    if (!t || !tick) return ERR_INVALID_POINTER;
    int32_t e = rtc_validate(t);
    if (e) return e;
    *tick = rtc_to_tick(t);
    return 0;
}

ABI int32_t rtc_set_tick(DateTime *t, const uint64_t *tick) {
    if (!t || !tick) return ERR_INVALID_POINTER;
    rtc_from_tick(*tick, t);
    return 0;
}

ABI uint32_t rtc_tick_resolution(void) {
    return 1000000;
}

ABI int32_t rtc_set_time_t(DateTime *t, int64_t seconds) {
    if (!t) return ERR_INVALID_POINTER;
    if (seconds < 0) return ERR_INVALID_VALUE;
    rtc_from_tick(UNIX_EPOCH_TICKS + (uint64_t)seconds * 1000000u, t);
    return 0;
}

ABI int32_t rtc_get_time_t(const DateTime *t, int64_t *seconds) {
    if (!t || !seconds) return ERR_INVALID_POINTER;
    int32_t e = rtc_validate(t);
    if (e) return e;
    uint64_t tick = rtc_to_tick(t);
    *seconds = tick < UNIX_EPOCH_TICKS ? 0 : (int64_t)((tick - UNIX_EPOCH_TICKS) / 1000000);
    return 0;
}
