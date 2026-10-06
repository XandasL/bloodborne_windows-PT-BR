# How to Run Bloodborne (PS4 CUSA03173) on PC

A complete, step-by-step walkthrough for preparing, verifying, and executing **Bloodborne (PS4 CUSA03173, Update 1.09)** on Windows and Linux using the native runner.

---

## 📑 Table of Contents
1. [Prerequisites & Requirements](#1-prerequisites--requirements)
2. [Obtaining & Decrypting the Game (PS4)](#2-obtaining--decrypting-the-game-ps4)
3. [Verifying Your Game Dump Layout](#3-verifying-your-game-dump-layout)
4. [Running with the Pre-Built Release (v0.0.1)](#4-running-with-the-pre-built-release-v001)
5. [Running from Source](#5-running-from-source)
6. [Framerate Patches & Upscaling](#6-framerate-patches--upscaling)
7. [Troubleshooting & FAQ](#7-troubleshooting--faq)

---

## 1. Prerequisites & Requirements

| Specification | Minimum | Recommended |
| :--- | :--- | :--- |
| **Operating System** | Windows 10/11 64-bit or Linux (Ubuntu 22.04+, Arch) | Windows 11 (22H2+) / Linux Kernel 6.0+ |
| **CPU** | x86-64 with AVX2 (Intel 8th Gen+ / AMD Ryzen 2000+) | 8-core CPU (Ryzen 5000+ / Intel 12th Gen+) |
| **GPU** | Vulkan 1.3 capable (GeForce GTX 1660 / Radeon RX 5600) | RTX 3060 / 4060+ or RX 6700 XT+ |
| **RAM** | 16 GB | 32 GB |
| **Storage** | 50 GB free space on SSD | NVMe SSD |
| **Software** | Python 3.8+ (added to `PATH`) | Python 3.11+ |
| **Game Version** | **PS4 CUSA03173 (Update 1.09)** — Decrypted plaintext | Update 1.09 applied |

---

## 2. Obtaining & Decrypting the Game (PS4)

Retail PS4 disc/PKG packages are cryptographically signed with Sony SAMU hardware keys. The PC runner executes the x86-64 machine code directly and requires **decrypted plaintext ELF binaries**.

### Option A: Using ItemzFlow (Recommended)
1. Boot your jailbroken PS4 (firmware 5.05 – 11.00).
2. Launch **ItemzFlow Game Manager**.
3. Highlight **Bloodborne** and press **Options**.
4. Select **Dump Game & Patch** (or **Dump All & Decrypt**).
5. Choose your target USB drive (`/mnt/usb0`).
6. ItemzFlow decrypts `eboot.bin`, all `sce_module/*.prx` libraries, and copies the `dvdroot_ps4/` asset folders.

### Option B: Using GoldHEN FTP Dump
1. Enable GoldHEN's FTP Server on your PS4.
2. Launch Bloodborne so the game is running (this decrypts the binaries in PS4 RAM).
3. Connect from your PC via FTP (e.g., FileZilla, port 2121 or 1337).
4. Navigate to `/mnt/sandbox/pfsmnt/CUSA03173-app0/` or `/user/app/CUSA03173/`.
5. Download the entire directory to your PC.

---

## 3. Verifying Your Game Dump Layout

On your PC, your dumped folder (e.g. `D:\Games\Bloodborne\CUSA03173`) must contain:

```text
CUSA03173/
├── eboot.bin                   # Decrypted game executable (~100-150 MB)
├── sce_sys/
│   └── param.sfo               # SFO metadata file (Title ID CUSA03173)
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
Verify that `eboot.bin` has a valid unencrypted header:
```powershell
$bytes = [System.IO.File]::ReadAllBytes("D:\Games\Bloodborne\CUSA03173\eboot.bin")[0..3]
[System.BitConverter]::ToString($bytes)
```
* **Valid:** Shows `4F-15-3D-1D` (ELF/SELF container).
* **Invalid:** If `prepare.py` complains about encrypted segments, the dump was copied cold without decryption.

---

## 4. Running with the Pre-Built Release (v0.0.1)

1. **Download Release:**
   Download [`bloodborne-windows-x64.zip`](https://github.com/GuruMachanica/bloodborne_windows/releases/download/v0.0.1/bloodborne-windows-x64.zip) (or Linux tarball) from the [v0.0.1 Releases Page](https://github.com/GuruMachanica/bloodborne_windows/releases/tag/v0.0.1).
2. **Extract:** Extract the ZIP file into a convenient directory (e.g., `C:\Games\BloodborneRunner`).
3. **Smoke Test:** Confirm Vulkan 1.3 hardware initialization:
   ```powershell
   .\run.ps1 -SmokeTest
   ```
4. **Boot Bloodborne:** Set the game path and launch:
   ```powershell
   $env:BB_GAME_DIR = "D:\Games\Bloodborne\CUSA03173"
   .\run.ps1
   ```

*(On Linux, use `export BB_GAME_DIR="/path/to/CUSA03173"` and `./run.sh`)*

---

## 5. Running from Source

If you cloned the source repository:

```powershell
# 1. Configure and compile the runner
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target bb-probe

# 2. Point to game directory
$env:BB_GAME_DIR = "D:\Games\Bloodborne\CUSA03173"

# 3. Launch automated preparation and runner
.\run.ps1
```

---

## 6. Framerate Patches & Upscaling

The automated pipeline incorporates delta-time engine patches:

* **60 FPS Target:**
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

## 7. Troubleshooting & FAQ

### Q1: `encrypted/compressed SELF segment is unsupported`
* **Cause:** The binary was extracted from an encrypted PKG without PS4 runtime decryption.
* **Fix:** Boot the game on your PS4 and dump via ItemzFlow or GoldHEN while running.

### Q2: Black screen or immediate crash at boot
* **Fix 1:** Verify graphics driver supports Vulkan 1.3 (`vulkaninfo --summary`).
* **Fix 2:** In Windows Graphics Settings, add `bb-probe.exe` and select **High Performance** (forces discrete NVIDIA/AMD GPU over integrated graphics).
* **Fix 3:** Add the runner folder to Windows Defender / Antivirus exclusions to allow low-memory virtual allocations ($< 1\text{ TiB}$).

### Q3: Controller not responding
* **Fix:** Connect a DualSense, DualShock 4, or Xbox controller via USB/Bluetooth before launching. SDL3 automatically maps standard controllers.
