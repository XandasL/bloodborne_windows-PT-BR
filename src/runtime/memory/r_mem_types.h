/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory Runtime Types and Internal Signatures.
 * Single responsibility: Core type definitions and shared prototypes. (~68 LOC)
 */
#ifndef R_MEM_TYPES_H
#define R_MEM_TYPES_H
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
#include "../../include/platform/sync.h"
#include "../../include/platform/memory.h"
#else
#include "platform/bb_common.h"
#include "platform/sync.h"
#include "platform/memory.h"
#endif

typedef struct BbDirectPool BbDirectPool;

#define PAGE UINT64_C(16384)
#define USER_MIN UINT64_C(0x1000000000)
#define USER_MAX UINT64_C(0xfc00000000)
#define LIMIT 4096
#define INVALID ((int32_t)UINT32_C(0x80020016))
#define NO_MEMORY ((int32_t)UINT32_C(0x8002000c))
#define ACCESS ((int32_t)UINT32_C(0x8002000d))
#define NO_SPACE ((int32_t)UINT32_C(0x80020023))
#define MAP_FIXED_FLAG 0x10
#define MAP_NO_OVERWRITE 0x80
#define FLEXIBLE_SIZE (UINT64_C(448) * 1024 * 1024)
#define FLEX_SPAN (UINT64_C(1024) * 1024 * 1024)

enum { KIND_RESERVED = 1, KIND_DIRECT, KIND_FLEXIBLE };
enum { HOOK_MAP, HOOK_UNMAP, HOOK_INVALIDATE };
typedef struct { uint64_t start, size; int type, used; } Block;
typedef struct { uintptr_t start, end; int kind, prot, type; uint64_t phys; } Vma;
typedef struct {
    uintptr_t start, end; uint64_t offset; int32_t protection, memory_type;
    uint32_t flags; char name[32];
} VirtualQueryInfo;
typedef struct { void *start; uint64_t offset, length; int8_t prot, type; int16_t reserved; int32_t operation; } BatchEntry;
typedef void (*GpuRange)(uintptr_t address, uint64_t size);

void r_mem_write_lock(void);
void r_mem_write_unlock(void);
void r_mem_read_lock(void);
void r_mem_read_unlock(void);
uint64_t r_mem_generation(void);
uint64_t r_mem_pool_size(void);
BbDirectPool* r_mem_pool(void);
uint8_t* r_mem_backing(void);
uint64_t r_mem_flex_alloc(uint64_t size);
void r_mem_flex_free(uint64_t phys, uint64_t size);

size_t r_mem_vma_index(uintptr_t a);
int r_mem_vma_insert(size_t at, Vma v);
void r_mem_vma_erase(size_t at, size_t n);
size_t r_mem_carve(uintptr_t start, uintptr_t end, int *error);
int r_mem_covered(uintptr_t start, uintptr_t end, int allow_reserved);
int r_mem_overlaps(uintptr_t start, uintptr_t end, int ignore_reserved);
uintptr_t r_mem_find_free(uintptr_t hint, uint64_t size, uint64_t alignment);
void r_mem_drop_range(uintptr_t start, uintptr_t end);
int32_t r_mem_place(void **inout, uint64_t size, int prot, int flags,
                    uint64_t alignment, int kind, int type, uint64_t phys);
void r_mem_queue_hook(int kind, uintptr_t address, uint64_t size);
void r_mem_flush_hooks(void);

extern Block g_mem_blocks[LIMIT];
extern Vma *g_mem_vmas;
extern size_t g_mem_vma_count, g_mem_vma_capacity;
extern size_t g_mem_allocations, g_mem_maps, g_mem_flex_maps, g_mem_protects, g_mem_queries;
extern uint64_t g_mem_live_bytes, g_mem_flex_bytes;

#endif /* R_MEM_TYPES_H */
