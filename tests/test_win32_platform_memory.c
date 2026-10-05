/* SPDX-License-Identifier: MIT
 * Windows Platform Direct Memory Aliasing & Section Test.
 */
#include "platform/memory.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("[TEST] Starting Windows Direct Memory Aliasing & Section Test...\n");

    const size_t POOL_SIZE = 16 * 1024 * 1024; // 16 MB
    BbDirectPool* pool = bb_platform_direct_create(POOL_SIZE);
    if (!pool) {
        fprintf(stderr, "FAILED: bb_platform_direct_create returned NULL\n");
        return 1;
    }
    printf("  [PASS] Created 16MB pagefile section pool\n");

    void* backing = bb_platform_direct_backing_base(pool);
    if (!backing) {
        fprintf(stderr, "FAILED: bb_platform_direct_backing_base returned NULL\n");
        return 1;
    }
    printf("  [PASS] Direct backing base mapped at %p\n", backing);

    // Reserve two distinct virtual spans
    void* vaddr_a = VirtualAlloc(NULL, 64 * 1024, MEM_RESERVE, PAGE_NOACCESS);
    void* vaddr_b = VirtualAlloc(NULL, 64 * 1024, MEM_RESERVE, PAGE_NOACCESS);
    assert(vaddr_a != NULL && vaddr_b != NULL);
    assert(vaddr_a != vaddr_b);

    // Alias both virtual spans to offset 0 of the same physical section
    int res_a = bb_platform_direct_map(pool, vaddr_a, 0, 64 * 1024, BB_PROT_READ | BB_PROT_WRITE);
    int res_b = bb_platform_direct_map(pool, vaddr_b, 0, 64 * 1024, BB_PROT_READ | BB_PROT_WRITE);

    if (res_a != 0 || res_b != 0) {
        fprintf(stderr, "FAILED: bb_platform_direct_map failed (res_a=%d, res_b=%d)\n", res_a, res_b);
        return 1;
    }
    printf("  [PASS] Aliased two distinct virtual spans (A: %p, B: %p) to physical pool\n", vaddr_a, vaddr_b);

    // Write magic signature to virtual address A
    volatile uint32_t* ptr_a = (volatile uint32_t*)vaddr_a;
    volatile uint32_t* ptr_b = (volatile uint32_t*)vaddr_b;

    ptr_a[0] = 0xDEADBEEF;
    ptr_a[1] = 0xC001CAFE;

    // Read signature from virtual address B (must match without memory copying)
    if (ptr_b[0] != 0xDEADBEEF || ptr_b[1] != 0xC001CAFE) {
        fprintf(stderr, "FAILED: Aliasing mismatch! ptr_b[0]=0x%X, ptr_b[1]=0x%X\n", ptr_b[0], ptr_b[1]);
        return 1;
    }
    printf("  [PASS] Physical coherency verified: write at Span A reads identically at Span B (0xDEADBEEF, 0xC001CAFE)\n");

    // Test hole punching (commit zeroing)
    bb_platform_direct_punch_hole(pool, 0, 4096);
    if (ptr_a[0] != 0) {
        fprintf(stderr, "FAILED: Hole punching did not zero physical pages (ptr_a[0]=0x%X)\n", ptr_a[0]);
        return 1;
    }
    printf("  [PASS] Hole punch zeroing verified: ptr_a[0] is zeroed\n");

    // Clean up
    UnmapViewOfFile(vaddr_a);
    UnmapViewOfFile(vaddr_b);
    bb_platform_direct_destroy(pool);
    printf("  [PASS] Cleaned up direct pool and views\n");

    printf("[SUCCESS] Windows Direct Memory Aliasing test PASSED!\n");
    return 0;
}
