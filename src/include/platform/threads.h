/* SPDX-License-Identifier: MIT
 * Platform thread management and TLS interface.
 * Single responsibility: Thread lifecycle and guest TLS anchors. (~50 LOC)
 */
#ifndef BB_PLATFORM_THREADS_H
#define BB_PLATFORM_THREADS_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BbThread BbThread;
typedef void* (*BbThreadEntry)(void* arg);

BbThread* bb_platform_thread_create(BbThreadEntry entry, void* arg,
                                    size_t stack_size, const char* name);
int32_t   bb_platform_thread_join(BbThread* thread, void** result);
int32_t   bb_platform_thread_detach(BbThread* thread);
void      bb_platform_thread_set_name(BbThread* thread, const char* name);
void      bb_platform_thread_yield(void);
uint64_t  bb_platform_thread_id(void);

void      bb_platform_tls_set_guest(void* tcb);
void*     bb_platform_tls_get_guest(void);
void*     bb_platform_alloc_low_stack(size_t size);
void      bb_platform_process_restart(void);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_THREADS_H */
