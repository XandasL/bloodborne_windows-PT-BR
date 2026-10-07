// SPDX-License-Identifier: GPL-2.0-or-later
#include "bbgpu_internal.h"
#include "../bbport_settings.h"
#include "common/elf_info.h"
#include "common/logging/log.h"
#include "common/thread.h"
#include "core/libraries/libs.h"
#include "video_core/renderdoc.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <SDL3/SDL.h>

namespace Core {
class Emulator {
public:
    static void FillElfInfo(const BbGpuConfig& cfg) {
        auto& i = Common::ElfInfo::Instance();
        i.initialized = true;
        i.game_serial = cfg.serial ? cfg.serial : "UNKNOWN";
        i.title = cfg.title ? cfg.title : "";
        i.sdk_ver = cfg.sdk_version;
        i.psf_attributes.raw = cfg.psf_attributes;
    }
};
} // namespace Core

namespace VideoCore {
void LoadRenderDoc() {} void StartCapture() {} void EndCapture() {} void TriggerCapture() {}
void SetOutputDir(const std::filesystem::path&, const std::string&) {}
bool IsRenderDocLoaded() { return false; } void RequestScreenshot(ScreenshotRequest) {}
u32 ConsumeGameOnlyScreenshotRequests() { return 0; } u32 ConsumeWithOverlaysScreenshotRequests() { return 0; }
ScreenshotRequests ConsumeScreenshotRequests() { return {}; }
} // namespace VideoCore

static std::thread g_window_thread;
static std::mutex g_window_mutex;
static std::condition_variable g_window_cv;
static bool g_window_ready;

extern "C" int bbgpu_init(const BbGpuConfig* config) {
    BbSettings::Load();
    g_sdk_version = config->sdk_version;
#ifdef _WIN32
    if (config->user_dir && !std::getenv("BB_GPU_USER_DIR")) _putenv_s("BB_GPU_USER_DIR", config->user_dir);
#else
    if (config->user_dir) setenv("BB_GPU_USER_DIR", config->user_dir, 0);
#endif
    Core::Emulator::FillElfInfo(*config);
    const std::string title = config->title ? config->title : "Bloodborne";
    const s32 w = config->width, h = config->height;
    g_window_thread = std::thread([title, w, h] {
        Common::SetCurrentThreadName("bb:window");
        auto* win = new Frontend::WindowSDL(w, h, title.c_str());
        { std::scoped_lock lk{g_window_mutex}; g_window = win; g_window_ready = true; }
        g_window_cv.notify_all();
        while (win->PollEvents()) SDL_Delay(2);
        std::fflush(stdout);
        std::_Exit(0);
    });
    g_window_thread.detach();
    { std::unique_lock lk{g_window_mutex}; g_window_cv.wait(lk, [] { return g_window_ready; }); }
    Core::Loader::SymbolsResolver r;
    Libraries::GnmDriver::RegisterLib(&r);
    Libraries::VideoOut::RegisterLib(&r);
    return 0;
}

extern "C" void bbgpu_register_kernel(void) {
    Libraries::Kernel::StartKernelService();
    Core::Loader::SymbolsResolver r;
    Libraries::Kernel::RegisterEventQueue(&r);
    Libraries::AvPlayer::RegisterLib(&r);
}
