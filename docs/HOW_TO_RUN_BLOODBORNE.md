# Comprehensive Guide: Dumping, Preparing & Running Bloodborne on Windows

This guide provides the complete end-to-end instructions for preparing your legally owned copy of **Bloodborne (PS4 CUSA03173, Update 1.09)** and executing it natively on Windows with hardware Vulkan acceleration.

---

## 📋 System Prerequisites

| Component | Minimum Specification | Recommended |
| :--- | :--- | :--- |
| **Operating System** | Windows 10/11 64-bit | Windows 11 (Version 22H2+) |
| **Processor** | x86-64 CPU with AVX2 (Intel 8th Gen+ / AMD Ryzen 2000+) | 8-core CPU (Ryzen 5000+ / Intel 12th Gen+) |
| **Graphics Card** | Vulkan 1.3 capable GPU with 4GB VRAM | NVIDIA GeForce RTX 3060 / 4050+ or AMD RX 6600+ |
| **System Memory** | 16 GB RAM | 32 GB RAM |
| **Storage** | 50 GB free space on SSD (NVMe recommended) | High-speed NVMe SSD |
| **Python** | Python 3.8 or newer (added to system `PATH`) | Python 3.11+ |
| **Game Version** | **PS4 CUSA03173 (v1.09)** — Decrypted plaintext | v1.09 Update Applied |

---

## 💿 Step 1: Dumping and Decrypting Your Game (PS4)

To run Bloodborne on PC, you must decrypt the game executable (`eboot.bin`), system modules (`sce_module/*.prx`), and asset archives from your own jailbroken PS4 (firmware 5.05 – 11.00).

> [!CAUTION]
> **Do not use encrypted PKG or retail disc files directly.** Retail PS4 executables are signed and encrypted with Sony SAMU keys (`0x1D3D154F` encrypted flags). The runner requires **decrypted plaintext ELF binaries**.

### Recommended Dumping Tools:
1. **ItemzFlow Game Manager (Recommended):**
   - Launch ItemzFlow on your jailbroken PS4.
   - Navigate to **Bloodborne** -> Open **Game Options** -> Select **Dump Game & Patch**.
   - ItemzFlow automatically decrypts the `eboot.bin`, all `*.prx` modules, and extracts the game files to `/user/app/CUSA03173` or your external USB drive (`/mnt/usb0`).
2. **PS4-Dump / GoldHEN FTP:**
   - Enable GoldHEN's built-in FTP server.
   - Launch Bloodborne on the PS4 so the game mounts into memory (this decrypts the binaries in RAM).
   - Connect via FTP (e.g., FileZilla) and transfer the decrypted game folder to your PC.

---

## 📁 Step 2: Expected Game Directory Layout

On your PC, your dumped folder (e.g. `D:\Games\Bloodborne\CUSA03173`) must match this exact structure:

```text
CUSA03173/
├── eboot.bin                   # Decrypted PS4 game executable (~100-150 MB)
├── sce_sys/
│   └── param.sfo               # SFO metadata file (Title ID, version 01.09)
├── sce_module/
│   ├── libc.prx                # Decrypted native C library module
│   └── libSceFios2.prx         # Decrypted I/O streaming library module
└── dvdroot_ps4/                # Game asset filesystem
    ├── chr/                    # Character models and animations
    ├── map/                    # Map files and collision data
    ├── menu/                   # UI textures and fonts
    ├── msg/                    # Text and localization files
    ├── obj/                    # Environmental objects
    ├── param/                  # Game balance and regulation parameters
    ├── parts/                  # Equipment and weapon models
    ├── sfx/                    # Particle and visual effects
    └── sound/                  # Audio banks and music
```

### 🔍 How to Verify Your Executable is Decrypted:
Open PowerShell and run this quick check:
```powershell
$bytes = [System.IO.File]::ReadAllBytes("D:\Games\Bloodborne\CUSA03173\eboot.bin")[0..3]
[System.BitConverter]::ToString($bytes)
```
* **If it shows `4F-15-3D-1D`:** Your `eboot.bin` has the valid PS4 SELF container header and is ready for `prepare.py`.
* If `scripts/prepare.py` reports `encrypted/compressed SELF segment is unsupported`, the dump is encrypted and must be re-dumped using ItemzFlow.

---

## 🚀 Step 3: Launching via Automated Scripts

We have provided single-click launch automation that executes all four asset preparation passes and boots the runner.

### Method A: Using PowerShell (Recommended)
1. Open PowerShell in the project directory.
2. Specify your game folder and launch:
   ```powershell
   $env:BB_GAME_DIR = "D:\Games\Bloodborne\CUSA03173"
   .\run.ps1
   ```
3. To customize framerate target:
   ```powershell
   .\run.ps1 -Fps 60       # 60 FPS delta-time patch
   .\run.ps1 -Fps uncap    # Uncapped framerate
   ```

### Method B: Using Windows Command Prompt
```cmd
set BB_GAME_DIR=D:\Games\Bloodborne\CUSA03173
run.bat
```

---

## ⚙️ Step 4: What the Pipeline Does Behind the Scenes

When you execute `run.ps1`, it automatically carries out four offline compilation passes into the `out/` folder:

```
PS4 Game Dump (CUSA03173)
       │
       ├─▶ [Pass 1: scripts/prepare.py] ──────▶ out/boot.bin (ELF Header Extractor)
       ├─▶ [Pass 2: scripts/link_libc.py] ────▶ out/libc.bin (Static Libc Linker)
       ├─▶ [Pass 3: scripts/link_modules.py] ─▶ out/boot-linked.bin (Multi-module Linker)
       └─▶ [Pass 4: scripts/patches.py] ──────▶ out/patches.bin (60 FPS Patches)
                                                        │
                                                        ▼
                                           bb-probe.exe boot-linked.bin
```

1. **`prepare.py`:** Parses `eboot.bin`, maps ELF segments below 1 TiB, and generates `out/boot.bin` (BBPROBE container format).
2. **`link_libc.py`:** Reads `sce_module/libc.prx` and resolves native host library contracts.
3. **`link_modules.py`:** Intertwines `libSceFios2.prx` streaming exports and allocates guest TLS descriptors into `out/boot-linked.bin`.
4. **`content_profile.py` & `patches.py`:** Reads `param.sfo` game settings and compiles GoldHEN/shadPS4 60 FPS timing patches into `out/patches.bin`.
5. **`bb-probe.exe`:** Maps the unified image, binds the Vulkan 1.3 pipeline, registers Vectored Exception Handling, and transfers CPU execution to the native game entry point!

---

## 🎮 Step 5: Controls & Controller Setup

The runner utilizes **SDL3** for unified controller input, automatically mapping:
* **PlayStation Controllers:** DualSense and DualShock 4 (native touchpad coordinates and rumble supported).
* **Xbox Controllers:** Full XInput controller support.
* **Keyboard Fallback:** If no gamepad is connected, keyboard input maps to basic navigation.

---

## 🛠️ Troubleshooting Common Issues

### Issue 1: `encrypted/compressed SELF segment is unsupported`
* **Cause:** The `eboot.bin` was dumped without decrypting the binary.
* **Solution:** Re-dump the game on your PS4 while the game is running using ItemzFlow or GoldHEN Dump.

### Issue 2: `vulkan_smoke` or GPU Initialization Fails
* **Cause:** Outdated graphics drivers or running on a low-power integrated GPU.
* **Solution:** 
  1. Update to NVIDIA Driver 560+ or latest AMD Adrenalin.
  2. The runner automatically prioritizes discrete GPUs (e.g. RTX 4050). Ensure Windows Graphics Settings sets `bb-probe.exe` to "High Performance".

### Issue 3: Game Crash or Access Violation at Early Boot
* **Cause:** Antivirus interfering with low-memory virtual allocations below the 40-bit boundary (`< 1 TiB`).
* **Solution:** Add the `bloodborne_windows` directory to your Windows Defender exclusion list.
