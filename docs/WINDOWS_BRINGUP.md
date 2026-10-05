# Windows Bring-Up Verification Roadmap: `bloodborne_windows`

This document tracks the phased empirical verification of `bloodborne_windows` on native Windows hardware (NVIDIA / AMD / Intel x86-64), pivoting from architecture refactoring to runtime correctness and stability.

---

## 🚦 Phased Verification Checklist

### Phase 1: Host Toolchain & Compilation Correctness
- [x] **SysV AMD64 ABI Verification:** Ensure host compiler natively generates SysV ABI call gates (`__attribute__((sysv_abi))`) for direct guest execution without clobbering registers (Clang / MinGW GCC required; vanilla MSVC `cl.exe` unsupported).
- [x] **Unified Memory Pool Struct:** Eliminate structure divergence across `mem_direct.c` and `mem_alias.c` via [`src/platform/memory/windows/mem_internal.h`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/platform/memory/windows/mem_internal.h).
- [x] **Portable Atomics:** Replace compiler-specific builtins with portable CAS (`bb_atomic_cas_ptr`) in [`src/include/bb_common.h`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/src/include/bb_common.h).
- [x] **CMake Windows Portability:** Remove unconditional `pkg_check_modules` calls from [`gpu/CMakeLists.txt`](file:///c:/Desktop/Stand-Up/Projects/Games/bloodborne_windows/gpu/CMakeLists.txt).
- [ ] CMake configuration succeeds cleanly on Windows without external pkg-config.
- [ ] `bb-probe.exe` compiles and links.
- [ ] `bbgpu` video core library builds.

---

### Phase 2: Low-Level Host Platform Smoke Tests
- [ ] **Vulkan Smoke Test:** Run `bb-probe.exe --vulkan-only` to verify Vulkan 1.3 physical device selection, queue initialization, compute dispatch, and 4096-byte memory readback.
- [ ] **SDL3 Window Creation:** Initialize native Win32 window with `SDL_PROP_WINDOW_WIN32_HWND_POINTER` and attach `VK_KHR_win32_surface`.
- [ ] **Physical Memory Aliasing:** Verify that two distinct virtual spans mapped via `MapViewOfFileEx` read and write to the same physical section.
- [ ] **Vectored Exception Handling (VEH):** Verify `AddVectoredExceptionHandler` cleanly catches page faults, services GPU dirty tracking, and safely resumes guest thread execution.
- [ ] **Input Polling:** Confirm SDL3 detects connected XInput / DualShock 4 / DualSense controllers.

---

### Phase 3: Guest Image & Loader Ingestion
- [ ] `scripts/prepare.py` parses and validates PS4 CUSA03173 ELF program headers.
- [ ] `scripts/link_libc.py` links static native libc symbols into `out/libc.bin`.
- [ ] `scripts/link_modules.py` resolves dynamic library exports and TLS descriptors.
- [ ] `scripts/content_profile.py` & `scripts/patches.py` generate offline game profile and 60 FPS patches.
- [ ] Low-address memory reservation below 40-bit boundary (`< 1 TiB`) succeeds on host.
- [ ] All ELF segments mapped with appropriate guest protections (`PAGE_EXECUTE_READ`, `PAGE_READWRITE`).

---

### Phase 4: First Guest Execution & Runtime HLE
- [ ] Control transfers to guest entry point via `enter_guest`.
- [ ] First guest instruction executes natively on host CPU.
- [ ] First PS4 libc / kernel call resolves via SysV call gate.
- [ ] First guest thread created via Win32 `_beginthreadex`.
- [ ] Virtual filesystem mounts and accesses decrypted game assets (`app0:`).
- [ ] First Vulkan command buffer submitted from guest GNM draw calls.
- [ ] First frame rendered to SDL3 display window.

---

### Phase 5: In-Game Stability & Playability
- [ ] Title screen displayed and accepts menu input.
- [ ] Character creation screen renders without vertex explosions.
- [ ] Intro cinematic plays.
- [ ] Ingestion into Hunter's Dream.
- [ ] Transition into Central Yharnam.
- [ ] Save data persisted and verified across process restarts (`user/savedata`).
- [ ] 30-minute stability run without memory leak or VEH crash.
- [ ] 2-hour stress test with audio, motion vectors, and FSR 3.1 enabled.
