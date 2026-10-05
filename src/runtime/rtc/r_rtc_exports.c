/* SPDX-License-Identifier: MIT
 * PS4 RTC Exports and NID Resolution.
 * Single responsibility: RTC HLE table and current clock queries. (~70 LOC)
 */
#include "r_rtc_types.h"

ABI int32_t rtc_get_tick(const DateTime *t, uint64_t *tick);
ABI int32_t rtc_set_tick(DateTime *t, const uint64_t *tick);
ABI uint32_t rtc_tick_resolution(void);
ABI int32_t rtc_set_time_t(DateTime *t, int64_t seconds);
ABI int32_t rtc_get_time_t(const DateTime *t, int64_t *seconds);

static ABI int32_t rtc_current_local(DateTime *t) {
    if (!t) return ERR_INVALID_POINTER;
    uint64_t utc = rtc_now_utc();
    rtc_from_tick((uint64_t)((int64_t)utc + rtc_local_offset(utc)), t);
    return 0;
}

static ABI int32_t rtc_current_clock(DateTime *t, int32_t minutes) {
    if (!t) return ERR_INVALID_POINTER;
    rtc_from_tick((uint64_t)((int64_t)rtc_now_utc() + (int64_t)minutes * 60000000LL), t);
    return 0;
}

static ABI int32_t rtc_current_tick(uint64_t *tick) {
    if (!tick) return ERR_INVALID_POINTER;
    *tick = rtc_now_utc();
    return 0;
}

static ABI int32_t rtc_utc_to_local(const uint64_t *utc, uint64_t *local) {
    if (!utc || !local) return ERR_INVALID_POINTER;
    *local = (uint64_t)((int64_t)*utc + rtc_local_offset(*utc));
    return 0;
}

static ABI int32_t rtc_local_to_utc(const uint64_t *local, uint64_t *utc) {
    if (!local || !utc) return ERR_INVALID_POINTER;
    *utc = (uint64_t)((int64_t)*local - rtc_local_offset(*local));
    return 0;
}

static ABI int32_t rtc_rfc2822_local(char *out, const uint64_t *utc_tick) {
    if (!out) return ERR_INVALID_POINTER;
    uint64_t utc = utc_tick ? *utc_tick : rtc_now_utc();
    int64_t offset = rtc_local_offset(utc);
    DateTime t;
    rtc_from_tick((uint64_t)((int64_t)utc + offset), &t);
    static const char *days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    int64_t minutes = offset / 60000000LL;
    char text[64];
    snprintf(text, sizeof(text), "%s, %02u %s %04u %02u:%02u:%02u %c%02d%02d",
             days[rtc_day_of_week(t.year, t.month, t.day)], t.day,
             months[t.month - 1], t.year, t.hour, t.minute, t.second, minutes < 0 ? '-' : '+',
             (int)((minutes < 0 ? -minutes : minutes) / 60), (int)((minutes < 0 ? -minutes : minutes) % 60));
    memcpy(out, text, strlen(text) + 1);
    return 0;
}

static const RuntimeExport exports[] = {
    {"sceRtcGetCurrentClockLocalTime", rtc_current_local}, {"sceRtcGetCurrentClock", rtc_current_clock},
    {"sceRtcGetCurrentTick", rtc_current_tick}, {"sceRtcGetCurrentNetworkTick", rtc_current_tick},
    {"sceRtcConvertUtcToLocalTime", rtc_utc_to_local}, {"sceRtcConvertLocalTimeToUtc", rtc_local_to_utc},
    {"sceRtcGetDayOfWeek", rtc_day_of_week}, {"sceRtcGetTick", rtc_get_tick}, {"sceRtcSetTick", rtc_set_tick},
    {"sceRtcGetTickResolution", rtc_tick_resolution}, {"sceRtcSetTime_t", rtc_set_time_t},
    {"sceRtcGetTime_t", rtc_get_time_t}, {"sceRtcFormatRFC2822LocalTime", rtc_rfc2822_local},
};

uintptr_t runtime_rtc_resolve(const char *name) { return RUNTIME_LOOKUP(exports, name); }
