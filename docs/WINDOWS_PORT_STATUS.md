# Windows Port Status Matrix: `bloodborne_windows`

This document tracks the phased migration and validation of `bbport` subsystems (forked from [deadinside28/bloodborne_pc](https://github.com/deadinside28/bloodborne_pc)) into a modular, decoupled cross-platform architecture.

> **Status:** Architecture ported & compiling; active Windows Bring-Up and runtime verification in progress. See [WINDOWS_BRINGUP.md](WINDOWS_BRINGUP.md) for the active checklist.

---

## 1. Subsystem Migration Matrix

| Subsystem | Linux Status | Windows Status | Target Modules & Tests | Notes |
| :--- | :---: | :---: | :--- | :--- |
| **Build System (CMake)** | ✅ Working via `build.sh` | ✅ Root CMake Configured | [`CMakeLists.txt`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/CMakeLists.txt), [`gpu/CMakeLists.txt`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/gpu/CMakeLists.txt) | Unified multi-platform build with modular source globs. |
| **SDL Window & Surface** | ✅ Working (X11/Wayland) | ✅ Implemented | [`gpu/shim/window.cpp`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/gpu/shim/window.cpp) | `SDL_PROP_WINDOW_WIN32_HWND_POINTER` implemented natively. |
| **Vulkan Renderer Core** | ✅ Working | ✅ Platform Ready | `vk_platform.cpp`, [`src/vulkan_smoke.c`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/vulkan_smoke.c) | Win32 surface hooked via WindowSDL; Vulkan smoke verified. |
| **Platform Time / Clock** | ✅ Working | ✅ Implemented & Tested | [`src/platform/time/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/time/) | Win32 QPC timing, wall clock, tz offset, CSPRNG, and hybrid sleep ($\le 75$ LOC). |
| **Platform Sync (Mutex/Sema)** | ✅ Working | ✅ Implemented & Tested | [`src/platform/sync/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/sync/) | Win32 CriticalSection, SRWLOCK & Semaphores ($\le 75$ LOC). |
| **Platform Filesystem** | ✅ Working | ✅ Implemented & Tested | [`src/platform/fs/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/fs/) | Win32 CreateFileW, FindFirstFileW, SetEndOfFile & stat ($\le 75$ LOC). |
| **Platform Threads / TLS** | ✅ Working | ✅ Implemented & Tested | [`src/platform/threads/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/threads/) | Win32 `_beginthreadex`, low-stack alloc, and process restart ($\le 75$ LOC). |
| **Platform Virtual Memory** | ✅ Working | ✅ Implemented & Tested | [`src/platform/memory/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/memory/) | Win32 VirtualAlloc, Section direct-memory aliasing & hole punching ($\le 75$ LOC). |
| **Fault Handling (VEH)** | ✅ Working | ✅ Implemented & Tested | [`src/platform/faults/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/faults/) | Win32 `AddVectoredExceptionHandler` with GPU page protection recovery ($\le 75$ LOC). |
| **Runtime Virtual Memory** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/memory/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/memory/) | 13 single-responsibility units ($\le 75$ LOC each): VMA, pool, direct, flex, cache, query, protect. |
| **Runtime Threads & Kernel** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/threads/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/threads/), [`src/runtime/kernel/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/kernel/) | 14 single-responsibility units ($\le 75$ LOC each): attrs, tls, lifecycle, keys, time, signals. |
| **Runtime Audio (SDL3)** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/audio/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/audio/) | 6 single-responsibility units ($\le 75$ LOC each): init, open, out, close, graceful fallback. |
| **Runtime AJM (ATRAC9)** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/ajm/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/ajm/) | 11 units ($\le 75$ LOC each): ATRAC9 parser with **RIFF underflow bug patch applied**. |
| **Runtime Save Data** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/savedata/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/savedata/) | 7 units ($\le 75$ LOC each): mount, param, thread-safe search, memory, exports. |
| **Runtime RTC** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/rtc/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/rtc/) | 5 units ($\le 75$ LOC each): civil calculations, ticks, time_t, RFC2822 formatting. |
| **Runtime AppContent** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/content/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/content/) | 4 units ($\le 75$ LOC each): module tracker, content profile parameters, addon list. |
| **Runtime Services** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/services/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/services/) | 10 units ($\le 75$ LOC each): **Zero `#ifndef _WIN32`**; User, Net, Http, NP, Dialogs, IME, Trophies, PlayGo. |
| **Runtime Pad** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/pad/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/pad/) | 5 units ($\le 75$ LOC each): host gamepad sampling, touch coordinates, script playback, rumble. |
| **Runtime Core & Dispatch** | ✅ Working | ✅ Modularized & Tested | [`src/runtime/core/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/core/) | 6 units ($\le 75$ LOC each): dynamic module TLS, exit handlers, static guards, NID dispatch. |
| **Loader Subsystem** | ✅ Working | ✅ Modularized & Tested | [`src/loader/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/loader/) | 9 units ($\le 75$ LOC each): BBPROBE parser, segments, relocations, BBPATCH2, SFO, CLI main. |

---

## 2. Architectural Invariants Enforced

1. **Strict Line Budget ($\le 75$ LOC per file):** Every C source and header file across `src/platform/`, `src/runtime/`, and `src/loader/` strictly adheres to $\le 75$ lines of code (verified via automated codebase line scanning).
2. **Zero Platform `#ifdef` in HLE:** `src/runtime/` and `src/loader/` call only `bb_platform_*` functions. All OS headers (`<windows.h>`, `<sys/mman.h>`, `<pthread.h>`, `<sys/syscall.h>`, etc.) reside strictly within `src/platform/<subsystem>/<platform>/`.
3. **Physical Memory Aliasing on Windows:** Implemented via Win32 named section page file mapping (`CreateFileMappingA` / `MapViewOfFileEx`) with section punch-hole zeroing (`VirtualAlloc(MEM_COMMIT)`).
4. **Fault Isolation & Graceful Degradation:** Optional subsystems (audio devices, gamepads, window input) fall back cleanly without terminating execution.
5. **Security Patches Maintained:** Fixed ATRAC9 RIFF buffer 18-exabyte unsigned underflow bug in [`r_ajm_atrac9.c`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/ajm/r_ajm_atrac9.c).
