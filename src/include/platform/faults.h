/* SPDX-License-Identifier: MIT
 * Platform fault handling and exception dispatch.
 * Single responsibility: Exception registration and GPU page hooks. (~45 LOC)
 */
#ifndef BB_PLATFORM_FAULTS_H
#define BB_PLATFORM_FAULTS_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*BbFaultCallback)(void* context, void* fault_address);

void bb_platform_faults_init(void);
void bb_platform_faults_register_gpu(BbFaultCallback cb);

void bb_platform_faults_set_recovery(void* jmp_buf_ptr);
void* bb_platform_faults_get_recovery(void);

void bb_platform_dump_stack_trace(void* context);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_FAULTS_H */
