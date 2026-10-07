// SPDX-License-Identifier: GPL-2.0-or-later
#include "../bbport_settings.h"
#include <algorithm>

namespace BbSettings {

float PresetScale(int preset) {
    static constexpr float scales[PresetCount] = {1.0f, 1.5f, 1.7f, 2.0f, 3.0f};
    return scales[std::clamp(preset, 0, PresetCount - 1)];
}

const char* PresetName(int preset) {
    static constexpr const char* names[PresetCount] = {
        "Native AA", "Quality", "Balanced", "Performance", "Ultra Performance"
    };
    return names[std::clamp(preset, 0, PresetCount - 1)];
}

const char* UpscalerName(int upscaler) {
    static constexpr const char* names[UpscalerCount] = {
        "off", "fsr3", "fsr4", "fsr411", "taa"
    };
    return names[std::clamp(upscaler, 0, UpscalerCount - 1)];
}

} // namespace BbSettings
