// SPDX-License-Identifier: GPL-2.0-or-later
#include "copy_pool.h"
#include "../bbport_toggles.h"

namespace BbCopy {
namespace {
struct Batch {
    std::vector<Item> items;
    unsigned long long bytes = 0;
};
thread_local Batch batch;
} // namespace

bool Enabled() {
    return GetPool().Enabled() && !BbToggle::Disabled(BbToggle::ParallelCopies);
}

void Async(std::function<void()> task) {
    if (!Enabled()) { task(); return; }
    GetPool().Async(std::move(task));
}

void FlushBatch() {
    if (batch.items.empty()) return;
    auto items = std::make_shared<std::vector<Item>>(std::move(batch.items));
    batch.items = {};
    batch.items.reserve(256);
    batch.bytes = 0;
    Async([items] {
        for (const auto& item : *items) item.run(item);
    });
}

void QueueCopy(const Item& item) {
    if (!Enabled()) { item.run(item); return; }
    batch.items.push_back(item);
    batch.bytes += item.size;
    if (batch.bytes >= 512 * 1024 || batch.items.size() >= 256) FlushBatch();
}

void AfterCopies(std::function<void()> callback) {
    FlushBatch();
    GetPool().AfterCopies(std::move(callback));
}

void WaitAsync() {
    if (!batch.items.empty()) {
        for (const auto& item : batch.items) item.run(item);
        batch.items.clear();
        batch.bytes = 0;
    }
    GetPool().WaitAll();
}

void ParallelFor(std::size_t count, const std::function<void(std::size_t)>& task) {
    if (count <= 1 || in_copy_thread || !Enabled()) {
        for (std::size_t i = 0; i < count; ++i) task(i);
        return;
    }
    GetPool().Run(count, task);
}

} // namespace BbCopy
