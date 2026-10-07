// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

typedef void (STDMETHODCALLTYPE* PfnCreateCBV)(ID3D12Device*, const D3D12_CONSTANT_BUFFER_VIEW_DESC*, D3D12_CPU_DESCRIPTOR_HANDLE);
static void STDMETHODCALLTYPE HookCreateCBV(ID3D12Device* This, const D3D12_CONSTANT_BUFFER_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE h) {
    REAL(PfnCreateCBV, 3)(This, desc, h);
    View* v = ViewAtCpu(h.ptr); if (!v || !desc) return;
    uint64_t off = 0;
    *v = (View){VIEW_CBV, ResByVa(desc->BufferLocation, &off), off, desc->SizeInBytes, 0, ""};
    snprintf(v->text, sizeof(v->text), "CBV r%d+%llu size %u", v->res ? v->res->id : -1, (unsigned long long)off, desc->SizeInBytes);
}

typedef void (STDMETHODCALLTYPE* PfnCreateSRV)(ID3D12Device*, ID3D12Resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC*, D3D12_CPU_DESCRIPTOR_HANDLE);
static void STDMETHODCALLTYPE HookCreateSRV(ID3D12Device* This, ID3D12Resource* res, const D3D12_SHADER_RESOURCE_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE h) {
    REAL(PfnCreateSRV, 4)(This, res, desc, h);
    View* v = ViewAtCpu(h.ptr); if (!v) return;
    ResInfo* r = Res(res);
    *v = (View){VIEW_SRV, r, 0, 0, desc ? desc->Format : -1, ""};
    if (desc && desc->ViewDimension == D3D12_SRV_DIMENSION_BUFFER)
        snprintf(v->text, sizeof(v->text), "SRV r%d buffer first=%llu num=%u stride=%u format=%d flags=%d", r ? r->id : -1,
                 (unsigned long long)desc->Buffer.FirstElement, desc->Buffer.NumElements, desc->Buffer.StructureByteStride, desc->Format, desc->Buffer.Flags);
    else snprintf(v->text, sizeof(v->text), "SRV r%d dim=%d format=%d", r ? r->id : -1, desc ? desc->ViewDimension : -1, desc ? desc->Format : -1);
}

typedef void (STDMETHODCALLTYPE* PfnCreateUAV)(ID3D12Device*, ID3D12Resource*, ID3D12Resource*, const D3D12_UNORDERED_ACCESS_VIEW_DESC*, D3D12_CPU_DESCRIPTOR_HANDLE);
static void STDMETHODCALLTYPE HookCreateUAV(ID3D12Device* This, ID3D12Resource* res, ID3D12Resource* counter, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE h) {
    REAL(PfnCreateUAV, 5)(This, res, counter, desc, h);
    View* v = ViewAtCpu(h.ptr); if (!v) return;
    ResInfo* r = Res(res);
    *v = (View){VIEW_UAV, r, 0, 0, desc ? desc->Format : -1, ""};
    if (desc && desc->ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
        snprintf(v->text, sizeof(v->text), "UAV r%d buffer first=%llu num=%u stride=%u format=%d flags=%d", r ? r->id : -1,
                 (unsigned long long)desc->Buffer.FirstElement, desc->Buffer.NumElements, desc->Buffer.StructureByteStride, desc->Format, desc->Buffer.Flags);
    else snprintf(v->text, sizeof(v->text), "UAV r%d dim=%d format=%d mip=%u", r ? r->id : -1, desc ? desc->ViewDimension : -1,
                  desc ? desc->Format : -1, desc && desc->ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2D ? desc->Texture2D.MipSlice : 0);
}

typedef void (STDMETHODCALLTYPE* PfnCreateSampler)(ID3D12Device*, const D3D12_SAMPLER_DESC*, D3D12_CPU_DESCRIPTOR_HANDLE);
static void STDMETHODCALLTYPE HookCreateSampler(ID3D12Device* This, const D3D12_SAMPLER_DESC* desc, D3D12_CPU_DESCRIPTOR_HANDLE h) {
    REAL(PfnCreateSampler, 6)(This, desc, h);
    View* v = ViewAtCpu(h.ptr); if (!v || !desc) return;
    *v = (View){VIEW_SAMPLER, NULL, 0, 0, 0, ""};
    snprintf(v->text, sizeof(v->text), "SAMPLER filter=0x%x address=%d,%d,%d", desc->Filter, desc->AddressU, desc->AddressV, desc->AddressW);
}

typedef void (STDMETHODCALLTYPE* PfnCopyDescriptorsSimple)(ID3D12Device*, UINT, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_DESCRIPTOR_HEAP_TYPE);
static void STDMETHODCALLTYPE HookCopyDescriptorsSimple(ID3D12Device* This, UINT n, D3D12_CPU_DESCRIPTOR_HANDLE dst, D3D12_CPU_DESCRIPTOR_HANDLE src, D3D12_DESCRIPTOR_HEAP_TYPE type) {
    REAL(PfnCopyDescriptorsSimple, 7)(This, n, dst, src, type);
    const UINT inc = ID3D12Device_GetDescriptorHandleIncrementSize(This, type);
    for (UINT i = 0; i < n; ++i) {
        View* s = ViewAtCpu(src.ptr + (SIZE_T)i * inc); View* d = ViewAtCpu(dst.ptr + (SIZE_T)i * inc);
        if (s && d) *d = *s;
    }
}

typedef void (STDMETHODCALLTYPE* PfnCopyDescriptors)(ID3D12Device*, UINT, const D3D12_CPU_DESCRIPTOR_HANDLE*, const UINT*, UINT, const D3D12_CPU_DESCRIPTOR_HANDLE*, const UINT*, D3D12_DESCRIPTOR_HEAP_TYPE);
static void STDMETHODCALLTYPE HookCopyDescriptors(ID3D12Device* This, UINT nd, const D3D12_CPU_DESCRIPTOR_HANDLE* dsts, const UINT* dsizes, UINT ns, const D3D12_CPU_DESCRIPTOR_HANDLE* srcs, const UINT* ssizes, D3D12_DESCRIPTOR_HEAP_TYPE type) {
    REAL(PfnCopyDescriptors, 8)(This, nd, dsts, dsizes, ns, srcs, ssizes, type);
    const UINT inc = ID3D12Device_GetDescriptorHandleIncrementSize(This, type);
    UINT di = 0, dk = 0;
    for (UINT si = 0; si < ns; ++si) {
        const UINT scount = ssizes ? ssizes[si] : 1;
        for (UINT k = 0; k < scount && di < nd; ++k) {
            View* s = ViewAtCpu(srcs[si].ptr + (SIZE_T)k * inc); View* d = ViewAtCpu(dsts[di].ptr + (SIZE_T)dk * inc);
            if (s && d) *d = *s;
            if (++dk == (dsizes ? dsizes[di] : 1)) { dk = 0; ++di; }
        }
    }
}
