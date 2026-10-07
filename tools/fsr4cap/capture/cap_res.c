// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

static ResInfo resources[4096];
static int resource_count;

static ResInfo* Res(ID3D12Resource* r) {
    if (!r) return NULL;
    for (int i = 0; i < resource_count; ++i) if (resources[i].res == r) return &resources[i];
    if (resource_count == 4096) return NULL;
    ResInfo* info = &resources[resource_count];
    info->res = r; info->id = resource_count++;
    info->desc = ID3D12Resource_GetDesc(r);
    D3D12_HEAP_PROPERTIES props = {0}; D3D12_HEAP_FLAGS flags;
    info->heap = SUCCEEDED(ID3D12Resource_GetHeapProperties(r, &props, &flags)) ? props.Type : 0;
    info->va = info->desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER
                   ? (real_get_va ? ((PfnGetGpuVa)real_get_va)(r) : ID3D12Resource_GetGPUVirtualAddress(r)) : 0;
    fprintf(trace, "RESOURCE r%d dim=%d %llux%ux%u mips=%u format=%d flags=0x%x heap=%d va=0x%llx\n",
            info->id, info->desc.Dimension, (unsigned long long)info->desc.Width, info->desc.Height,
            info->desc.DepthOrArraySize, info->desc.MipLevels, info->desc.Format, info->desc.Flags,
            info->heap, (unsigned long long)info->va);
    return info;
}

static ResInfo* ResByVa(D3D12_GPU_VIRTUAL_ADDRESS va, uint64_t* offset) {
    for (int i = 0; i < resource_count; ++i) {
        ResInfo* r = &resources[i];
        if (r->va && va >= r->va && va < r->va + r->desc.Width) { *offset = va - r->va; return r; }
    }
    return NULL;
}

static const void* MapRead(ResInfo* r, uint64_t offset, uint64_t size) {
    if (!r || (r->heap != D3D12_HEAP_TYPE_UPLOAD && r->heap != D3D12_HEAP_TYPE_READBACK)) return NULL;
    if (offset >= r->desc.Width) return NULL;
    void* data = NULL;
    if (FAILED(ID3D12Resource_Map(r->res, 0, NULL, &data)) || !data) return NULL;
    ID3D12Resource_Unmap(r->res, 0, NULL);
    return (const uint8_t*)data + offset;
}

static void DumpBytes(const char* what, ResInfo* r, uint64_t offset, uint64_t size) {
    if (r && r->desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && offset < r->desc.Width && offset + size > r->desc.Width)
        size = r->desc.Width - offset;
    const void* data = MapRead(r, offset, size);
    if (!data) { fprintf(trace, " %s=(not CPU visible)", what); return; }
    char name[64];
    const uint64_t h = Fnv(data, size);
    snprintf(name, sizeof(name), "data_%016llx.bin", (unsigned long long)h);
    WriteFile2(name, data, size);
    fprintf(trace, " %s=%s", what, name);
}
