// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#define COBJMACROS
#define WIDL_C_INLINE_WRAPPERS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../capture.h"

static FILE* trace;
static char dir[MAX_PATH];
static int dispatch_count;

static inline uint64_t Fnv(const void* data, size_t size) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < size; ++i) h = (h ^ ((const uint8_t*)data)[i]) * 1099511628211ull;
    return h;
}

static inline void WriteFile2(const char* name, const void* data, size_t size) {
    char path[MAX_PATH]; snprintf(path, sizeof(path), "%s\\%s", dir, name);
    FILE* f = fopen(path, "rb"); if (f) { fclose(f); return; }
    f = fopen(path, "wb"); if (f) { fwrite(data, 1, size, f); fclose(f); }
}

typedef struct { ID3D12Resource* res; int id; D3D12_RESOURCE_DESC desc; D3D12_HEAP_TYPE heap; D3D12_GPU_VIRTUAL_ADDRESS va; } ResInfo;
typedef struct { int kind; ResInfo* res; uint64_t a, b; int format; char text[160]; } View;
enum { VIEW_NONE, VIEW_CBV, VIEW_SRV, VIEW_UAV, VIEW_SAMPLER };
typedef struct { SIZE_T cpu; UINT64 gpu; UINT count, inc; View* views; } Heap;
typedef struct { void* obj; uint64_t hash; void* root; } Pso;
typedef struct { void* obj; uint64_t hash; } Root;
typedef struct { int type; UINT64 value; UINT32 consts[64]; UINT nconsts; } RootArg;
enum { ARG_NONE, ARG_TABLE, ARG_CBV, ARG_SRV, ARG_UAV, ARG_CONSTS };
typedef struct { void* list; void* root; void* pso; RootArg args[32]; } ListState;

#define SLOT(vtbl, method) (offsetof(vtbl, method) / sizeof(void*))
static void** device_vtbl;
static void** list_vtbl;
static void* real[512];
#define REAL(fn_type, slot) ((fn_type)real[slot])
typedef D3D12_GPU_VIRTUAL_ADDRESS (STDMETHODCALLTYPE* PfnGetGpuVa)(ID3D12Resource*);
static void* real_get_va;
