# Architectural Plan & Modular Restructuring: `bloodborne_windows`

## 1. Objectives & Quality Invariants
1. **Strict Line-of-Code Budget:** Maximum ~75 LOC per C/C++ source file (target range: 50–80 LOC). Every file has exactly one clear, single responsibility.
2. **True Cross-Platform Decoupling:** Zero `#ifdef _WIN32` or `#ifdef __linux__` inside PS4 HLE runtime code (`src/runtime/`). All OS primitives are isolated in `src/platform/`.
3. **Fault Isolation & Independent Degradation:** If an optional or platform subsystem encounters an issue (e.g. audio device, high-end FSR asset, direct memory alias), it fails gracefully into a safe fallback mode (e.g. silent timer clock, fallback AA, standard paging) without bringing down the runtime or other modules.
4. **Preserved PS4 Semantics & ABIs:** Maintain System V AMD64 ABI, exact Orbis error code translation, 40-bit address restrictions (< 1 TiB), and accurate timing.

---

## 2. Target Directory Hierarchy

```
bloodborne_windows/
├── CMakeLists.txt                       # Unified top-level build (Ninja / MSVC / Clang-CL)
├── docs/
│   ├── WINDOWS_PORT_STATUS.md           # Progress tracking matrix by subsystem
│   └── RESTRUCTURING_PLAN.md            # Architectural blueprint and modular layout
├── src/
│   ├── include/
│   │   ├── bb_common.h                  # Common macros, types, error codes
│   │   └── platform/
│   │       ├── memory.h                 # VM reserve, commit, protect, physical aliasing
│   │       ├── sync.h                   # Mutexes, RWLocks, Semaphores, Conditions
│   │       ├── threads.h                # Thread spawning, joining, naming, guest TLS
│   │       ├── fs.h                     # File open, read, write, stat, dir listing
│   │       ├── faults.h                 # Vectored / Signal exception dispatch
│   │       ├── time.h                   # Monotonic clocks, sleep, TSC frequency
│   │       └── process.h                # Process parameters, PID, graceful restart
│   │
│   ├── platform/                        # Platform implementations (~75 LOC per file)
│   │   ├── memory/
│   │   │   ├── mem_types.h
│   │   │   ├── linux/
│   │   │   │   ├── mem_vm.c             # mmap / mprotect
│   │   │   │   ├── mem_direct.c         # memfd direct pool
│   │   │   │   └── mem_alias.c          # Physical aliasing
│   │   │   └── windows/
│   │   │       ├── mem_vm.c             # VirtualAlloc2 / VirtualProtect
│   │   │       ├── mem_direct.c         # CreateFileMappingW section
│   │   │       └── mem_alias.c          # MapViewOfFile3 placeholder splitting
│   │   ├── sync/
│   │   │   ├── sync_types.h
│   │   │   ├── linux/
│   │   │   │   ├── mutex.c              # pthread_mutex wrappers
│   │   │   │   ├── rwlock.c             # pthread_rwlock wrappers
│   │   │   │   ├── cond.c               # pthread_cond wrappers
│   │   │   │   └── sema.c               # POSIX semaphore simulation
│   │   │   └── windows/
│   │   │       ├── mutex.c              # SRWLOCK / CriticalSection
│   │   │       ├── rwlock.c             # Slim Reader/Writer (SRW)
│   │   │       ├── cond.c               # CONDITION_VARIABLE
│   │   │       └── sema.c               # Semaphore kernel objects / WaitOnAddress
│   │   ├── threads/
│   │   │   ├── thread_types.h
│   │   │   ├── linux/
│   │   │   │   ├── thread_create.c      # pthread_create with low-memory stack
│   │   │   │   ├── thread_tls.c         # arch_prctl(ARCH_SET_GS)
│   │   │   │   └── thread_attr.c        # Affinity & stack configuration
│   │   │   └── windows/
│   │   │       ├── thread_create.c      # _beginthreadex with reserved stack
│   │   │       ├── thread_tls.c         # TEB / TLS slot anchor
│   │   │       └── thread_attr.c        # Win32 thread priorities & affinity
│   │   ├── fs/
│   │   │   ├── fs_types.h
│   │   │   ├── linux/
│   │   │   │   ├── fs_io.c              # open, pread, pwrite, close
│   │   │   │   ├── fs_stat.c            # fstat, stat
│   │   │   │   └── fs_dir.c             # opendir, readdir
│   │   │   └── windows/
│   │   │       ├── fs_io.c              # CreateFileW, ReadFile, WriteFile
│   │   │       ├── fs_stat.c            # GetFileInformationByHandle
│   │   │       └── fs_dir.c             # FindFirstFileW / FindNextFileW
│   │   ├── faults/
│   │   │   ├── fault_types.h
│   │   │   ├── linux/
│   │   │   │   ├── fault_sigaction.c    # SIGSEGV / SIGBUS handling
│   │   │   │   └── fault_trace.c        # Linux stack walking
│   │   │   └── windows/
│   │   │       ├── fault_veh.c          # AddVectoredExceptionHandler
│   │   │       └── fault_trace.c        # CaptureStackBackTrace
│   │   ├── time/
│   │   │   ├── linux/time.c             # clock_gettime, nanosleep
│   │   │   └── windows/time.c           # QueryPerformanceCounter, Sleep
│   │   └── process/
│   │       ├── linux/process.c          # fork/exec restart
│   │       └── windows/process.c        # CreateProcessW restart
│   │
│   ├── runtime/                         # Pure PS4 HLE contracts (~75 LOC per file)
│   │   ├── memory/
│   │   │   ├── r_mem_vma.c              # Virtual memory area list management
│   │   │   ├── r_mem_direct.c           # sceKernelAllocateDirectMemory
│   │   │   ├── r_mem_protect.c          # sceKernelProtectVirtualRange
│   │   │   └── r_mem_query.c            # sceKernelVirtualQuery
│   │   ├── sync/
│   │   │   ├── r_sync_mutex.c           # scePthreadMutex* NID bindings
│   │   │   ├── r_sync_rwlock.c          # scePthreadRwlock* NID bindings
│   │   │   └── r_sync_sema.c            # sceKernelSem* NID bindings
│   │   ├── threads/
│   │   │   ├── r_thread_spawn.c         # scePthreadCreate
│   │   │   ├── r_thread_keys.c          # Thread-specific keys
│   │   │   └── r_thread_once.c          # scePthreadOnce
│   │   ├── fs/
│   │   │   ├── r_fs_mount.c             # Mount table (/app0, /data, /savedata)
│   │   │   ├── r_fs_path.c              # Guest-to-host path translation & guards
│   │   │   └── r_fs_syscalls.c          # sceKernelOpen/Read/Write/Close
│   │   ├── audio/
│   │   │   ├── r_audio_ports.c          # Port table & configuration
│   │   │   ├── r_audio_stream.c         # SDL audio feed & pacing
│   │   │   └── r_audio_remap.c          # 8ch to SDL 7.1 channel remapping
│   │   ├── savedata/
│   │   │   ├── r_save_mount.c           # Mount /savedata slots
│   │   │   ├── r_save_search.c          # Directory search & sort
│   │   │   └── r_save_sound_hack.c      # Bloodborne audio flag fix
│   │   ├── pad/
│   │   │   ├── r_pad_sample.c           # SDL gamepad sampling
│   │   │   ├── r_pad_keyboard.c         # Keyboard fallback map
│   │   │   └── r_pad_touch.c            # Touchpad emulation
│   │   └── ajm/
│   │       ├── r_ajm_batch.c            # Batch & chunk parser
│   │       ├── r_ajm_atrac9.c           # LibAtrac9 decoder invocation
│   │       └── r_ajm_riff.c             # Safe RIFF chunk parser (bug-free)
│   │
│   └── loader/
│       ├── probe_main.c                 # Entry point & CLI argument parsing
│       ├── probe_elf.c                  # Flat memory image loader
│       ├── probe_relocs.c               # Relocation application
│       └── probe_traps.c                # Unresolved NID trap generators
│
├── gpu/
│   ├── CMakeLists.txt                   # Platform-neutral GPU build
│   └── shim/
│       └── window.cpp                   # Fixed Win32 SDL3 HWND surface
```

---

## 3. Subsystem Breakdown & ~75 LOC File Plan

### 3.1. Virtual Memory & Physical Aliasing
| File | Responsibility | Target LOC |
|---|---|---|
| `src/include/platform/memory.h` | Platform VM interface definitions & prot flags | ~45 LOC |
| `src/platform/memory/windows/mem_vm.c` | `VirtualAlloc2` with `MEM_RESERVE_PLACEHOLDER` | ~70 LOC |
| `src/platform/memory/windows/mem_direct.c` | `CreateFileMappingW` anonymous paging section | ~65 LOC |
| `src/platform/memory/windows/mem_alias.c` | `MapViewOfFile3` / placeholder splitting & aliasing | ~75 LOC |
| `src/platform/memory/linux/mem_vm.c` | POSIX `mmap` anonymous reservations | ~60 LOC |
| `src/platform/memory/linux/mem_direct.c` | `memfd_create` sparse pool allocation | ~65 LOC |
| `src/platform/memory/linux/mem_alias.c` | Multiple `mmap` offsets over single `memfd` | ~60 LOC |

### 3.2. Synchronization Primitives
| File | Responsibility | Target LOC |
|---|---|---|
| `src/include/platform/sync.h` | Opaque handle contracts for Mutex/RWLock/Sema | ~50 LOC |
| `src/platform/sync/windows/mutex.c` | Win32 `SRWLOCK` with recursion simulation | ~75 LOC |
| `src/platform/sync/windows/rwlock.c` | Win32 `AcquireSRWLockShared`/`Exclusive` | ~60 LOC |
| `src/platform/sync/windows/sema.c` | Win32 Semaphore kernel object / timed waits | ~75 LOC |
| `src/platform/sync/linux/mutex.c` | POSIX `pthread_mutex_lock`/`unlock` wrappers | ~65 LOC |
| `src/platform/sync/linux/rwlock.c` | POSIX `pthread_rwlock` wrappers | ~60 LOC |
| `src/platform/sync/linux/sema.c` | Counted tokens + condition variable queue | ~75 LOC |

### 3.3. Threading & TLS Handling
| File | Responsibility | Target LOC |
|---|---|---|
| `src/include/platform/threads.h` | Thread interface, attributes, stack & TLS contracts | ~45 LOC |
| `src/platform/threads/windows/thread_create.c` | `_beginthreadex` with low-memory stack bounds | ~75 LOC |
| `src/platform/threads/windows/thread_tls.c` | Windows TEB / TLS slot anchor mapping | ~65 LOC |
| `src/platform/threads/linux/thread_create.c` | `pthread_create` with stack allocation | ~70 LOC |
| `src/platform/threads/linux/thread_tls.c` | `arch_prctl(ARCH_SET_GS)` implementation | ~55 LOC |

### 3.4. Filesystem & Mount Translation
| File | Responsibility | Target LOC |
|---|---|---|
| `src/include/platform/fs.h` | Platform file operations (UTF-8 host paths) | ~50 LOC |
| `src/platform/fs/windows/fs_io.c` | `CreateFileW`, `ReadFile`, `WriteFile`, `CloseHandle` | ~75 LOC |
| `src/platform/fs/windows/fs_stat.c` | `GetFileInformationByHandle` & timestamp conversion | ~70 LOC |
| `src/platform/fs/windows/fs_dir.c` | `FindFirstFileW`, `FindNextFileW` directory listing | ~75 LOC |
| `src/platform/fs/linux/fs_io.c` | `open`, `pread`, `pwrite`, `close` | ~65 LOC |
| `src/platform/fs/linux/fs_stat.c` | `stat`, `fstat` conversion | ~60 LOC |
| `src/platform/fs/linux/fs_dir.c` | `opendir`, `readdir` listing | ~70 LOC |

---

## 4. Phase-by-Phase Execution Order

```
[Phase 0: Build Infrastructure & Tracking]
  ├── Create docs/WINDOWS_PORT_STATUS.md
  ├── Fix gpu/shim/window.cpp (Win32 HWND surface hookup)
  ├── Clean gpu/CMakeLists.txt (remove hardcoded X11/pkg-config on WIN32)
  └── Create top-level CMakeLists.txt

[Phase 1: Platform Interface & Linux Extraction (Zero Regression)]
  ├── Create src/include/platform/*.h
  ├── Extract src/platform/linux/*
  └── Validate with Linux test suite (all tests pass identically)

[Phase 2: Windows Platform Implementations]
  ├── src/platform/time/windows/
  ├── src/platform/sync/windows/
  ├── src/platform/fs/windows/
  ├── src/platform/faults/windows/
  ├── src/platform/threads/windows/
  └── src/platform/memory/windows/

[Phase 3: Runtime HLE Modular Refactoring (~75 LOC/file)]
  ├── Modularize src/runtime/ into separate subpackages
  ├── Fix known bugs (parse_riff underflow, mutex lifetime race, save search data race)
  └── Compile on Windows with Ninja / MSVC / Clang-CL
```
