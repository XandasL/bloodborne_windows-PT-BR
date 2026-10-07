// SPDX-License-Identifier: GPL-2.0-or-later
#include "write_log_internal.h"
#include <cstdlib>
#include <cstring>
#include <x86intrin.h>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace BbWriteLog {
std::array<Entry, RingSize> ring;
std::atomic<std::uint64_t> head{0};
std::array<Entry, 256> hits;
std::atomic<std::uint64_t> hits_head{0};

int Mode() {
    static const int mode = [] {
        const char* env = std::getenv("BB_WRITE_LOG");
        return env ? std::atoi(env) : 0;
    }();
    return mode;
}

bool Enabled() { return Mode() == 1; }

void Record(std::uint64_t address, const void* data, std::uint64_t size, Source source) {
#ifdef _WIN32
    static thread_local const std::uint32_t tid = static_cast<std::uint32_t>(GetCurrentThreadId());
#else
    static thread_local const std::uint32_t tid = static_cast<std::uint32_t>(gettid());
#endif
    Entry e{address, size, 0, __rdtsc(), static_cast<std::uint32_t>(source), tid};
    std::memcpy(&e.first, data, size < 8 ? size : 8);
    ring[head.fetch_add(1, std::memory_order_relaxed) % ring.size()] = e;

    if (size > 64) return;
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::uint64_t at = (8 - (address & 7)) & 7; at + 8 <= size; at += 8) {
        std::uint64_t v;
        std::memcpy(&v, bytes + at, 8);
        if (v == Pattern) {
            Entry hit = e;
            hit.address = address + at;
            hit.first = v;
            hits[hits_head.fetch_add(1, std::memory_order_relaxed) % hits.size()] = hit;
        }
    }
}

void NoteIntent(std::uint64_t a, const void* d, std::uint64_t s, Source src) {
    if (Mode() == 2) Record(a, d, s, src);
}

void Note(std::uint64_t a, const void* d, std::uint64_t s, Source src) {
    if (Enabled()) Record(a, d, s, src);
}
} // namespace BbWriteLog
