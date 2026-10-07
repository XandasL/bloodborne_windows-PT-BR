// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cassert>
#include <span>
#include "gpu/shadps4/video_core/renderer_vulkan/motion_history.h"

using namespace Vulkan::Motion;

inline void TestRanges() {
    const std::array<uint16_t, 5> indices{1000, 1007, 1002, 1007, UINT16_MAX};
    const auto range = IndexedRange(std::span(indices), -900, true);
    assert(range.first == 100 && range.count == 8);
    assert(IndexedRange(std::span(indices), -1001, true).count == 0);
    assert(IndexedRange(std::span(indices), 0, false).count == 64536);
    const std::array<uint32_t, 3> large{100000, 100003, UINT32_MAX};
    assert(IndexedRange(std::span(large), 7, true).count == 4);
    assert(IndexedRange(std::span(large), INT32_MAX, true).count == 0);
    const std::array<uint16_t, 1> restart{UINT16_MAX};
    assert(IndexedRange(std::span(restart), 0, true).count == 0);
    assert(IndexedRange(std::span<const uint16_t>{}, 0, false).count == 0);
}

inline void TestGatingAndJitter() {
    History gated(1024);
    const History::GateKey weapon{.shader = 7, .index_count = 3, .instances = 1};
    const auto frame = [&](uint64_t pal) { gated.NextFrame(); return gated.Moving(weapon, pal); };
    assert(!gated.Moving(weapon, 1) && gated.stats.still == 1);
    assert(!frame(1) && frame(2) && frame(3) && !frame(3));
    gated.NextFrame(); gated.NextFrame();
    assert(!gated.Moving(weapon, 4) && !gated.Moving(weapon, 5));
    assert(ClassifyBuffer(864) == BufferRole::Other && ClassifyBuffer(416) == BufferRole::Other);
    assert(ClassifyBuffer(96) == BufferRole::SmallSkeleton && ClassifyBuffer(384) == BufferRole::SmallSkeleton);
    assert(ClassifyBuffer(656) == BufferRole::Skeleton && ClassifyBuffer(64) == BufferRole::Other);

    IndexRangeCache ranges; uint32_t scans = 0;
    const IndexRangeCache::Key mesh{.address = 0x1000, .count = 3, .index_size = 2};
    const auto scan = [&] { ++scans; return IndexRangeCache::Result{{4, 3}, 99}; };
    assert(ranges.Get(mesh, 0, scan).topology == 99 && scans == 1);
    assert(ranges.Get(mesh, IndexRangeCache::Revalidate - 1, scan).range == (VertexRange{4, 3}) && scans == 1);
    ranges.Get(mesh, IndexRangeCache::Revalidate, scan); assert(scans == 2);
    ranges.Trim(IndexRangeCache::Revalidate + IndexRangeCache::Unused + 1); assert(ranges.Size() == 0);

    assert(JitterPhases(1920, 1920) == 8 && JitterPhases(1280, 1920) == 18);
    assert(JitterPhases(960, 1920) == 32 && JitterPhases(640, 1920) == 72 && JitterPhases(0, 1920) == 8);
}
