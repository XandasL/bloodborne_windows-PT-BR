// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../../bbgpu.h"
#include "../sdl_window.h"

extern Frontend::WindowSDL* g_window;

struct Symbol {
    std::string nid, library, module;
    uint64_t address;
};
extern std::vector<Symbol> g_symbols;
extern uint32_t g_sdk_version;

namespace Core::Loader {
class SymbolsResolver;
}

namespace Libraries::Kernel {
void StartKernelService();
void RegisterEventQueue(Core::Loader::SymbolsResolver* sym);
}
namespace Libraries::GnmDriver { void RegisterLib(Core::Loader::SymbolsResolver* sym); }
namespace Libraries::AvPlayer { void RegisterLib(Core::Loader::SymbolsResolver* sym); }
namespace Libraries::VideoOut { void RegisterLib(Core::Loader::SymbolsResolver* sym); }
