// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "app_noise.h"

static inline void Message(uint32_t type, const wchar_t* msg) {
    fprintf(stderr, "FFX %s: %ls\n", type == FFX_API_MESSAGE_TYPE_ERROR ? "error" : "warning", msg);
}

static inline uint64_t PickVersion(ffxFunctions* ffx, ID3D12Device* dev, const char* want) {
    uint64_t count = 0; struct ffxQueryDescGetVersions vers = {0};
    vers.header.type = FFX_API_QUERY_DESC_TYPE_GET_VERSIONS;
    vers.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    vers.device = dev; vers.outputCount = &count;
    ffx->Query(NULL, &vers.header);
    uint64_t ids[32]; const char* names[32];
    if (count > 32) count = 32;
    vers.versionIds = ids; vers.versionNames = names;
    ffx->Query(NULL, &vers.header);
    uint64_t chosen = 0;
    for (uint64_t i = 0; i < count; ++i) {
        printf("version %llu: id 0x%llx %s\n", (unsigned long long)i, (unsigned long long)ids[i], names[i]);
        if (!chosen && strstr(names[i], want)) chosen = ids[i];
    }
    return chosen;
}

static inline void SaveOutputRaw(ID3D12Resource* output, UINT ow, UINT oh) {
    const UINT pitch = (ow * 8 + 255) & ~255u;
    ID3D12Resource* rb = Buffer(D3D12_HEAP_TYPE_READBACK, (UINT64)pitch * oh);
    D3D12_TEXTURE_COPY_LOCATION src = {.pResource = output, .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX};
    D3D12_TEXTURE_COPY_LOCATION dst = {.pResource = rb, .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT};
    dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    dst.PlacedFootprint.Footprint.Width = ow; dst.PlacedFootprint.Footprint.Height = oh;
    dst.PlacedFootprint.Footprint.Depth = 1; dst.PlacedFootprint.Footprint.RowPitch = pitch;
    D3D12_RESOURCE_BARRIER b = {.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION};
    b.Transition.pResource = output; b.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    b.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    ID3D12GraphicsCommandList_ResourceBarrier(list, 1, &b);
    ID3D12GraphicsCommandList_CopyTextureRegion(list, &dst, 0, 0, 0, &src, NULL);
    Submit();
    uint8_t* data; CHECK(ID3D12Resource_Map(rb, 0, NULL, (void**)&data));
    char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
    snprintf(strrchr(path, '\\') + 1, 64, "output_%ux%u.raw", ow, oh);
    FILE* f = fopen(path, "wb");
    for (UINT y = 0; f && y < oh; ++y) fwrite(data + (size_t)y * pitch, 1, (size_t)ow * 8, f);
    if (f) fclose(f);
    printf("output written to %s\n", path);
}
