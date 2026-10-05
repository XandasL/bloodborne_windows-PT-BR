# Windows Bring-Up Verification Roadmap: `bloodborne_windows`

This document tracks the phased empirical verification of `bloodborne_windows` on native Windows hardware (NVIDIA / AMD / Intel x86-64), pivoting from architecture refactoring to runtime correctness and stability.

---

## 🚦 Phased Verification Checklist

### Phase 1: Host Toolchain & Compilation Correctness
- [x] **SysV AMD64 ABI Verification:** Ensure host compiler natively generates SysV ABI call gates (`__attribute__((sysv_abi))`) for direct guest execution without clobbering registers (Clang / MinGW GCC verified; MSVC `cl.exe` guarded with compile-time check).
- [x] **Unified Memory Pool Struct:** Structure divergence between `mem_direct.c` and `mem_alias.c` resolved via [`src/platform/memory/windows/mem_internal.h`](../src/platform/memory/windows/mem_internal.h).
- [x] **Portable Atomics:** Compiler-specific builtins replaced with portable CAS (`bb_atomic_cas_ptr`) in [`src/include/bb_common.h`](../src/include/bb_common.h). Verified under multi-threaded concurrency.
- [x] **CMake Windows Portability:** Removed unconditional `pkg_check_modules` calls from [`gpu/CMakeLists.txt`](../gpu/CMakeLists.txt).
- [ ] Full CMake project link (`bbgpu.dll` & `bb-probe.exe`).

---

### Phase 2: Low-Level Host Platform Smoke Tests
- [x] **Vulkan Smoke Test:** Executed on native hardware via `src/vulkan_smoke.c`. Automatically selected discrete **`NVIDIA GeForce RTX 4050 Laptop GPU`**, submitted graphics/transfer command buffer, executed memory barrier, and completed 4096-byte readback (**PASS**).
- [x] **Physical Memory Aliasing:** Verified via [`tests/test_win32_platform_memory.c`](../tests/test_win32_platform_memory.c). Two distinct virtual address spans (`0x...10000` & `0x...20000`) mapped to physical pool via `MapViewOfFileEx`; verified physical write coherency (`0xDEADBEEF`, `0xC001CAFE`) and commit hole punch zeroing (**PASS**).
- [x] **Vectored Exception Handling (VEH):** Verified via [`tests/test_win32_platform_veh.c`](../tests/test_win32_platform_veh.c). Intercepted `EXCEPTION_ACCESS_VIOLATION` on `PAGE_NOACCESS`, repaired page protection on-the-fly, transparently resumed thread execution, and verified `setjmp`/`longjmp` recovery (**PASS**).
- [x] **Threads, Mutex, Semaphore & TLS:** Verified via [`tests/test_win32_platform_threads_sync.c`](../tests/test_win32_platform_threads_sync.c). Spawned concurrent worker threads via `_beginthreadex`, tested C11 `_Thread_local` guest TCB isolation, critical section mutex counter, and semaphore signaling (**PASS**).
- [x] **Win32 Filesystem:** Verified via [`tests/test_win32_platform_fs.c`](../tests/test_win32_platform_fs.c). Created, wrote, read, stat, and enumerated directory files via Win32 API (**PASS**).
- [ ] **Input Polling:** Confirm SDL3 detects connected XInput / DualShock 4 / DualSense controllers.

---

### Phase 3: Guest Image & Loader Ingestion
- [x] `scripts/prepare.py` parses and validates PS4 CUSA03173 ELF program headers (CLI & options verified).
- [x] `scripts/link_libc.py` links static native libc symbols into `out/libc.bin` (CLI & options verified).
- [x] `scripts/link_modules.py` resolves dynamic library exports and TLS descriptors (CLI & options verified).
- [x] `scripts/content_profile.py` & `scripts/patches.py` generate offline game profile and 60 FPS patches (CLI & options verified).
- [ ] Low-address memory reservation below 40-bit boundary (`< 1 TiB`) mapped with live game dump.
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
