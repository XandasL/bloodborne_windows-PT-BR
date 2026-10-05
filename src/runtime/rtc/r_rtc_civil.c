/* SPDX-License-Identifier: MIT
 * PS4 RTC civil calendar conversion algorithms.
 * Single responsibility: Gregorian civil calculations and range validation. (~55 LOC)
 */
#include "r_rtc_types.h"

int rtc_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

int rtc_days_in_month(int y, int m) {
    static const int d[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (m == 2 && rtc_leap(y)) ? 29 : d[m - 1];
}

int64_t rtc_days_from_civil(int64_t y, int64_t m, int64_t d) {
    y -= (m <= 2);
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    int64_t yoe = y - era * 400;
    int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 306;
}

void rtc_civil_from_days(int64_t z, int *y, int *m, int *d) {
    z += 306;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    int64_t doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int64_t mp = (5 * doy + 2) / 153;
    *d = (int)(doy - (153 * mp + 2) / 5 + 1);
    *m = (int)(mp < 10 ? mp + 3 : mp - 9);
    *y = (int)(yoe + era * 400 + (*m <= 2));
}

int32_t rtc_validate(const DateTime *t) {
    if (t->year < 1 || t->year > 9999) return ERR_INVALID_YEAR;
    if (t->month < 1 || t->month > 12) return ERR_INVALID_MONTH;
    if (t->day < 1 || t->day > rtc_days_in_month(t->year, t->month)) return ERR_INVALID_DAY;
    if (t->hour > 23) return ERR_INVALID_HOUR;
    if (t->minute > 59) return ERR_INVALID_MINUTE;
    if (t->second > 59) return ERR_INVALID_SECOND;
    if (t->microsecond > 999999) return ERR_INVALID_MICROSECOND;
    return 0;
}
