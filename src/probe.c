/* SPDX-License-Identifier: MIT
 * PS4 Loader probe entry point (forwarder).
 * Single responsibility: Documentation bridge to modular src/loader/ units. (~20 LOC)
 */
#include "loader/loader_types.h"

/* The monolithic loader in probe.c has been decomposed into modular units in src/loader/:
 * - loader_mem.c: Low-space memory allocation and section permissions
 * - loader_sfo.c: Param.sfo parser for title and metadata
 * - loader_patch.c: BBPATCH2 runtime patch engine
 * - loader_reloc.c: Import trampolines and trap dispatch
 * - loader_config.c: Content profiles and GPU configuration
 * - loader_entry.c: Guest entry transfer and restart logic
 * - loader_boot.c: BBPROBE image parsing and relocations
 * - loader_main.c: CLI orchestration and kickoff
 */
