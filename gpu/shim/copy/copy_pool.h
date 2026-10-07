// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "../bbport_copy.h"

namespace BbCopy {
extern thread_local bool in_copy_thread;

using u64 = unsigned long long;

struct Task {
    std::function<void()> run;
    u64 epoch{};
};

struct Epoch {
    std::size_t pending = 0;
    std::vector<std::function<void()>> callbacks;
};

struct Job {
    const std::function<void(std::size_t)>* task{};
    std::size_t count{};
    std::atomic<std::size_t> next{0};
    std::atomic<std::size_t> done{0};
};

class Pool {
public:
    Pool();
    ~Pool();
    bool Enabled() const { return !threads.empty(); }
    void Async(std::function<void()> task);
    void AfterCopies(std::function<void()> callback);
    void WaitAll();
    void Run(std::size_t count, const std::function<void(std::size_t)>& task);

    void RunAsync(Task& task);
    void Complete(u64 epoch);
    static void Work(Job& job);
    void Loop(unsigned index);

    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    std::shared_ptr<Job> current;
    u64 generation = 0;
    bool stop = false;
    std::deque<Task> async_tasks;
    std::deque<Epoch> epochs;
    u64 epoch_base = 0;
    std::size_t pending = 0;
    bool draining = false;
};

Pool& GetPool();
} // namespace BbCopy
