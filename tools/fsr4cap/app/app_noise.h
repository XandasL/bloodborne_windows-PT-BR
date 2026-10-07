// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "app_d3d.h"

static inline uint16_t Half(float f) {
    uint32_t x; memcpy(&x, &f, 4);
    const uint32_t sign = (x >> 16) & 0x8000u;
    const int exp = (int)((x >> 23) & 0xffu) - 127 + 15;
    if (exp <= 0) return (uint16_t)sign;
    return (uint16_t)(sign | ((uint32_t)exp << 10) | ((x >> 13) & 0x3ffu));
}

static uint32_t seed = 12345u;
static inline float Next(void) {
    seed = seed * 1664525u + 1013904223u;
    return (float)(seed >> 8) / (float)(1u << 24);
}

static inline ID3D12Resource* Buffer(D3D12_HEAP_TYPE type, UINT64 size) {
    D3D12_HEAP_PROPERTIES heap = {.Type = type};
    D3D12_RESOURCE_DESC desc = {.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER, .Width = size, .Height = 1,
                                .DepthOrArraySize = 1, .MipLevels = 1, .SampleDesc = {1, 0}, .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR};
    ID3D12Resource* r;
    CHECK(ID3D12Device_CreateCommittedResource(device, &heap, D3D12_HEAP_FLAG_NONE, &desc,
                                               type == D3D12_HEAP_TYPE_UPLOAD ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST,
                                               NULL, &IID_ID3D12Resource, (void**)&r));
    return r;
}

static inline void Upload(ID3D12Resource* tex, UINT w, UINT h, UINT bpp, void (*fill)(uint8_t* row, UINT w)) {
    const UINT pitch = (w * bpp + 255) & ~255u;
    ID3D12Resource* up = Buffer(D3D12_HEAP_TYPE_UPLOAD, (UINT64)pitch * h);
    uint8_t* data; CHECK(ID3D12Resource_Map(up, 0, NULL, (void**)&data));
    for (UINT y = 0; y < h; ++y) fill(data + (size_t)y * pitch, w);
    ID3D12Resource_Unmap(up, 0, NULL);
    D3D12_RESOURCE_DESC desc = ID3D12Resource_GetDesc(tex);
    D3D12_TEXTURE_COPY_LOCATION dst = {.pResource = tex, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    D3D12_TEXTURE_COPY_LOCATION src = {.pResource = up, .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT};
    src.PlacedFootprint.Footprint.Format = desc.Format;
    src.PlacedFootprint.Footprint.Width = w; src.PlacedFootprint.Footprint.Height = h;
    src.PlacedFootprint.Footprint.Depth = 1; src.PlacedFootprint.Footprint.RowPitch = pitch;
    ID3D12GraphicsCommandList_CopyTextureRegion(list, &dst, 0, 0, 0, &src, NULL);
    Submit();
    ID3D12Resource_Release(up);
}

static inline void FillColor(uint8_t* row, UINT w) {
    uint16_t* p = (uint16_t*)row;
    for (UINT x = 0; x < w; ++x) {
        p[4 * x + 0] = Half(Next() * 1.5f); p[4 * x + 1] = Half(Next() * 1.5f);
        p[4 * x + 2] = Half(Next() * 1.5f); p[4 * x + 3] = Half(1.0f);
    }
}
static inline void FillMotion(uint8_t* row, UINT w) {
    uint16_t* p = (uint16_t*)row;
    for (UINT x = 0; x < 2 * w; ++x) p[x] = Half(Next() * 4.0f - 2.0f);
}
static inline void FillDepth(uint8_t* row, UINT w) {
    float* p = (float*)row;
    for (UINT x = 0; x < w; ++x) p[x] = 0.9f + Next() * 0.1f;
}
