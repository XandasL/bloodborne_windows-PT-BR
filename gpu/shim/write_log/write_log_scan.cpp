// SPDX-License-Identifier: GPL-2.0-or-later
#include "write_log_internal.h"
#include "write_log_scan.h"
#include <cstdio>
#include <cstring>
#include <x86intrin.h>

namespace BbWriteLog {
namespace {
const char* sources[] = {"backing", "WriteData", "fence", "EOP (decoded)", "WriteData (decoded)", "EOS (decoded)"};

void PrintEntry(const Entry& e, const char* what, std::uint64_t now) {
    std::fprintf(stderr, "Write log: %s %s %#llx +%llu first %#llx tid %u, %.3f s before\n",
                 what, e.source < 6 ? sources[e.source] : "?",
                 (unsigned long long)e.address, (unsigned long long)e.size,
                 (unsigned long long)e.first, e.tid, double(now - e.tsc) / 3.0e9);
}
} // namespace

void DumpPatternHits() {
    const std::uint64_t now = __rdtsc();
    const std::uint64_t nh = hits_head.load();
    for (std::uint64_t i = nh > hits.size() ? nh - hits.size() : 0; i < nh; ++i) {
        PrintEntry(hits[i % hits.size()], "pattern", now);
    }
}

void DumpBlockWrites(std::uint64_t block) {
    for (std::uint64_t at = block - 0x30; at < block + 0x60; at += 8) {
        std::uint64_t value = 0;
        std::memcpy(&value, reinterpret_cast<const void*>(at), 8);
        std::fprintf(stderr, "Write log: [%#llx] = %#llx\n",
                     (unsigned long long)at, (unsigned long long)value);
    }
}

void DumpMatchingWrites(std::uint64_t block, const std::uint64_t regs[8]) {
    const std::uint64_t now = __rdtsc();
    const std::uint64_t n = head.load();
    int shown = 0;
    for (std::uint64_t i = n; i-- > (n > RingSize ? n - RingSize : 0) && shown < 200;) {
        const Entry& e = ring[i % RingSize];
        if (e.address + e.size > block - 0x30 && e.address < block + 0x60) {
            PrintEntry(e, "in block", now);
            ++shown;
        }
    }
    shown = 0;
    for (std::uint64_t i = n; i-- > (n > RingSize ? n - RingSize : 0) && shown < 64;) {
        const Entry& e = ring[i % RingSize];
        bool nearby = false;
        for (int r = 0; r < 8; ++r) {
            nearby |= regs[r] + 0x1000 > e.address && regs[r] < e.address + e.size + 0x1000;
        }
        if (nearby) {
            PrintEntry(e, "near", now);
            ++shown;
        }
    }
    std::fprintf(stderr, "Write log: %llu writes logged\n", (unsigned long long)n);
}
} // namespace BbWriteLog
