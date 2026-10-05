/* SPDX-License-Identifier: MIT
 * PS4 RTC types and constants.
 * Single responsibility: DateTime layout and error constants. (~45 LOC)
 */
#ifndef R_RTC_TYPES_H
#define R_RTC_TYPES_H

#include "runtime.h"
#include "platform/time.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define ERR_INVALID_POINTER ((int32_t)0x80B50002)
#define ERR_INVALID_VALUE   ((int32_t)0x80B50003)
#define ERR_INVALID_YEAR    ((int32_t)0x80B50008)
#define ERR_INVALID_MONTH   ((int32_t)0x80B50009)
#define ERR_INVALID_DAY     ((int32_t)0x80B5000A)
#define ERR_INVALID_HOUR    ((int32_t)0x80B5000B)
#define ERR_INVALID_MINUTE  ((int32_t)0x80B5000C)
#define ERR_INVALID_SECOND  ((int32_t)0x80B5000D)
#define ERR_INVALID_MICROSECOND ((int32_t)0x80B5000E)

#define UNIX_EPOCH_TICKS UINT64_C(0xdcbffeff2bc000)
#define TICKS_PER_DAY UINT64_C(86400000000)

typedef struct { uint16_t year, month, day, hour, minute, second; uint32_t microsecond; } DateTime;
_Static_assert(sizeof(DateTime) == 16, "OrbisRtcDateTime layout");

int rtc_leap(int y);
int rtc_days_in_month(int y, int m);
int64_t rtc_days_from_civil(int64_t y, int64_t m, int64_t d);
void rtc_civil_from_days(int64_t z, int *y, int *m, int *d);
int32_t rtc_validate(const DateTime *t);

uint64_t rtc_to_tick(const DateTime *t);
void rtc_from_tick(uint64_t tick, DateTime *t);
uint64_t rtc_now_utc(void);
int64_t rtc_local_offset(uint64_t utc);

int32_t rtc_day_of_week(int32_t year, int32_t month, int32_t day);

#endif /* R_RTC_TYPES_H */
