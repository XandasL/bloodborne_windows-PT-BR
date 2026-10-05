/* SPDX-License-Identifier: MIT
 * PS4 Loader Types and Internal Signatures.
 * Single responsibility: Loader structures, segments, and relocations. (~65 LOC)
 */
#ifndef LOADER_TYPES_H
#define LOADER_TYPES_H

#include "runtime.h"
#include "gpu/bbgpu.h"
#include "platform/bb_common.h"
#include "platform/memory.h"
#include "platform/faults.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

typedef struct { uint64_t address, size, flags; } Segment;
typedef struct { uint64_t target, kind, value, addend; } Reloc;
typedef struct { uint64_t base, size, init, tls_address, tls_memsz, tls_filesz, tls_module; } LinkedModule;

void fail(const char *message);
uint64_t read64(FILE *f);
size_t round_page(size_t size);
void *loader_allocate(size_t size);
void loader_protect(void *p, size_t size, unsigned flags);
int loader_mapped(Segment *segments, uint64_t count, uint64_t address, uint64_t bytes);

int sfo_value(const char *path, const char *key, char *text, size_t text_size, uint32_t *number);
void apply_patches(const char *path, Segment *segments, uint64_t ns, const Reloc *relocs, uint64_t nr);
void setup_import_traps(unsigned char *traps, uint64_t count);
void enter_guest(void *entry_point, void *params, void *exit_fn);
void loader_configure_content(const char *path);
void loader_configure_app(const char *app0, const char *user_dir);
void loader_init_gpu(const char *app0, const char *user_dir, uint64_t sdk, uint32_t attributes);
int loader_load_boot(FILE *f, const char *magic, uint64_t *out_entry, const char *patch_file);
int vulkan_smoke(void);

extern unsigned char *g_loader_image;
extern char (*g_loader_names)[128];
extern uint64_t g_loader_import_count;
extern LinkedModule g_loader_modules[16];
extern uint64_t g_loader_module_count;
extern int g_entered_game;
extern size_t g_page_size;

#endif /* LOADER_TYPES_H */
