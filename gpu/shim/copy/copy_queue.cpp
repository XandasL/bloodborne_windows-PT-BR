// SPDX-License-Identifier: GPL-2.0-or-later
#include "copy_pool.h"

namespace BbCopy {

void Pool::Async(std::function<void()> task) {
    {
        std::scoped_lock lk{mutex};
        const u64 epoch = epoch_base + epochs.size() - 1;
        ++epochs.back().pending;
        ++pending;
        async_tasks.push_back({std::move(task), epoch});
    }
    cv.notify_one();
}

void Pool::AfterCopies(std::function<void()> callback) {
    {
        std::unique_lock lk{mutex};
        if (epochs.size() > 1 || epochs.back().pending != 0 || draining) {
            epochs.back().callbacks.push_back(std::move(callback));
            epochs.emplace_back();
            return;
        }
    }
    callback();
}

void Pool::WaitAll() {
    while (true) {
        Task task;
        {
            std::scoped_lock lk{mutex};
            if (pending == 0 && epochs.size() == 1 && !draining) return;
            if (!async_tasks.empty()) {
                task = std::move(async_tasks.front());
                async_tasks.pop_front();
            }
        }
        if (task.run) {
            RunAsync(task);
        } else {
            std::this_thread::yield();
        }
    }
}

void Pool::Run(std::size_t count, const std::function<void(std::size_t)>& task) {
    auto job = std::make_shared<Job>();
    job->task = &task;
    job->count = count;
    {
        std::scoped_lock lk{mutex};
        current = job;
        ++generation;
    }
    cv.notify_all();
    Work(*job);
    while (job->done.load(std::memory_order_acquire) < count) {
        std::this_thread::yield();
    }
    std::scoped_lock lk{mutex};
    if (current == job) current.reset();
}

} // namespace BbCopy
