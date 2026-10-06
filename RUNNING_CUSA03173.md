# How to Run Bloodborne (PS4 CUSA00900 / CUSA03173) on PC

A complete, step-by-step walkthrough for preparing, verifying, and executing **Bloodborne (PS4 CUSA00900 EU / CUSA03173 US, Update 1.09)** on Windows and Linux using the high-performance HLE Recompiler & Translation Runner.

> [!NOTE]
> **Architecture & Nature of This Project:**  
> This project is a **High-Level Emulation (HLE) Recompiler & Translation Runtime**, **NOT** an official native source port compiled from FromSoftware proprietary source code.  
> Because both the PlayStation 4 and modern PCs share the AMD64 (x86-64) architecture, guest CPU code runs directly on host cores via SysV ABI call gates, while GPU rendering and OS services are translated in real-time through Vulkan 1.3 and Win32/POSIX runtime wrappers (conceptually similar to Proton, Wine, and shadPS4).

---

## 📑 Table of Contents
1. [Supported Game Editions](#1-supported-game-editions)
2. [Prerequisites & Requirements](#2-prerequisites--requirements)
3. [Obtaining & Decrypting the Game (PS4)](#3-obtaining--decrypting-the-game-ps4)
4. [Verifying Your Game Dump Layout](#4-verifying-your-game-dump-layout)
5. [Running with the Pre-Built Release (v0.0.1)](#5-running-with-the-pre-built-release-v001)
6. [Running from Source](#6-running-from-source)
7. [Framerate Patches & Temporal Upscaling](#7-framerate-patches--temporal-upscaling)
8. [Troubleshooting & FAQ](#8-troubleshooting--faq)

---

## 1. Supported Game Editions

The runner and patch database natively support **all major regional editions** of Bloodborne updated to **v01.09**:

| Title ID | Region | Edition Description | Status |
| :--- | :--- | :--- | :---: |
| **`CUSA00900`** | Europe / UK | Standard & GOTY Edition | ✅ Fully Supported |
| **`CUSA03173`** | North America | Complete / GOTY Edition | ✅ Fully Supported |
| **`CUSA00207`** | North America | Standard Launch Edition | ✅ Fully Supported |
| **`CUSA00208`** | Europe | Alternative European Disc | ✅ Fully Supported |
| **`CUSA03023`** | Japan / Asia | The Old Hunters Edition | ✅ Fully Supported |
| **`CUSA01363`** | Japan | Standard Japanese Release | ✅ Fully Supported |

---

## 2. Prerequisites & Requirements

| Specification | Minimum | Recommended |
| :--- | :--- | :--- |
| **Operating System** | Windows 10/11 64-bit or Linux (Ubuntu 22.04+, Arch, Fedora) | Windows 11 (22H2+) / Linux Kernel 6.0+ |
| **CPU** | x86-64 CPU with AVX2 (Intel 8th Gen+ / AMD Ryzen 2000+) | 8-core CPU (Ryzen 5000+ / Intel 12th Gen+) |
| **GPU** | Vulkan 1.3 capable (GeForce GTX 1660 / Radeon RX 5600) | RTX 3060 / 4060+ or RX 6700 XT+ |
| **RAM** | 16 GB | 32 GB |
| **Storage** | 50 GB free space on SSD | NVMe SSD |
| **Software** | Python 3.8+ (added to `PATH`) | Python 3.11+ |
| **Game Version** | Decrypted plaintext dump with **Update 1.09** | Update 1.09 applied |

---

## 3. Obtaining & Decrypting the Game (PS4)

Retail PS4 disc/PKG packages are cryptographically signed with Sony SAMU hardware keys. The runner executes the x86-64 machine code directly and requires **decrypted plaintext ELF binaries**.

### Option A: Using ItemzFlow (Recommended)
1. Boot your jailbroken PS4 (firmware 5.05 – 11.00).
2. Launch **ItemzFlow Game Manager**.
3. Highlight **Bloodborne** (`CUSA00900` or `CUSA03173`) and press **Options**.
4. Select **Dump Game & Patch** (or **Dump All & Decrypt**).
5. Choose your target USB drive (`/mnt/usb0`).
6. ItemzFlow decrypts `eboot.bin`, all `sce_module/*.prx` libraries, and copies the `dvdroot_ps4/` asset folders.

### Option B: Using GoldHEN FTP Dump
1. Enable GoldHEN's FTP Server on your PS4.
2. Launch Bloodborne on the PS4 so the game mounts in memory (this decrypts binaries into RAM).
3. Connect from your PC via FTP (e.g., FileZilla, port 2121 or 1337).
4. Navigate to `/mnt/sandbox/pfsmnt/CUSA00900-app0/` (or `CUSA03173-app0`).
5. Download the entire directory to your PC.

---

## 4. Verifying Your Game Dump Layout

On your PC, your dumped folder (e.g. `D:\Games\Bloodborne\CUSA00900` or `CUSA03173`) must contain:

```text
CUSA00900/  (or CUSA03173)
├── eboot.bin                   # Decrypted game executable (~100-150 MB)
├── sce_sys/
│   └── param.sfo               # SFO metadata file (Title ID CUSA00900/CUSA03173)
├── sce_module/
│   ├── libc.prx                # Decrypted C runtime module
│   └── libSceFios2.prx         # Decrypted I/O file streaming module
└── dvdroot_ps4/                # Game assets filesystem
    ├── chr/                    # Character models and animations
    ├── map/                    # Map files and collision data
    ├── menu/                   # HUD, textures, and fonts
    ├── msg/                    # Text dialog and item descriptions
    ├── obj/                    # Environmental geometry
    ├── param/                  # Regulation and balance params
    ├── parts/                  # Weapons and armor models
    ├── sfx/                    # Particle effects and shaders
    └── sound/                  # Audio banks and music
```

### 🔍 Quick PowerShell Check for Decryption
Verify that `eboot.bin` has a valid unencrypted container header:
```powershell
$bytes = [System.IO.File]::ReadAllBytes("D:\Games\Bloodborne\CUSA00900\eboot.bin")[0..3]
[System.BitConverter]::ToString($bytes)
```
* **Valid:** Shows `4F-15-3D-1D` (ELF/SELF container).
* **Invalid:** If `prepare.py` complains about encrypted segments, the dump was copied cold without runtime decryption.

---

## 5. Running with the Pre-Built Release (v0.0.1)

1. **Download:**
   Download [`bloodborne-windows-x64.zip`](https://github.com/GuruMachanica/bloodborne_windows/releases/download/v0.0.1/bloodborne-windows-x64.zip) (or Linux tarball) from the [v0.0.1 Releases Page](https://github.com/GuruMachanica/bloodborne_windows/releases/tag/v0.0.1).
2. **Extract:** Extract the ZIP file into a convenient folder (e.g., `C:\Games\BloodborneRunner`).
3. **Smoke Test:** Confirm Vulkan 1.3 hardware initialization:
   ```powershell
   .\run.ps1 -SmokeTest
   ```
4. **Boot Bloodborne:** 
   Place your `CUSA00900` or `CUSA03173` folder right next to the runner folder, or specify its path explicitly:
   ```powershell
   # Point to your dumped game folder
   $env:BB_GAME_DIR = "D:\Games\Bloodborne\CUSA00900"
   .\run.ps1
   ```

*(On Linux, use `export BB_GAME_DIR="/path/to/CUSA00900"` and `./run.sh`)*

---

## 6. Running from Source

If you cloned the source repository:

```powershell
# 1. Configure and compile the runner
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target bb-probe

# 2. Point to game directory
$env:BB_GAME_DIR = "D:\Games\Bloodborne\CUSA00900"

# 3. Launch automated preparation and runner
.\run.ps1
```

---

## 7. Framerate Patches & Temporal Upscaling

The automated pipeline includes community-tested engine patches:

* **60 FPS Delta-Time Patch:**
  ```powershell
  .\run.ps1 -Fps 60
  ```
* **Uncapped Framerate:**
  ```powershell
  .\run.ps1 -Fps uncap
  ```
* **Temporal Upscaling (AMD FSR 3.1):**
  ```powershell
  .\run.ps1 -Fps 60 -Upscaler fsr3
  ```

---

## 8. Troubleshooting & FAQ

### Q: "Is this an official native PC port compiled from source code?"
**No.** This is an open-source **High-Level Emulation (HLE) Recompiler & Translation Runner**. It runs the original decrypted PS4 binary machine code on your CPU via ABI thunking and translates graphics commands into Vulkan.

### Q: `encrypted/compressed SELF segment is unsupported`
* **Cause:** The binary was extracted from an encrypted PKG without PS4 runtime decryption.
* **Fix:** Boot the game on your jailbroken PS4 and dump via ItemzFlow while running.

### Q: Black screen or immediate crash at boot
* **Fix 1:** Verify graphics driver supports Vulkan 1.3 (`vulkaninfo --summary`).
* **Fix 2:** In Windows Graphics Settings, add `bb-probe.exe` and select **High Performance** (forces discrete NVIDIA/AMD GPU).
* **Fix 3:** Add the runner folder to Windows Defender / Antivirus exclusions to allow low-memory virtual allocations ($< 1\text{ TiB}$).

### Q: Controller not responding
* **Fix:** Connect a DualSense, DualShock 4, or Xbox controller via USB/Bluetooth before launching. SDL3 automatically detects and maps standard gamepads.
