# bloodborne_windows — Native Windows & Linux Port of Bloodborne

**English** · [Русский](README.ru.md)

`bloodborne_windows` is a native x86-64 runner and high-performance HLE runtime for **Bloodborne** (PlayStation 4, CUSA03173, game version 1.09) running directly on **Windows** and **Linux**.

> [!IMPORTANT]
> **Fork Origin & Upstream Attribution:**
> This repository is a fork of the pioneering Linux runner [**deadinside28/bloodborne_pc**](https://github.com/deadinside28/bloodborne_pc) by deadinside28.
> The original repository laid the groundwork for native PS4 eboot execution on Linux. This fork re-architects the entire codebase into a clean, decoupled cross-platform system with a strict single-responsibility modular structure, removes Linux-exclusive coupling, implements native Windows kernel and memory primitives, and guarantees long-term maintainability.

> [!NOTE]
> **No copyrighted game assets or binaries are included.** You must provide your own legally dumped, plaintext copy of *Bloodborne* (PS4 CUSA03173 v1.09). This project is not affiliated with Sony Interactive Entertainment, FromSoftware, or AMD.

---

## 🌟 Highlights & Architecture

- **Native Execution (Zero CPU Emulation):** The PS4 ELF image is parsed and mapped directly into host memory below the 40-bit (< 1 TiB) address boundary required by PS4 GPU descriptors. Game x86-64 machine code executes directly on your processor at native speed using SysV AMD64 ABI transitions.
- **Strict Single-Responsibility Codebase ($\le 75$ LOC per file):** Every C source and header file across `src/platform/`, `src/runtime/`, and `src/loader/` adheres strictly to a $\le 75$ lines of code budget.
- **Zero Platform `#ifdef` in HLE Runtime:** All business logic in `src/runtime/` and `src/loader/` communicates with the OS exclusively through `bb_platform_*` interfaces. Operating system headers (`<windows.h>`, `<sys/mman.h>`, `<pthread.h>`, etc.) reside strictly within isolated platform translation units.
- **Windows Physical Direct-Memory Section Aliasing:** Implemented via Win32 page-file section mappings (`CreateFileMappingA`, `MapViewOfFileEx`), allowing arbitrary virtual address spans to alias the same physical pool. Hole punching is performed via commit zeroing (`VirtualAlloc(MEM_COMMIT)`).
- **Vectored Exception Handling (VEH):** Uses `AddVectoredExceptionHandler` on Windows to intercept page access violations (`STATUS_ACCESS_VIOLATION`) for seamless GPU page tracking and speculative memory reads, matching Linux `sigaction` / `SA_SIGINFO`.
- **Fault-Isolated Graceful Degradation:** Optional subsystems (audio devices, gamepads) degrade gracefully (e.g. falling back to silent timer sinks or keyboard input) if hardware or drivers are absent, preventing aborts.
- **Security & Vulnerability Patches:** Includes bug fixes such as resolving the 18-exabyte unsigned buffer underflow vulnerability in the ATRAC9 RIFF header parser (`r_ajm_atrac9.c`).
- **High-Performance Vulkan Renderer:** Derived from shadPS4's GPU video core with two-stage draw pipelining, object motion vector reconstruction, and temporal upscaling via AMD FSR 3.1, FSR 4 (INT8), and FSR 4.1.1.

---

## 🎮 How to Test & Run

### Prerequisites
1. **Operating System:** Windows 10/11 (x86-64) or modern 64-bit Linux.
2. **Graphics:** GPU supporting Vulkan 1.3+ (NVIDIA GeForce GTX 10-series or newer, AMD Radeon RX 5000+ / RDNA, Intel Arc).
3. **Toolchain:**
   - **Windows:** MinGW GCC 13+ / Clang 16+ or MSVC 2022 (with CMake 3.24+).
   - **Linux:** GCC 11+ or Clang 14+ with standard build tools.
4. **Python:** Python 3.8+ for offline asset parsing and relocation linking.
5. **Game Dump:** Decrypted, plaintext copy of *Bloodborne* CUSA03173 (v1.09).

---

### Step 1: Smoke Test (No Game Dump Required)

You can immediately verify your Vulkan driver and graphics stack without any game files:

#### Using CMake:
```powershell
# Configure and build
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run Vulkan smoke test
.\build\bb-probe.exe --vulkan-only
```

#### Using the PowerShell Runner:
```powershell
.\run.ps1 -SmokeTest
```

If successful, Vulkan initializes, dispatches a test compute/fill buffer, and confirms presenter readiness.

---

### Step 2: Preparing and Running the Game

1. Place your decrypted **CUSA03173** directory either next to the repository (`../CUSA03173`) or specify its path via the `BB_GAME_DIR` environment variable.
2. Execute the launcher script:

#### On Windows (PowerShell):
```powershell
$env:BB_GAME_DIR = "C:\Path\To\CUSA03173"
.\run.ps1
```

#### On Windows (Command Prompt):
```cmd
set BB_GAME_DIR=C:\Path\To\CUSA03173
run.bat
```

#### On Linux:
```bash
export BB_GAME_DIR=/path/to/CUSA03173
./run.sh
```

The script automatically executes the four preparation steps into `out/`:
1. `scripts/prepare.py`: Extracts program headers and verifies SELF segments.
2. `scripts/link_libc.py`: Links PS4 native libc imports.
3. `scripts/link_modules.py`: Links dynamic game modules and TLS offsets.
4. `scripts/content_profile.py` & `scripts/patches.py`: Generates offline base profile and 60/90/uncapped FPS delta-time patches.

---

## 📁 Modular Subsystem Overview

| Subsystem | Directory | Description |
| :--- | :--- | :--- |
| **Platform Abstraction** | [`src/platform/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/) | Isolated Win32 & Linux OS services: memory, sync, threads, fs, time, faults. |
| **Virtual Memory** | [`src/runtime/memory/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/memory/) | 13 units ($\le 75$ LOC): Direct pool, VMA, section mapping, page protection. |
| **Threads & Kernel** | [`src/runtime/threads/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/threads/), [`kernel/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/kernel/) | 14 units ($\le 75$ LOC): Orbis thread lifecycle, TLS, signal traps, time keys. |
| **Audio & AJM** | [`src/runtime/audio/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/audio/), [`ajm/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/ajm/) | 17 units ($\le 75$ LOC): SDL3 streaming, ATRAC9 parser with RIFF underflow patch. |
| **Save Data** | [`src/runtime/savedata/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/savedata/) | 7 units ($\le 75$ LOC): Mount points, param.sfo serialization, thread-safe search. |
| **Real-Time Clock** | [`src/runtime/rtc/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/rtc/) | 5 units ($\le 75$ LOC): Gregorian civil conversion, ticks, time_t, RFC2822. |
| **AppContent** | [`src/runtime/content/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/content/) | 4 units ($\le 75$ LOC): System module tracker, offline base-game content profiles. |
| **System Services** | [`src/runtime/services/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/services/) | 10 units ($\le 75$ LOC): User, NetCtl, HTTP, NP/PSN, Dialogs, Virtual Keyboard, Trophies, PlayGo. |
| **Input / Gamepad** | [`src/runtime/pad/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/pad/) | 5 units ($\le 75$ LOC): DualShock 4 sampling, touch coordinates, rumble, script playback. |
| **Core & Dispatch** | [`src/runtime/core/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/runtime/core/) | 6 units ($\le 75$ LOC): Module TLS, exit handlers, static C++ guards, NID lookup. |
| **Loader & Boot** | [`src/loader/`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/loader/) | 9 units ($\le 75$ LOC): BBPROBE loader, relocations, BBPATCH2 runtime patching, CLI entry. |

---

## ⌨️ Controls & Input

### Gamepad (DualShock 4 / DualSense / Xbox / Standard Gamepads)
- Automatically detected via SDL3.
- Full rumble and touchpad emulation (Select / Back maps to left touchpad click).

### Keyboard Fallback (When No Gamepad is Connected)
| Keyboard Key | Gamepad Action |
| :--- | :--- |
| **W, A, S, D** | Left Analog Stick (Movement) |
| **Arrow Keys** | Right Analog Stick (Camera) |
| **Space** | Cross ($\times$) — Dodge / Sprint / Confirm |
| **Left Shift** | Circle ($\bigcirc$) — Back / Cancel |
| **E** | Square ($\square$) — Use Item |
| **Q** | Triangle ($\triangle$) — Switch Weapon Form / Heal |
| **1 / 3** | L1 / R1 — Left / Right Light Attack |
| **R / F** | L2 / R2 — Left Gun / Heavy Attack |
| **Z / C** | L3 / R3 — Lock-On / Crouch |
| **Enter** | Options / Start |
| **Tab / Backspace** | Touchpad Left / Right Click |
| **I, K, J, L** | D-Pad Up / Down / Left / Right |
| **Insert / L3+R3** | Open In-Game Settings Overlay Menu |

---

## ⚖️ License & Acknowledgments

- Upstream project: [deadinside28/bloodborne_pc](https://github.com/deadinside28/bloodborne_pc) (MIT License).
- GPU Video Core derived from [shadPS4](https://github.com/shadps4-emu/shadPS4) (GPL-3.0 License).
- ATRAC9 decoder: [LibAtrac9](https://github.com/Thealexbarney/LibAtrac9).
- AMD FSR: FidelityFX Super Resolution technology (AMD / MIT License).
