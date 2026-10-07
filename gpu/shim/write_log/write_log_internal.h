// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include "../bbport_write_log.h"

namespace BbWriteLog {
struct Entry {
    std::uint64_t address, size, first, tsc;
    std::uint32_t source, tid;
};

constexpr std::size_t RingSize = 1 << 20;
extern std::array<Entry, RingSize> ring;
extern std::atomic<std::uint64_t> head;

extern std::array<Entry, 256> hits;
extern std::atomic<std::uint64_t> hits_head;
constexpr std::uint64_t Pattern = 0x0000005300000000ull;

int Mode();
} // namespace BbWriteLog
