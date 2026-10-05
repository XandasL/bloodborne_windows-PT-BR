/* SPDX-License-Identifier: MIT
 * Platform time, clock, and sleep interface.
 * Single responsibility: Clock abstraction and accurate sleeps. (~35 LOC)
 */
#ifndef BB_PLATFORM_TIME_H
#define BB_PLATFORM_TIME_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t bb_platform_time_ns(void);
uint64_t bb_platform_time_us(void);
uint64_t bb_platform_time_ms(void);

uint64_t bb_platform_tsc_read(void);
uint64_t bb_platform_tsc_frequency(void);

uint64_t bb_platform_time_realtime_us(void);
int64_t  bb_platform_time_tz_offset_us(int64_t epoch_sec);
uint64_t bb_platform_random_u64(void);

void bb_platform_sleep_ns(uint64_t ns);
void bb_platform_sleep_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_TIME_H */
