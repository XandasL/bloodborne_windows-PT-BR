/* SPDX-License-Identifier: MIT
 * Common definitions, macros, and portable type contracts.
 * Single responsibility: Core foundational macros & error codes. (~40 LOC)
 */
#ifndef BB_COMMON_H
#define BB_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
#define BB_SYSV_ABI __attribute__((sysv_abi))
#define BB_FORCE_INLINE __attribute__((always_inline)) inline
#define BB_NORETURN __attribute__((noreturn))
#define bb_atomic_cas_ptr(ptr, oldval, newval) __sync_bool_compare_and_swap((ptr), (oldval), (newval))
#elif defined(_MSC_VER)
#if !defined(__clang__)
#error "MSVC cl.exe does not support __attribute__((sysv_abi)) required for direct PS4 AMD64 guest execution. Use Clang (clang/clang-cl) or MinGW GCC on Windows."
#endif
#define BB_SYSV_ABI __attribute__((sysv_abi))
#define BB_FORCE_INLINE __forceinline
#define BB_NORETURN __declspec(noreturn)
#define bb_atomic_cas_ptr(ptr, oldval, newval) (_InterlockedCompareExchangePointer((void* volatile*)(ptr), (void*)(newval), (void*)(oldval)) == (void*)(oldval))
#endif

#define BB_ORBIS_ERROR(code) ((int32_t)(UINT32_C(0x80020000) | (code)))

#define BB_ALIGN_UP(val, align) (((val) + (align) - 1) & ~((align) - 1))
#define BB_ALIGN_DOWN(val, align) ((val) & ~((align) - 1))

#define BB_KB(n) ((size_t)(n) * 1024ULL)
#define BB_MB(n) (BB_KB(n) * 1024ULL)
#define BB_GB(n) (BB_MB(n) * 1024ULL)

#define BB_UNUSED(x) ((void)(x))

#endif /* BB_COMMON_H */
