// SPDX-License-Identifier: GPL-2.0-or-later
#include "core/memory.h"
#include "video_core/renderer_vulkan/vk_rasterizer.h"
#include "../bbport_copy.h"
#include "../bbport_toggles.h"
#include "../bbport_write_log.h"
#include <algorithm>
#include <cstring>

extern "C" {
int runtime_memory_is_mapped(uintptr_t address, uint64_t size);
int runtime_memory_write_backing(uintptr_t address, const void* data, uint64_t size);
uint64_t runtime_memory_clamp(uintptr_t address, uint64_t size);
int runtime_memory_region(uintptr_t address, uintptr_t* start, uintptr_t* end, int* mapped);
void runtime_memory_gpu_protect(uintptr_t address, uint64_t size, int read, int write);
typedef void (*RuntimeGpuRange)(uintptr_t address, uint64_t size);
void runtime_memory_set_gpu_hooks(RuntimeGpuRange map, RuntimeGpuRange unmap, RuntimeGpuRange invalidate);
}

namespace Core {
void MemoryManager::SetRasterizer(Vulkan::Rasterizer* rasterizer_) {
    rasterizer = rasterizer_;
    runtime_memory_set_gpu_hooks(
        [](uintptr_t a, uint64_t s) {
            auto* r = Memory::Instance()->GetRasterizer();
            r->RegisterMemory(a, s); r->MapMemory(a, s);
        },
        [](uintptr_t a, uint64_t s) { Memory::Instance()->GetRasterizer()->UnmapMemory(a, s); },
        [](uintptr_t a, uint64_t s) { Memory::Instance()->GetRasterizer()->InvalidateMemory(a, s); });
}

void MemoryManager::InvalidateMemory(VAddr address, u64 size) {
    if (rasterizer) rasterizer->InvalidateMemory(address, size);
}

u64 MemoryManager::ClampRangeSize(VAddr virtual_addr, u64 size) {
    return runtime_memory_clamp(virtual_addr, size);
}

static void CopySparseSerial(VAddr source, u8* dest, u64 size) {
    while (size) {
        uintptr_t start = 0, end = 0; int mapped = 0;
        if (!runtime_memory_region(source, &start, &end, &mapped)) end = source + size;
        const u64 n = std::min<u64>(size, end - source);
        if (mapped) std::memcpy(dest, reinterpret_cast<const void*>(source), n);
        else std::memset(dest, 0, n);
        source += n; dest += n; size -= n;
    }
}

void MemoryManager::CopySparseMemory(VAddr source, u8* dest, u64 size) {
    constexpr u64 Chunk = 512 * 1024;
    if (size < 4 * Chunk || !BbCopy::Enabled()) return CopySparseSerial(source, dest, size);
    BbCopy::ParallelFor((size + Chunk - 1) / Chunk, [&](std::size_t i) {
        const u64 offset = i * Chunk;
        CopySparseSerial(source + offset, dest + offset, std::min(Chunk, size - offset));
    });
}

bool MemoryManager::TryWriteBacking(void* address, const void* data, u64 size) {
    BbWriteLog::Note(reinterpret_cast<uintptr_t>(address), data, size, BbWriteLog::Backing);
    return runtime_memory_write_backing(reinterpret_cast<uintptr_t>(address), data, size) != 0;
}

void AddressSpace::Protect(VAddr virtual_addr, u64 size, MemoryPermission perms) {
    BbStats::Timer timer{BbStats::t_protect};
    runtime_memory_gpu_protect(virtual_addr, size, True(perms & MemoryPermission::Read), True(perms & MemoryPermission::Write));
}

boost::icl::interval_set<VAddr> AddressSpace::GetUsableRegions() {
    boost::icl::interval_set<VAddr> set;
    set += boost::icl::interval<VAddr>::right_open(0x10'0000'0000ULL, 0xfc'0000'0000ULL);
    return set;
}
} // namespace Core
