// SPDX-License-Identifier: GPL-2.0-or-later
#include <cstdio>
#include "test_motion_history_helpers.h"

int main() {
    TestRanges();
    const std::array<uint16_t, 5> indices{1000, 1007, 1002, 1007, UINT16_MAX};
    const auto range = IndexedRange(std::span(indices), -900, true);
    Draw draw{.shader = 1, .geometry = 2, .indices = 3, .topology = 4,
              .index_count = 5, .instances = 2, .first_instance = 100, .vertices = range};
    History history(32); history.NextFrame();
    const auto first = history.Prepare(draw);
    const auto duplicate = history.Prepare(draw);
    assert(first.store && !first.load && first.first_vertex == 100);
    assert(first.first_instance == 100 && first.instances == 2);
    assert(duplicate.store == first.store + 16 && history.Used() == 32);
    assert(!history.Prepare(draw).store && history.stats.exhausted == 1);
    history.NextFrame();
    assert(history.Prepare(draw).load == first.store);
    assert(history.Prepare(draw).load == duplicate.store);
    history.NextFrame(); history.NextFrame();
    assert(!history.Prepare(draw).load);

    history.NextFrame();
    auto changed = draw; changed.topology++; assert(!history.Prepare(changed).load);
    changed = draw; changed.first_instance++; assert(!history.Prepare(changed).load);
    history.NextFrame();
    changed = draw; changed.vertices.first++; assert(!history.Prepare(changed).load);

    History overflow(32);
    draw.vertices.first = 0; draw.vertices.count = INT32_MAX;
    draw.instances = UINT32_MAX; draw.first_instance = 0;
    assert(!overflow.Prepare(draw).store && overflow.stats.exhausted == 1);
    draw.first_instance = 10;
    assert(!overflow.Prepare(draw).store && overflow.stats.invalid == 1);
    draw.first_instance = 0; draw.vertices.first = INT32_MAX; draw.vertices.count = 2;
    assert(!overflow.Prepare(draw).store && overflow.stats.invalid == 2);

    TestGatingAndJitter();
    std::puts("Motion history: PASS (ranges, restart, offsets, matching, capacity, gating, index cache, jitter)");
}
