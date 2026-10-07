// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

typedef void (STDMETHODCALLTYPE* PfnCopyBufferRegion)(ID3D12GraphicsCommandList*, ID3D12Resource*, UINT64, ID3D12Resource*, UINT64, UINT64);
static void STDMETHODCALLTYPE HookCopyBufferRegion(ID3D12GraphicsCommandList* This, ID3D12Resource* dst, UINT64 doff, ID3D12Resource* src, UINT64 soff, UINT64 n) {
    ResInfo* d = Res(dst); ResInfo* s = Res(src);
    fprintf(trace, "COPYBUFFER r%d+%llu <- r%d+%llu size %llu", d ? d->id : -1, (unsigned long long)doff,
            s ? s->id : -1, (unsigned long long)soff, (unsigned long long)n);
    DumpBytes("data", s, soff, n); fprintf(trace, "\n");
    REAL(PfnCopyBufferRegion, 109)(This, dst, doff, src, soff, n);
}

typedef void (STDMETHODCALLTYPE* PfnCopyResource)(ID3D12GraphicsCommandList*, ID3D12Resource*, ID3D12Resource*);
static void STDMETHODCALLTYPE HookCopyResource(ID3D12GraphicsCommandList* This, ID3D12Resource* dst, ID3D12Resource* src) {
    ResInfo* d = Res(dst); ResInfo* s = Res(src);
    fprintf(trace, "COPYRESOURCE r%d <- r%d", d ? d->id : -1, s ? s->id : -1);
    if (s && s->desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) DumpBytes("data", s, 0, s->desc.Width);
    fprintf(trace, "\n");
    REAL(PfnCopyResource, 110)(This, dst, src);
}

typedef void (STDMETHODCALLTYPE* PfnCopyTextureRegion)(ID3D12GraphicsCommandList*, const D3D12_TEXTURE_COPY_LOCATION*, UINT, UINT, UINT, const D3D12_TEXTURE_COPY_LOCATION*, const D3D12_BOX*);
static void STDMETHODCALLTYPE HookCopyTextureRegion(ID3D12GraphicsCommandList* This, const D3D12_TEXTURE_COPY_LOCATION* dst, UINT x, UINT y, UINT z, const D3D12_TEXTURE_COPY_LOCATION* src, const D3D12_BOX* box) {
    ResInfo* d = Res(dst->pResource); ResInfo* s = Res(src->pResource);
    fprintf(trace, "COPYTEXTURE r%d (type %d) <- r%d (type %d)", d ? d->id : -1, dst->Type, s ? s->id : -1, src->Type);
    if (src->Type == D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT && s) {
        const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* f = &src->PlacedFootprint;
        fprintf(trace, " footprint offset %llu %ux%u format %d pitch %u", (unsigned long long)f->Offset,
                f->Footprint.Width, f->Footprint.Height, f->Footprint.Format, f->Footprint.RowPitch);
        DumpBytes("data", s, f->Offset, (uint64_t)f->Footprint.RowPitch * f->Footprint.Height);
    }
    fprintf(trace, "\n");
    REAL(PfnCopyTextureRegion, 111)(This, dst, x, y, z, src, box);
}

typedef void (STDMETHODCALLTYPE* PfnClearUavFloat)(ID3D12GraphicsCommandList*, D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*, const FLOAT[4], UINT, const D3D12_RECT*);
static void STDMETHODCALLTYPE HookClearUavFloat(ID3D12GraphicsCommandList* This, D3D12_GPU_DESCRIPTOR_HANDLE g, D3D12_CPU_DESCRIPTOR_HANDLE c, ID3D12Resource* r, const FLOAT v[4], UINT n, const D3D12_RECT* rects) {
    ResInfo* ri = Res(r);
    fprintf(trace, "CLEARUAVFLOAT r%d %g %g %g %g\n", ri ? ri->id : -1, v[0], v[1], v[2], v[3]);
    REAL(PfnClearUavFloat, 112)(This, g, c, r, v, n, rects);
}

typedef void (STDMETHODCALLTYPE* PfnClearUavUint)(ID3D12GraphicsCommandList*, D3D12_GPU_DESCRIPTOR_HANDLE, D3D12_CPU_DESCRIPTOR_HANDLE, ID3D12Resource*, const UINT[4], UINT, const D3D12_RECT*);
static void STDMETHODCALLTYPE HookClearUavUint(ID3D12GraphicsCommandList* This, D3D12_GPU_DESCRIPTOR_HANDLE g, D3D12_CPU_DESCRIPTOR_HANDLE c, ID3D12Resource* r, const UINT v[4], UINT n, const D3D12_RECT* rects) {
    ResInfo* ri = Res(r);
    fprintf(trace, "CLEARUAVUINT r%d %u %u %u %u\n", ri ? ri->id : -1, v[0], v[1], v[2], v[3]);
    REAL(PfnClearUavUint, 113)(This, g, c, r, v, n, rects);
}

static D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE HookGetGpuVa(ID3D12Resource* This) {
    Res(This);
    return ((PfnGetGpuVa)real_get_va)(This);
}
