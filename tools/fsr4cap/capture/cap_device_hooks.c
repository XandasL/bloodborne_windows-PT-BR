// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

static void Patch(void** vtbl, size_t slot, void* hook, int store) {
    DWORD old;
    VirtualProtect(&vtbl[slot], sizeof(void*), PAGE_READWRITE, &old);
    real[store] = vtbl[slot];
    vtbl[slot] = hook;
    VirtualProtect(&vtbl[slot], sizeof(void*), old, &old);
}

typedef HRESULT (STDMETHODCALLTYPE* PfnCreateComputePipelineState)(ID3D12Device*, const D3D12_COMPUTE_PIPELINE_STATE_DESC*, REFIID, void**);
static HRESULT STDMETHODCALLTYPE HookCreateComputePipelineState(ID3D12Device* This, const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc, REFIID riid, void** out) {
    HRESULT hr = REAL(PfnCreateComputePipelineState, 0)(This, desc, riid, out);
    if (SUCCEEDED(hr) && out && *out && pso_count < 1024) {
        const uint64_t h = Fnv(desc->CS.pShaderBytecode, desc->CS.BytecodeLength);
        char name[64]; snprintf(name, sizeof(name), "cs_%016llx.dxil", (unsigned long long)h);
        WriteFile2(name, desc->CS.pShaderBytecode, desc->CS.BytecodeLength);
        psos[pso_count++] = (Pso){*out, h, desc->pRootSignature};
        fprintf(trace, "PSO %p cs=%016llx root=%016llx\n", *out, (unsigned long long)h, (unsigned long long)RootHash(desc->pRootSignature));
    }
    return hr;
}

typedef HRESULT (STDMETHODCALLTYPE* PfnCreateRootSignature)(ID3D12Device*, UINT, const void*, SIZE_T, REFIID, void**);
static HRESULT STDMETHODCALLTYPE HookCreateRootSignature(ID3D12Device* This, UINT mask, const void* blob, SIZE_T size, REFIID riid, void** out) {
    HRESULT hr = REAL(PfnCreateRootSignature, 1)(This, mask, blob, size, riid, out);
    if (SUCCEEDED(hr) && out && *out && root_count < 256) {
        const uint64_t h = Fnv(blob, size);
        char name[64]; snprintf(name, sizeof(name), "root_%016llx.bin", (unsigned long long)h);
        WriteFile2(name, blob, size);
        roots[root_count++] = (Root){*out, h};
        fprintf(trace, "ROOTSIG %p %016llx\n", *out, (unsigned long long)h);
        CaptureDescribeRootSignature(trace, blob, size);
    }
    return hr;
}

typedef HRESULT (STDMETHODCALLTYPE* PfnCreateDescriptorHeap)(ID3D12Device*, const D3D12_DESCRIPTOR_HEAP_DESC*, REFIID, void**);
static HRESULT STDMETHODCALLTYPE HookCreateDescriptorHeap(ID3D12Device* This, const D3D12_DESCRIPTOR_HEAP_DESC* desc, REFIID riid, void** out) {
    HRESULT hr = REAL(PfnCreateDescriptorHeap, 2)(This, desc, riid, out);
    if (SUCCEEDED(hr) && out && *out && heap_count < 64) {
        ID3D12DescriptorHeap* h = *out;
        Heap* heap = &heaps[heap_count++];
        heap->cpu = ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(h).ptr;
        heap->gpu = (desc->Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
                        ? ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(h).ptr : 0;
        heap->count = desc->NumDescriptors;
        heap->inc = ID3D12Device_GetDescriptorHandleIncrementSize(This, desc->Type);
        heap->views = calloc(desc->NumDescriptors, sizeof(View));
        fprintf(trace, "HEAP type=%d count=%u visible=%d\n", desc->Type, desc->NumDescriptors, heap->gpu != 0);
    }
    return hr;
}
