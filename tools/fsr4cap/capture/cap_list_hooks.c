// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

typedef void (STDMETHODCALLTYPE* PfnSetPtr)(ID3D12GraphicsCommandList*, void*);
static void STDMETHODCALLTYPE HookSetComputeRootSignature(ID3D12GraphicsCommandList* This, void* rs) {
    State(This)->root = rs; memset(State(This)->args, 0, sizeof(State(This)->args));
    REAL(PfnSetPtr, 100)(This, rs);
}
static void STDMETHODCALLTYPE HookSetPipelineState(ID3D12GraphicsCommandList* This, void* pso) {
    State(This)->pso = pso; REAL(PfnSetPtr, 101)(This, pso);
}
typedef void (STDMETHODCALLTYPE* PfnSetTable)(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE);
static void STDMETHODCALLTYPE HookSetComputeRootDescriptorTable(ID3D12GraphicsCommandList* This, UINT i, D3D12_GPU_DESCRIPTOR_HANDLE h) {
    if (i < 32) State(This)->args[i] = (RootArg){ARG_TABLE, h.ptr};
    REAL(PfnSetTable, 102)(This, i, h);
}
typedef void (STDMETHODCALLTYPE* PfnSet32)(ID3D12GraphicsCommandList*, UINT, UINT, UINT);
static void STDMETHODCALLTYPE HookSetComputeRoot32BitConstant(ID3D12GraphicsCommandList* This, UINT i, UINT v, UINT off) {
    if (i < 32 && off < 64) {
        RootArg* a = &State(This)->args[i]; a->type = ARG_CONSTS; a->consts[off] = v;
        if (off + 1 > a->nconsts) a->nconsts = off + 1;
    }
    REAL(PfnSet32, 103)(This, i, v, off);
}
typedef void (STDMETHODCALLTYPE* PfnSet32s)(ID3D12GraphicsCommandList*, UINT, UINT, const void*, UINT);
static void STDMETHODCALLTYPE HookSetComputeRoot32BitConstants(ID3D12GraphicsCommandList* This, UINT i, UINT n, const void* v, UINT off) {
    if (i < 32 && off + n <= 64) {
        RootArg* a = &State(This)->args[i]; a->type = ARG_CONSTS; memcpy(&a->consts[off], v, n * 4);
        if (off + n > a->nconsts) a->nconsts = off + n;
    }
    REAL(PfnSet32s, 104)(This, i, n, v, off);
}
typedef void (STDMETHODCALLTYPE* PfnSetVa)(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_VIRTUAL_ADDRESS);
static void STDMETHODCALLTYPE HookSetComputeRootCBV(ID3D12GraphicsCommandList* This, UINT i, D3D12_GPU_VIRTUAL_ADDRESS va) {
    if (i < 32) State(This)->args[i] = (RootArg){ARG_CBV, va}; REAL(PfnSetVa, 105)(This, i, va);
}
static void STDMETHODCALLTYPE HookSetComputeRootSRV(ID3D12GraphicsCommandList* This, UINT i, D3D12_GPU_VIRTUAL_ADDRESS va) {
    if (i < 32) State(This)->args[i] = (RootArg){ARG_SRV, va}; REAL(PfnSetVa, 106)(This, i, va);
}
static void STDMETHODCALLTYPE HookSetComputeRootUAV(ID3D12GraphicsCommandList* This, UINT i, D3D12_GPU_VIRTUAL_ADDRESS va) {
    if (i < 32) State(This)->args[i] = (RootArg){ARG_UAV, va}; REAL(PfnSetVa, 107)(This, i, va);
}

typedef void (STDMETHODCALLTYPE* PfnDispatch)(ID3D12GraphicsCommandList*, UINT, UINT, UINT);
static void STDMETHODCALLTYPE HookDispatch(ID3D12GraphicsCommandList* This, UINT x, UINT y, UINT z) {
    ListState* s = State(This);
    fprintf(trace, "DISPATCH #%d cs=%016llx root=%016llx groups=%u,%u,%u\n", dispatch_count++,
            (unsigned long long)PsoHash(s->pso), (unsigned long long)RootHash(s->root), x, y, z);
    for (int i = 0; i < 32; ++i) {
        RootArg* a = &s->args[i]; if (a->type == ARG_NONE) continue;
        fprintf(trace, "  param %d:", i);
        if (a->type == ARG_CONSTS) { for (UINT k = 0; k < a->nconsts; ++k) fprintf(trace, " %08x", a->consts[k]); }
        else if (a->type == ARG_TABLE) {
            Heap* heap = NULL; View* v = ViewAtGpu(a->value, &heap);
            fprintf(trace, " table@%llu", heap ? (unsigned long long)((a->value - heap->gpu) / heap->inc) : 0ull);
            for (int k = 0; v && k < 24 && (v + k) < heap->views + heap->count; ++k) {
                if (v[k].kind == VIEW_NONE) continue;
                fprintf(trace, "\n    [%d] %s", k, v[k].text);
                if (v[k].kind == VIEW_CBV) DumpBytes("data", v[k].res, v[k].a, v[k].b);
            }
        } else {
            uint64_t off = 0; ResInfo* r = ResByVa(a->value, &off);
            fprintf(trace, " %s r%d+%llu", a->type == ARG_CBV ? "CBV" : a->type == ARG_SRV ? "SRV" : "UAV", r ? r->id : -1, (unsigned long long)off);
            if (a->type == ARG_CBV && r) { const uint64_t sz = r->desc.Width - off < 4096 ? r->desc.Width - off : 4096; DumpBytes("data", r, off, sz); }
        }
        fprintf(trace, "\n");
    }
    REAL(PfnDispatch, 108)(This, x, y, z);
}
