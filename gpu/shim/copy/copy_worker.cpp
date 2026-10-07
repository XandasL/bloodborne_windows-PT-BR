// SPDX-License-Identifier: GPL-2.0-or-later
#include "copy_pool.h"

namespace BbCopy {

void Pool::RunAsync(Task& task) {
    const bool was = in_copy_thread;
    in_copy_thread = true;
    task.run();
    in_copy_thread = was;
    Complete(task.epoch);
}

void Pool::Complete(u64 epoch) {
    std::unique_lock lk{mutex};
    --epochs[epoch - epoch_base].pending;
    --pending;
    if (draining) return;
    draining = true;
    while (true) {
        std::vector<std::function<void()>> ready;
        while (epochs.size() > 1 && epochs.front().pending == 0) {
            for (auto& callback : epochs.front().callbacks) {
                ready.push_back(std::move(callback));
            }
            epochs.pop_front();
            ++epoch_base;
        }
        if (ready.empty()) break;
        lk.unlock();
        for (auto& callback : ready) callback();
        lk.lock();
    }
    draining = false;
}

void Pool::Work(Job& job) {
    const bool was = in_copy_thread;
    in_copy_thread = true;
    for (std::size_t i; (i = job.next.fetch_add(1, std::memory_order_relaxed)) < job.count;) {
        (*job.task)(i);
        job.done.fetch_add(1, std::memory_order_release);
    }
    in_copy_thread = was;
}

} // namespace BbCopy
