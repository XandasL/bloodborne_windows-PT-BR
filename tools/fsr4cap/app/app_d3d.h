// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#define COBJMACROS
#define WIDL_C_INLINE_WRAPPERS
#define WIN32_LEAN_AND_MEAN
#define _WINDOWS
#include <windows.h>
#include <d3d12.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffx_api.h"
#include "ffx_api_loader.h"
#include "ffx_upscale.h"
#include "dx12/ffx_api_dx12.h"
#include "../capture.h"

#define CHECK(x) do { HRESULT hr_ = (x); if (FAILED(hr_)) { fprintf(stderr, "%s failed: 0x%08lx\n", #x, (unsigned long)hr_); exit(1); } } while (0)

static ID3D12Device* device;
static ID3D12CommandQueue* queue;
static ID3D12CommandAllocator* allocator;
static ID3D12GraphicsCommandList* list;
static ID3D12Fence* fence;
static UINT64 fence_value;
static HANDLE fence_event;

static inline void Submit(void) {
    CHECK(ID3D12GraphicsCommandList_Close(list));
    ID3D12CommandList* lists[] = {(ID3D12CommandList*)list};
    ID3D12CommandQueue_ExecuteCommandLists(queue, 1, lists);
    CHECK(ID3D12CommandQueue_Signal(queue, fence, ++fence_value));
    CHECK(ID3D12Fence_SetEventOnCompletion(fence, fence_value, fence_event));
    WaitForSingleObject(fence_event, INFINITE);
    CHECK(ID3D12CommandAllocator_Reset(allocator));
    CHECK(ID3D12GraphicsCommandList_Reset(list, allocator, NULL));
}

static inline ID3D12Resource* Texture(DXGI_FORMAT format, UINT w, UINT h, bool uav) {
    D3D12_HEAP_PROPERTIES heap = {.Type = D3D12_HEAP_TYPE_DEFAULT};
    D3D12_RESOURCE_DESC desc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, .Width = w, .Height = h, .DepthOrArraySize = 1,
        .MipLevels = 1, .Format = format, .SampleDesc = {1, 0},
        .Flags = uav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE,
    };
    ID3D12Resource* r;
    CHECK(ID3D12Device_CreateCommittedResource(device, &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, (void**)&r));
    return r;
}

static inline struct FfxApiResource Resource(ID3D12Resource* r, uint32_t format, uint32_t state) {
    D3D12_RESOURCE_DESC desc = ID3D12Resource_GetDesc(r);
    struct FfxApiResource out = {0};
    out.resource = r; out.state = state;
    out.description.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
    out.description.format = format; out.description.width = (uint32_t)desc.Width;
    out.description.height = desc.Height; out.description.depth = 1; out.description.mipCount = 1;
    out.description.usage = (state == FFX_API_RESOURCE_STATE_UNORDERED_ACCESS) ? FFX_API_RESOURCE_USAGE_UAV : 0;
    return out;
}
