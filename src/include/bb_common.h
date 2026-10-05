/* SPDX-License-Identifier: MIT
 * Common definitions, macros, and portable type contracts.
 * Single responsibility: Core foundational macros & error codes. (~40 LOC)
 */
#ifndef BB_COMMON_H
#define BB_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_MSC_VER)
#define BB_SYSV_ABI
#define BB_FORCE_INLINE __forceinline
#define BB_NORETURN __declspec(noreturn)
#else
#define BB_SYSV_ABI __attribute__((sysv_abi))
#define BB_FORCE_INLINE __attribute__((always_inline)) inline
#define BB_NORETURN __attribute__((noreturn))
#endif

#define BB_ORBIS_ERROR(code) ((int32_t)(UINT32_C(0x80020000) | (code)))

#define BB_ALIGN_UP(val, align) (((val) + (align) - 1) & ~((align) - 1))
#define BB_ALIGN_DOWN(val, align) ((val) & ~((align) - 1))

#define BB_KB(n) ((size_t)(n) * 1024ULL)
#define BB_MB(n) (BB_KB(n) * 1024ULL)
#define BB_GB(n) (BB_MB(n) * 1024ULL)

#define BB_UNUSED(x) ((void)(x))

#endif /* BB_COMMON_H */
