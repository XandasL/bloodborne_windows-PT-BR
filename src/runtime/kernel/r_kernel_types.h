/* SPDX-License-Identifier: MIT
 * PS4 Kernel Services Types and Internal Signatures.
 * Single responsibility: Kernel service definitions and prototypes. (~65 LOC)
 */
#ifndef R_KERNEL_TYPES_H
#define R_KERNEL_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#if __has_include("../../runtime.h")
#include "../../runtime.h"
#else
#include "runtime.h"
#endif
#if __has_include("../../include/platform/bb_common.h")
#include "../../include/platform/bb_common.h"
#include "../../include/platform/time.h"
#include "../../include/platform/sync.h"
#else
#include "platform/bb_common.h"
#include "platform/time.h"
#include "platform/sync.h"
#endif

#ifndef ABI
#define ABI __attribute__((sysv_abi))
#endif

#define ERR(n) ((int32_t)(UINT32_C(0x80020000) | (n)))
#define PAGE 16384

typedef struct { int64_t sec, nsec; } GuestTimespec;
typedef struct { int64_t sec, usec; } GuestTimeval;
typedef struct { int32_t minuteswest, dsttime; } GuestTimezone;
typedef struct { uint32_t bits[4]; } GuestSigset;
typedef struct { GuestTimeval utime, stime; int64_t rest[14]; } GuestRusage;
typedef void (ABI *KeyDestructor)(void *);

int32_t runtime_guest_errno(int e);
int32_t r_kernel_fail_posix(int e);

uint64_t runtime_process_time_us(void);
uint64_t runtime_process_time_counter(void);
uint64_t runtime_tsc_frequency(void);
void runtime_thread_keys_cleanup(void);

#endif /* R_KERNEL_TYPES_H */
