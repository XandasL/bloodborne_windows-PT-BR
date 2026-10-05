/* SPDX-License-Identifier: MIT
 * Platform virtual memory and direct pool aliasing interface.
 * Single responsibility: 40-bit address allocation & aliased mapping. (~55 LOC)
 */
#ifndef BB_PLATFORM_MEMORY_H
#define BB_PLATFORM_MEMORY_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum BbProtFlags {
    BB_PROT_NONE  = 0,
    BB_PROT_READ  = 1,
    BB_PROT_WRITE = 2,
    BB_PROT_EXEC  = 4
};

typedef struct BbDirectPool BbDirectPool;

void* bb_platform_vm_reserve(uintptr_t preferred_base, size_t size);
int   bb_platform_vm_commit(void* addr, size_t size, int prot);
int   bb_platform_vm_decommit(void* addr, size_t size);
int   bb_platform_vm_protect(void* addr, size_t size, int prot);
int   bb_platform_vm_release(void* addr, size_t size);

BbDirectPool* bb_platform_direct_create(size_t size);
void* bb_platform_direct_backing_base(BbDirectPool* pool);
int   bb_platform_direct_map(BbDirectPool* pool, void* vaddr,
                             uint64_t phys_offset, size_t size, int prot);
int   bb_platform_direct_unmap(void* vaddr, size_t size);
void  bb_platform_direct_punch_hole(BbDirectPool* pool, uint64_t offset, size_t size);
void  bb_platform_direct_destroy(BbDirectPool* pool);

size_t bb_platform_page_size(void);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_MEMORY_H */
