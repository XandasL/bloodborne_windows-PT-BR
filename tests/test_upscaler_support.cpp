// SPDX-License-Identifier: GPL-2.0-or-later
// Saved FSR 4 choices must be safe before the first rendered frame on older GPUs.
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <unistd.h>
#include "bbport_settings.h"

static void test_matrix(const char* path, BbSettings::State& s) {
    using namespace BbSettings;
    for (bool fsr4 : {false, true}) {
        for (bool fsr411 : {false, true}) {
            for (int requested = 0; requested < UpscalerCount; ++requested) {
                FILE* cfg = std::fopen(path, "w"); assert(cfg);
                std::fprintf(cfg, "upscaler=%s\npreset=3\noutput_res=2560x1440\n", UpscalerName(requested));
                std::fclose(cfg);
                s.fsr4_problem = nullptr; Load();
                assert(s.upscaler == requested);
                ConfigureUpscalerSupport(fsr4, fsr411);
                const bool unsupp = (requested == UpscalerFsr4 && !fsr4) || (requested == UpscalerFsr411 && !(fsr4 && fsr411));
                assert(s.upscaler == (unsupp ? UpscalerFsr3 : requested));
                assert(s.fsr4_supported == fsr4 && s.fsr411_supported == (fsr4 && fsr411));
                assert(bool(s.fsr4_problem.load()) == unsupp && s.preset == Performance && s.output_res == 2);
                assert(s.startup_upscaler == requested);
                ConfigureUpscalerSupport(fsr4, fsr411);
                assert(s.upscaler == (unsupp ? UpscalerFsr3 : requested));
            }
        }
    }
}

int main() {
    using namespace BbSettings;
    char path[] = "/tmp/bbport-upscaler-test-XXXXXX";
    const int fd = mkstemp(path); assert(fd >= 0); close(fd);
    setenv("BB_CONFIG", path, 1);
    unsetenv("BB_UPSCALER"); unsetenv("BB_UPSCALE_PRESET"); unsetenv("BB_RENDER_RES");
    auto& s = Get();
    s.upscaler = UpscalerTaa; s.preset = Performance;
    assert(RenderPreset() == NativeAA && s.preset == Performance);
    Save();
    s.upscaler = UpscalerOff; Load();
    assert(s.upscaler == UpscalerTaa && s.preset == Performance && RenderPreset() == NativeAA && !ResolutionNeedsRestart());
    s.upscaler = UpscalerFsr3; assert(RenderPreset() == Performance);
    test_matrix(path, s);

    s.startup_preset = Quality; s.startup_upscaler = UpscalerFsr3; s.upscaler = UpscalerFsr3; s.preset = Performance;
    assert(RenderPreset() == Performance && !ResolutionNeedsRestart());
    setenv("BB_RENDER_RES", "1706x960", 1);
    assert(RenderPreset() == Quality && ResolutionNeedsRestart());
    s.preset = Quality; assert(!ResolutionNeedsRestart());
    s.upscaler = UpscalerOff; assert(ResolutionNeedsRestart());
    s.upscaler = UpscalerFsr4; assert(!ResolutionNeedsRestart());
    s.upscaler = UpscalerTaa; assert(ResolutionNeedsRestart());
    s.upscaler = UpscalerFsr4;
    const int output = s.startup_output_res;
    s.output_res = (output + 1) % OutputCount;
    assert(ResolutionNeedsRestart());
    s.output_res = output;
    setenv("BB_RENDER_RES", "", 1);
    assert(!FixedRenderSession() && !ResolutionNeedsRestart());
    unsetenv("BB_RENDER_RES"); std::remove(path);
    std::puts("Upscaler support: saved choices and FSR 3.1 fallback PASS");
}
