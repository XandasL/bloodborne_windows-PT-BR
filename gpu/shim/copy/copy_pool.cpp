// SPDX-License-Identifier: GPL-2.0-or-later
#include "copy_pool.h"
#include "../bbport_threads.h"
#include "common/thread.h"
#include <algorithm>
#include <cstdlib>

namespace BbCopy {
thread_local bool in_copy_thread = false;

Pool::Pool() {
    unsigned count = std::clamp(BbThreads::Available() / 4, 1u, 4u);
    if (const char* env = std::getenv("BB_COPY_THREADS")) {
        count = static_cast<unsigned>(std::clamp(std::atoi(env), 0, 16));
    }
    epochs.emplace_back();
    for (unsigned i = 0; i < count; ++i) {
        threads.emplace_back([this, i] { Loop(i); });
    }
}

Pool::~Pool() {
    {
        std::scoped_lock lk{mutex};
        stop = true;
    }
    cv.notify_all();
    for (auto& thread : threads) thread.join();
}

void Pool::Loop(unsigned index) {
    Common::SetCurrentThreadName(("bb:Copy" + std::to_string(index)).c_str());
    u64 seen = 0;
    std::unique_lock lk{mutex};
    while (true) {
        cv.wait(lk, [&] {
            return stop || !async_tasks.empty() || (generation != seen && current);
        });
        if (stop) return;
        if (!async_tasks.empty()) {
            Task task = std::move(async_tasks.front());
            async_tasks.pop_front();
            lk.unlock();
            RunAsync(task);
            lk.lock();
            continue;
        }
        seen = generation;
        const auto job = current;
        lk.unlock();
        Work(*job);
        lk.lock();
    }
}

Pool& GetPool() {
    static Pool pool;
    return pool;
}
} // namespace BbCopy
