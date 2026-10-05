/* SPDX-License-Identifier: MIT
 * PS4 GPU Memory Notification and Backing View Operations.
 * Single responsibility: GPU hook queuing and backing store updates. (~75 LOC)
 */
#include "r_mem_types.h"

uint64_t runtime_disabled_optimizations;
_Thread_local sigjmp_buf *runtime_fault_recover;

static GpuRange hook_map, hook_unmap, hook_invalidate;
typedef struct { int kind; uintptr_t address; uint64_t size; } PendingHook;
static PendingHook pending[64];
static size_t pending_count;

void r_mem_queue_hook(int kind, uintptr_t address, uint64_t size) {
    GpuRange hook = kind == HOOK_MAP ? hook_map : kind == HOOK_UNMAP ? hook_unmap : hook_invalidate;
    if (!hook) return;
    if (pending_count == sizeof(pending) / sizeof(*pending)) {
        fputs("STOP: GPU hook queue overflow\n", stderr); exit(21);
    }
    pending[pending_count++] = (PendingHook){kind, address, size};
}

void r_mem_flush_hooks(void) {
    for (;;) {
        r_mem_write_lock();
        if (!pending_count) { r_mem_write_unlock(); return; }
        PendingHook h = pending[0];
        memmove(pending, pending + 1, --pending_count * sizeof(*pending));
        GpuRange hook = h.kind == HOOK_MAP ? hook_map : h.kind == HOOK_UNMAP ? hook_unmap : hook_invalidate;
        r_mem_write_unlock();
        if (hook) hook(h.address, h.size);
    }
}

void runtime_memory_set_gpu_hooks(GpuRange map, GpuRange unmap, GpuRange invalidate) {
    r_mem_write_lock();
    hook_map = map; hook_unmap = unmap; hook_invalidate = invalidate;
    for (size_t i = 0; i < g_mem_vma_count; ++i)
        if (g_mem_vmas[i].kind != KIND_RESERVED)
            r_mem_queue_hook(HOOK_MAP, g_mem_vmas[i].start, g_mem_vmas[i].end - g_mem_vmas[i].start);
    r_mem_write_unlock();
    r_mem_flush_hooks();
}

int runtime_memory_write_backing(uintptr_t address, const void *data, uint64_t size) {
    uint8_t *backing = r_mem_backing();
    if (!backing) return 0;
    r_mem_read_lock();
    int ok = 1;
    for (uintptr_t at = address, end = address + size; at < end && ok;) {
        size_t i = r_mem_vma_index(at);
        if (i == g_mem_vma_count || g_mem_vmas[i].start > at || g_mem_vmas[i].kind == KIND_RESERVED) {
            ok = 0; break;
        }
        uint64_t n = (g_mem_vmas[i].end < end ? g_mem_vmas[i].end : end) - at;
        memcpy(backing + g_mem_vmas[i].phys + (at - g_mem_vmas[i].start),
               (const uint8_t*)data + (at - address), n);
        at += n;
    }
    r_mem_read_unlock();
    return ok;
}
