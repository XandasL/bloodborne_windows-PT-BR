// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#include "shadps4_dlss_state.h"

DlssState g_dlss_state;

void DlssLog(int warning, const char* message) {
    if (g_dlss_state.log && message) {
        g_dlss_state.log(warning, message);
    }
}

bool DlssCheck(const char* operation, NVSDK_NGX_Result result) {
    if (NVSDK_NGX_FAILED(result)) {
        char text[256];
        std::snprintf(text, sizeof(text), "%s failed: 0x%08x", operation,
                      static_cast<unsigned>(result));
        DlssLog(1, text);
        return false;
    }
    return true;
}

void NVSDK_CONV DlssNgxLog(const char* message, NVSDK_NGX_Logging_Level, NVSDK_NGX_Feature) {
    if (message) {
        char text[1024];
        std::snprintf(text, sizeof(text), "NGX: %s", message);
        DlssLog(0, text);
    }
}
