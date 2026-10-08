# Windows/NVIDIA test incident register (October 2026)

> **Scope:** `XandasL/bloodborne_windows-PT-BR`, experimental `test/vram-gc` branch.  
> **Status as of 2026-10-08:** One **critical open investigation**, one earlier **open stability investigation**, one **resolved monitoring-script defect**.  
> **Safety:** The latest run caused a complete Windows hang and required a physical power-off. **Do not request another reproduction or GPU stress test** until there is a specific reviewed mitigation.  
> This is a **diagnostic record**, not a claim that the graphics card is defective or that the experimental VRAM collector has been proven responsible.
>
> GitHub Issues was disabled on this repository when this record was prepared, so incidents are tracked in versioned Markdown instead of Issues.

## INC-001 — Critical: Vulkan device loss and full-system NVIDIA GPU timeout

**Status: OPEN — blocking further live gameplay tests on the affected setup.**  
**Date:** 2026-10-08, UTC-03:00.

### System and game configuration

| Item | Observed value |
| --- | --- |
| GPU | NVIDIA GeForce RTX 5070, 12 GB VRAM |
| NVIDIA driver version | **617.14** (**confirmed by tester to be installed during the incident**, with no intervening driver update) |
| OS | Windows x64, kernel version `10.0.26300` (WinDbg) |
| Port | Windows native Vulkan port, branch `test/vram-gc` |
| Output | 3840×2160 |
| Scene/internal render | 2560×1440 |
| Framerate patch | 60 FPS++ (192 patch writes) |
| Upscaler | FSR 4 v07 INT8 quality, 1440p inputs → 2160p output |
| Environment | Tester reported unusually hot ambient weather; room temperature unknown |
| Other games | No comparable behavior previously reported, but not retested since the incident |
| After restart | Desktop and GPU behavior reportedly returned to normal; physical damage not established |

The exact board partner/model, PSU, hotspot temperature, and whether optional GC diagnostic environment variables were enabled in this run have **not** been confirmed. **Driver 617.14 is confirmed as the incident-time version by the tester**, who explicitly reported no driver update between the crash and the version check.

### Timeline / observed outcome

1. Game ran until the entire Windows system became unresponsive. Tester ultimately had to power it off using the hardware button.
2. NVIDIA monitoring CSV: 430 samples between **18:18:25.142** and **18:33:10.155** local time.
3. Windows mini live dump: `WATCHDOG-20261008-1833.dmp`, **18:33:15.850** local time.
4. Reliability Monitor around **18:33:23–25**: NVIDIA Overlay/NVIDIA App failure, one `LiveKernelEvent 141` and two `LiveKernelEvent 1b8` reports.
5. The forced power-off was later logged as an unexpected shutdown.

The `1b8` black-screen live dumps should **not** automatically be treated as separate failures: these may be related to the black screen or the subsequent physical power-button action.

### Crucial Windows / driver diagnosis

WinDbg `!analyze -v` for the `141` live dump:

```text
VIDEO_ENGINE_TIMEOUT_DETECTED (141)
One of the display engines failed to respond in timely fashion.

Failure.Bucket: LKD_0x141_IMAGE_nvlddmkm.sys
IMAGE_NAME: nvlddmkm.sys
MODULE_NAME: nvlddmkm
PROCESS_NAME: System

dxgkrnl!TdrCollectDbgInfoStage1
dxgmms2!VidSchiResetHwEngine
dxgmms2!VidSchiResetEngines
dxgmms2!VidSchiCheckHwProgress
dxgmms2!VidSchiWorkerThread
```

**Interpretation:** Windows detected a GPU engine timeout and entered the graphics recovery / reset path. `nvlddmkm.sys` is implicated as the graphics driver module, but does **not** prove the NVIDIA driver alone caused the issue. `System` is the dump context, not evidence about the original application. The dump is a mini live-kernel dump, which limits attribution. A WinDbg `Unable to load image nvlddmkm.sys` warning concerned debugger image/symbol loading, not a missing driver.

### Crucial game / Vulkan log

The end of `last_run(4).log`:

```text
Texture cache: memory pressure, 5528 of 5235 MiB (critical 8512):
153 images evicted, 0 written back since the last report
VRAM diag: total 5528 MiB; VMA blocks 3499 MiB, used 3135 MiB,
free/fragmented 363 MiB; Vulkan images 2188 MiB; other VMA 946 MiB;
texture-cache live 1109 images / 728 MiB guest

GPU [Debug] <Critical> vk_scheduler.cpp:451 SubmitExecution: Assertion Failed!
Device lost during submit
STOP: GPU library assertion failed (see GPU log above)
```

Relevant call site: [`gpu/shadps4/video_core/renderer_vulkan/vk_scheduler.cpp`](../gpu/shadps4/video_core/renderer_vulkan/vk_scheduler.cpp), `Scheduler::SubmitExecution`, graphics queue `submit(...)`, which reports/asserts on `vk::Result::eErrorDeviceLost`.

The game log contained **103** pairs of `Texture cache: memory pressure` and `VRAM diag` reports. Its final internal Vulkan/VMA estimate was ~5.5 GiB, with a ~5.1 GiB pressure threshold and ~8.3 GiB critical threshold. The **nvidia-smi** driver-reported `memory.used` values below account for a different scope; these quantities are not directly interchangeable. Repeated image evictions are worth auditing but are not proof of use-after-free or the source of the timeout.

### NVIDIA monitoring: `gpu_monitor_20261008_181823_067.csv`

| Metric | Maximum observed | Last sample, 18:33:10.155 |
| --- | ---: | ---: |
| GPU temperature | **83 °C** | **80 °C** |
| GPU utilization | **98%** | **98%** |
| Device `memory.used` | **7,836 MiB** | **7,739 MiB** |
| GPU `power.draw` | **221.27 W** | **204.35 W** |
| GPU fan speed | **100%** | **100%** |

Fans were reported at 100% for **319/430 samples (~74%)**. Ambient conditions were reported as unusually hot. However **83 °C on the main sensor alone does not demonstrate hotspot overheating, thermal throttling, electrical instability, or hardware damage**. A hard hang can truncate both game and GPU logs.

### Additional WinDbg module and tagged-data evidence (2026-10-08)

The tester followed up on the `WATCHDOG-20261008-1833.dmp` with
`lmvm nvlddmkm` and `.enumtag`:

```text
module name: nvlddmkm T (no symbols)
Loaded symbol image file: nvlddmkm.sys
Image path: nvlddmkm.sys
Timestamp: Thu Sep 17 20:53:52 2026 (6AAC7D90)
ImageSize: 06D45000
Mapping Form: Loaded
```

**Interpretation:**
- The NVIDIA kernel module `nvlddmkm.sys` was **loaded**. `no symbols` is a lack of private driver debug symbols and **does not imply corruption or a missing driver**.
- The PE image timestamp is **not the installed NVIDIA driver version** and is **not the driver installation date**. The tester subsequently reported **NVIDIA driver version 617.14** and explicitly confirmed that **no driver update occurred after the freeze**, establishing this as the incident-time driver version.
- The supplied `.enumtag` text was approximately **6.9 MB / 99,999 lines**, almost entirely raw hexadecimal dump-callback bytes. The paste **starts inside an existing data block**, and the only visible block header was near the end (`{8BE1C8F0-B5BD-48FE-BCB7BBD165DEB285} - 0x10 bytes`), so it should not be treated as a complete tag inventory.
- No legible Vulkan command or NVIDIA timeout root cause could be attributed from the raw `.enumtag` bytes. The command enumerates secondary bugcheck callback blocks; decoding opaque private structures would require knowledge of the data format or specialized debugger extensions.
- The raw `.enumtag` material may contain system-memory excerpts and hardware metadata; **it is intentionally not copied into this public repository**.

**Version evidence:** `nvidia-smi` reported driver **617.14**; the tester confirmed this exact version was installed when the freeze occurred and no NVIDIA driver update was performed afterward. No need to run Bloodborne again.

### Potential leads — all UNCONFIRMED

- Vulkan queue submit, image life-cycle and deferred frees, GPU completed-timeline synchronization, and barriers.
- High-frequency texture cache pressure/evictions, especially during or following transitions.
- NVIDIA driver / RTX 50-series interaction with this port's command stream.
- Heavy 4K output + FSR4 compute workload, performance/long-running command timeouts.
- High ambient temperature as a potential contributing factor.

**Not established:** an actual VRAM exhaustion, physical GPU defect, thermal shutdown, a bad power supply, or a GC-specific regression.

### Investigation checklist

- [x] Obtain and confirm NVIDIA driver version **617.14**, installed during the incident, with no intervening driver update.
- [ ] Obtain GPU manufacturer/model; check if thermals are normal in regular desktop use.
- [ ] Inspect sanitized `lmvm nvlddmkm` and any non-sensitive kernel dump extensions if needed; **do not publish the raw `.dmp` without explicit review**.
- [ ] Review synchronization/timeline lifetime guarantees for `DeleteImage`, image cache eviction and the scheduler's graphics queue submission.
- [ ] Compare `test/vram-gc` with the upstream scheduler/GC and a previously more stable Windows build.
- [ ] Add diagnostic logging that persists immediately before submit/device loss without increasing load significantly.
- [ ] Develop an explicit safe mitigation before any further affected-device playtesting.

---

## INC-002 — Intermittent process-level crashes on Windows / NVIDIA during extended gameplay

**Status: OPEN — earlier behavior; root cause not established.**  
**Period:** Reported in tests around 2026-10-07, **before** the full-system timeout in INC-001.

### Observed behavior

- An earlier Yharnam gameplay session experienced approximately **two process crashes**.
- User reported some areas dipped to ~50 FPS despite a 60 FPS cap, but **no apparent gameplay graphics corruption** (menu and loading-screen glitches were separate).
- Earlier crash diagnostics pointed toward the NVIDIA **user-mode** graphics component `nvoglv64.dll` (separate from the **kernel-mode** `nvlddmkm.sys` in INC-001). Full previous traces are **not embedded in this record** and should be reconfirmed when available.
- Subsequent GC test iterations permitted longer sessions and repeated area transitions without immediately reproducing those earlier crashes.
- The later full-system TDR in INC-001 means earlier improved stability **must not be reported as a complete crash fix**.

### Follow-up

- [ ] Preserve/review old `last_run*.log` and Windows application crash records (faulting module, exception code, driver version).
- [ ] Distinguish ordinary game-process exits from Windows GPU engine timeouts.
- [ ] Compare incidents by build commit / exact `bb-probe.exe` version, graphics settings and runtime flags.
- [ ] Do not claim a shared root cause with INC-001 absent a matching trace.

---

## INC-003 — GPU monitor PowerShell startup error (`Split-Path -eq`)

**Status: FIXED in code, verified by Windows CI** (does not fix INC-001).

### Reproduction / root cause

The one-click `Play Bloodborne + GPU Monitor.bat` initially failed on startup with:

```text
Split-Path: Cannot find a parameter that matches parameter name 'eq'.
tools/monitor-gpu.ps1:31
```

The original PowerShell expression:

```powershell
$portDir = if (Split-Path -Leaf $PSScriptRoot -eq "tools") {
```

was interpreted as the cmdlet receiving an invalid `-eq` option.

### Fix / verification

Changed to:

```powershell
$portDir = if ((Split-Path -Leaf $PSScriptRoot) -eq "tools") {
```

- Fix: [`520cf9c`](https://github.com/XandasL/bloodborne_windows-PT-BR/commit/520cf9c40d2717958e918a40e06503f8b520cee3).
- CI startup smoke test added: [`75fd5ac`](https://github.com/XandasL/bloodborne_windows-PT-BR/commit/75fd5ac7f798633e8c1e724af1f28b47623bdd50).
- Windows workflow [run `37845762059`](https://github.com/XandasL/bloodborne_windows-PT-BR/actions/runs/37845762059): **success**.
- Regression test exercises actual script startup with a harmless OS executable in place of the game, not an NVIDIA GPU gameplay session. A PowerShell syntax-only parser was insufficient to catch this runtime parameter-binding mistake.

### Usage

The CI test artifact now stages files for direct copy into the port folder (not manually into arbitrary directories): `bin/bb-probe.exe`, `bin/bb-gpu-capabilities.exe`, `Play Bloodborne.exe`, `tools/monitor-gpu.ps1`, and `Play Bloodborne + GPU Monitor.bat`.

The monitor merely queries NVIDIA telemetry with `nvidia-smi`; **there is no evidence it caused the graphics driver timeout**.

---

## Artifact index / privacy

- `last_run(4).log` — terminal `VK_ERROR_DEVICE_LOST`, GC and Vulkan diagnostics.
- `gpu_monitor_20261008_181823_067.csv` — 430 samples ending ~5.7 s before the Windows `0x141` dump.
- `WATCHDOG-20261008-1833.dmp` — kernel-generated live mini dump for 0x141; only **sanitized WinDbg output** included above.
- `WATCHDOG4400-20261008-1833.dmp`, `WATCHDOG4401-20261008-1833.dmp` — 0x1b8 live dumps, not examined in depth.
- Windows Reliability Monitor report exported as XML — incident timeline.

These original local artifacts **are not uploaded publicly** in this repository; they may expose device identifiers, memory contents, file paths, or personal information. The excerpts above are diagnostic summaries.

## Guidance for future builds

**Do not present CI build success as a gameplay/GPU stability certification.** CI compiles/runs a startup smoke test in a cloud runner, while INC-001 specifically involves extended native Vulkan rendering on an NVIDIA device. Keep the affected Windows build flagged as experimental/high-risk until the graphics stability investigation is completed.
