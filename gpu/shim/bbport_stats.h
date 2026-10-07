// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>

namespace BbStats {
inline std::atomic<std::uint64_t> tracker_faults{0};
inline std::atomic<std::int64_t> hot_pages{0};
inline std::atomic<std::uint64_t> images_registered{0};
inline std::atomic<std::uint64_t> image_upload_bytes{0};
inline std::atomic<std::uint64_t> buffer_upload_bytes{0};
inline std::atomic<int> gpu_thread_clock{-1};
inline std::atomic<std::uint64_t> draws{0}, dispatches{0}, submissions{0};
inline std::atomic<std::uint64_t> gpu_frames{0};
inline std::atomic<std::uint64_t> t_resident{0}, t_protect{0}, t_image_create{0}, t_refresh{0},
    t_staging{0}, t_host_wait{0}, t_copy{0}, copy_bytes{0}, t_read_faults{0}, read_faults{0},
    t_write_faults{0}, t_copy_cpu{0}, copy_sys_us{0}, copy_minflt{0};

inline const bool enabled = [] {
    const char* env = std::getenv("BB_FRAME_STATS");
    return env && env[0] == '1';
}();

struct Timer {
    std::atomic<std::uint64_t>& total;
    std::chrono::steady_clock::time_point start =
        enabled ? std::chrono::steady_clock::now() : std::chrono::steady_clock::time_point{};
    ~Timer() {
        if (!enabled) return;
        total.fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count(), std::memory_order_relaxed);
    }
};

inline std::atomic<std::uint64_t> gpu_idle_ns{0};
inline std::atomic<std::uint64_t> reduced_draws{0}, scene_draws{0};
inline std::atomic<std::uint64_t> sync_recording_ns{0}, host_copies_wait_ns{0}, tick_wait_ns{0},
    copy_threads_wait_ns{0}, host_copy_waits{0};

struct WaitTimer {
    std::atomic<std::uint64_t>& total;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    ~WaitTimer() {
        total.fetch_add(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now() - start).count(), std::memory_order_relaxed);
    }
};

inline std::atomic<std::uint64_t> gpu_sys_us{0}, gpu_user_us{0}, gpu_invol_switches{0},
    gpu_vol_switches{0}, gpu_minor_faults{0};
inline std::atomic<std::uint64_t> gpu_signal_faults{0};
inline std::atomic<std::uint64_t> protect_calls{0}, protect_pages{0}, protect_revoke_calls{0},
    protect_revoke_pages{0};
} // namespace BbStats
