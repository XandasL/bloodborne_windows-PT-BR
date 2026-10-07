// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <string>
#include "../bbport_settings.h"

namespace BbSettings {
const char* ConfigPath();
void SetConfigValue(Values& v, const std::string& key, const std::string& value);
} // namespace BbSettings
