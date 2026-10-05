/* SPDX-License-Identifier: MIT
 * PS4 Kernel Clocks and Sleeping Services.
 * Single responsibility: Time queries, TSC, and sleeping using platform time. (~70 LOC)
 */
#include "r_kernel_types.h"

static uint64_t s_start_ns;
static void init_time(void) { if (!s_start_ns) s_start_ns = bb_platform_time_ns(); }

ABI uint64_t process_time(void) { init_time(); return (bb_platform_time_ns() - s_start_ns) / 1000; }
ABI uint64_t process_time_counter(void) { init_time(); return bb_platform_time_ns() - s_start_ns; }
ABI uint64_t process_time_frequency(void) { return 1000000000; }
ABI uint64_t read_tsc(void) { return bb_platform_tsc_read(); }
ABI uint64_t tsc_frequency(void) { return bb_platform_tsc_frequency(); }
uint64_t runtime_process_time_us(void) { return process_time(); }
uint64_t runtime_process_time_counter(void) { return process_time_counter(); }
uint64_t runtime_tsc_frequency(void) { return tsc_frequency(); }

ABI int32_t kernel_clock_gettime(uint32_t id, GuestTimespec *ts) {
    (void)id; if (!ts) return ERR(22);
    uint64_t ns = bb_platform_time_ns();
    ts->sec = (int64_t)(ns / 1000000000ULL); ts->nsec = (int64_t)(ns % 1000000000ULL);
    return 0;
}
ABI int32_t posix_clock_gettime(uint32_t id, GuestTimespec *ts) { return kernel_clock_gettime(id, ts) ? r_kernel_fail_posix(22) : 0; }
ABI int32_t posix_clock_getres(uint32_t id, GuestTimespec *ts) { (void)id; if (ts) { ts->sec = 0; ts->nsec = 1000; } return 0; }

ABI int32_t kernel_usleep(uint32_t usec) { bb_platform_sleep_ns((uint64_t)usec * 1000); return 0; }
ABI int32_t posix_usleep(uint32_t usec) { bb_platform_sleep_ns((uint64_t)usec * 1000); return 0; }
ABI uint32_t posix_sleep(uint32_t sec) { bb_platform_sleep_ns((uint64_t)sec * 1000000000ULL); return 0; }
ABI int32_t posix_nanosleep(const GuestTimespec *rq, GuestTimespec *rem) {
    if (!rq || rq->nsec < 0 || rq->nsec >= 1000000000 || rq->sec < 0) return r_kernel_fail_posix(22);
    bb_platform_sleep_ns((uint64_t)rq->sec * 1000000000ULL + (uint64_t)rq->nsec);
    if (rem) { rem->sec = 0; rem->nsec = 0; }
    return 0;
}
ABI int32_t kernel_nanosleep(const GuestTimespec *rq, GuestTimespec *rem) { return posix_nanosleep(rq, rem) ? ERR(*runtime_errno()) : 0; }

ABI int32_t kernel_gettimezone(GuestTimezone *tz) { if (!tz) return ERR(22); tz->minuteswest = 0; tz->dsttime = 0; return 0; }
ABI int32_t posix_gettimeofday(GuestTimeval *tv, GuestTimezone *tz) {
    if (tv) {
        uint64_t ns = bb_platform_time_ns();
        tv->sec = (int64_t)(ns / 1000000000ULL); tv->usec = (int64_t)((ns % 1000000000ULL) / 1000ULL);
    }
    if (tz) kernel_gettimezone(tz);
    return 0;
}
ABI int32_t kernel_gettimeofday(GuestTimeval *tv) { return tv ? posix_gettimeofday(tv, NULL) : ERR(22); }
ABI int64_t posix_time(int64_t *out) {
    int64_t t = (int64_t)(bb_platform_time_ns() / 1000000000ULL);
    if (out) *out = t;
    return t;
}
