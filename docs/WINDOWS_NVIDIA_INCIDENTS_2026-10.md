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

### Related public reports in upstream graphics code (research 2026-10-08)

These were found in **other repositories**, not reproduced or verified as the same defect as INC-001. They are high-priority comparison targets for source review. In particular, distinct vendors/OSes can reach Vulkan `VK_ERROR_DEVICE_LOST` for unrelated reasons.

1. **[deadinside28/bloodborne_pc #56 — GPU hang / AMDGPU reset while loading map](https://github.com/deadinside28/bloodborne_pc/issues/56)** — **OPEN** (filed 2026-10-07). Original Linux bbport on AMD/RADV: map load stalls an indexed draw, Linux reports `amdgpu: ring gfx timeout` and performs a full GPU reset that temporarily kills the desktop graphics session. The game ends in `vk_scheduler.cpp`: `Device lost during submit`. **Directly relevant:** that log also contains the five `SanitizeCopyLayers: Coercing copy source layers … to minimum` warnings present in our startup log. In the reporter's capture VRAM is *not* exhausted. Their GPU breadcrumbs pinpoint an indexed draw — **not necessarily the same offending draw as ours**.

2. **[deadinside28/bloodborne_pc #39 — Windows AMD device lost / object motion](https://github.com/deadinside28/bloodborne_pc/issues/39)** — **OPEN** (filed 2026-10-06). Windows Vulkan bbport on an AMD RX 5600 XT: identical `vk_scheduler.cpp:450 SubmitExecution` / `Device lost during submit` signature, and again the same `SanitizeCopyLayers` warnings. In the issue body and follow-up comment, the author ran limited A/B tests: object motion on with vertex writes failed after ~35–55 s (five trials), while object motion off or its per-vertex-write path disabled ran **120 s without failure**. The author could not separate the vertex writes from the additional shader/pipeline variants as the exact trigger. **Our log also says** `Object motion: on (4194304 vertices per frame)`. This is a **strong audit lead**, **not a fix proven on NVIDIA** and **not grounds to ask for a hard-freeze reproduction**. The same issue separately reports actual AMD out-of-VRAM allocation failures with `0 images evicted`, which differ from our NVIDIA log, which actively evicted images and stayed below its logged critical threshold.

3. **[deadinside28/bloodborne_pc #34 — indirect-dispatch GPU hang](https://github.com/deadinside28/bloodborne_pc/issues/34)** — **OPEN** (filed 2026-10-06). AMD/RADV Linux, experimental `BB_GUEST_IN_PLACE=1`: GPU breadcrumbs show an indirect dispatch reading an implausibly large group count and ending in device loss. Relevant as an example of **command-stream validity** causing GPU failure; our use of that experimental memory model has **not** been established.

4. **[shadps4-emu/shadPS4 #4510 — same submit assertion on RTX 5080](https://github.com/shadps4-emu/shadPS4/issues/4510)** — **OPEN** (filed 2026-06-02). Reports exact `Device lost during submit` on NVIDIA RTX 5080, **but a different title: EA Sports UFC 3**, not Bloodborne. Shows the renderer family and NVIDIA can encounter the symptom but is **not direct Bloodborne evidence**.

5. **[shadps4-emu/shadPS4 #4816 — device-loss diagnostic proposal](https://github.com/shadps4-emu/shadPS4/issues/4816)** — **OPEN** (filed 2026-08-08). A different game's fork study proposes `VK_EXT_device_fault`, `VK_EXT_device_address_binding_report` and `VK_NV_device_diagnostic_checkpoints` for more useful Vulkan loss diagnostics, and discusses a possible unbounded retry on a lost device. **Evaluate extension availability and overhead before borrowing designs**; these are proposed diagnostics, not an established fix for this incident.

6. Community reports include an [shadPS4 Bloodborne session that sometimes froze the entire desktop before resuming](https://www.reddit.com/r/shadps4/comments/1uktigz/bloodborne_on_shadps4_randomly_freezes_but/) (2026-07-01) and [Windows Bloodborne on RTX 5070 with repeat crashes after fast travel](https://www.reddit.com/r/BloodbornePC/comments/1vd276d/game_crashes_when_fast_traveling/) (2026-08-01). Both are **shadPS4**, not necessarily this bbport, and neither reproduces our exact WinDbg `0x141` evidence.

**Action for developers:** review object-motion per-vertex writes / pipeline variants, cached and deferred image lifetimes, copy-layer coercion, and Vulkan GPU breadcrumbs/checkpoints **before** further affected-machine testing. Compare the actual source branches and commits before porting any fix. Similar warnings/errors alone do not show common causality.

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
