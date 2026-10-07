// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

void CaptureInstall(ID3D12Device* device, ID3D12GraphicsCommandList* list, const char* directory) {
    snprintf(dir, sizeof(dir), "%s", directory);
    CreateDirectoryA(dir, NULL);
    char path[MAX_PATH]; snprintf(path, sizeof(path), "%s\\trace.txt", dir);
    trace = fopen(path, "w"); setvbuf(trace, NULL, _IOFBF, 1 << 20);
    device_vtbl = *(void***)device; list_vtbl = *(void***)list;

    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateComputePipelineState), HookCreateComputePipelineState, 0);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateRootSignature), HookCreateRootSignature, 1);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateDescriptorHeap), HookCreateDescriptorHeap, 2);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateConstantBufferView), HookCreateCBV, 3);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateShaderResourceView), HookCreateSRV, 4);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateUnorderedAccessView), HookCreateUAV, 5);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CreateSampler), HookCreateSampler, 6);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CopyDescriptorsSimple), HookCopyDescriptorsSimple, 7);
    Patch(device_vtbl, SLOT(ID3D12DeviceVtbl, CopyDescriptors), HookCopyDescriptors, 8);

    D3D12_HEAP_PROPERTIES heap = {.Type = D3D12_HEAP_TYPE_UPLOAD};
    D3D12_RESOURCE_DESC desc = {.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER, .Width = 256, .Height = 1,
                                .DepthOrArraySize = 1, .MipLevels = 1, .SampleDesc = {1, 0}, .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR};
    ID3D12Resource* probe = NULL;
    if (SUCCEEDED(ID3D12Device_CreateCommittedResource(device, &heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&probe))) {
        void** vtbl = *(void***)probe;
        Patch(vtbl, SLOT(ID3D12ResourceVtbl, GetGPUVirtualAddress), HookGetGpuVa, 300);
        real_get_va = real[300];
    }

    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRootSignature), HookSetComputeRootSignature, 100);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetPipelineState), HookSetPipelineState, 101);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRootDescriptorTable), HookSetComputeRootDescriptorTable, 102);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRoot32BitConstant), HookSetComputeRoot32BitConstant, 103);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRoot32BitConstants), HookSetComputeRoot32BitConstants, 104);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRootConstantBufferView), HookSetComputeRootCBV, 105);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRootShaderResourceView), HookSetComputeRootSRV, 106);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, SetComputeRootUnorderedAccessView), HookSetComputeRootUAV, 107);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, Dispatch), HookDispatch, 108);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, CopyBufferRegion), HookCopyBufferRegion, 109);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, CopyResource), HookCopyResource, 110);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, CopyTextureRegion), HookCopyTextureRegion, 111);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, ClearUnorderedAccessViewFloat), HookClearUavFloat, 112);
    Patch(list_vtbl, SLOT(ID3D12GraphicsCommandListVtbl, ClearUnorderedAccessViewUint), HookClearUavUint, 113);
}

void CaptureMark(const char* text) {
    if (trace) { fprintf(trace, "MARK %s\n", text); fflush(trace); }
}

void CaptureNoteResource(ID3D12Resource* r, const char* name) {
    ResInfo* info = Res(r); if (info) fprintf(trace, "NAME r%d %s\n", info->id, name);
}
