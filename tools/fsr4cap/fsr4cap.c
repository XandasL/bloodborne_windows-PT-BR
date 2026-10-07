// SPDX-License-Identifier: GPL-2.0-or-later
#include "app/app_ffx.h"

int main(int argc, char** argv) {
    char log_path[MAX_PATH]; GetModuleFileNameA(NULL, log_path, MAX_PATH);
    strcpy(strrchr(log_path, '\\') + 1, "fsr4cap.log");
    freopen(log_path, "w", stdout); setvbuf(stdout, NULL, _IONBF, 0); *stderr = *stdout;
    const char* want = argc > 1 ? argv[1] : "FSR4";
    uint32_t rw = 1280, rh = 720, ow = 1920, oh = 1080;
    int frames = argc > 4 ? atoi(argv[4]) : 4;
    if (argc > 2) sscanf(argv[2], "%ux%u", &rw, &rh);
    if (argc > 3) sscanf(argv[3], "%ux%u", &ow, &oh);

    CHECK(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, (void**)&device));
    D3D12_COMMAND_QUEUE_DESC qd = {.Type = D3D12_COMMAND_LIST_TYPE_DIRECT};
    CHECK(ID3D12Device_CreateCommandQueue(device, &qd, &IID_ID3D12CommandQueue, (void**)&queue));
    CHECK(ID3D12Device_CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void**)&allocator));
    CHECK(ID3D12Device_CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL, &IID_ID3D12GraphicsCommandList, (void**)&list));
    CHECK(ID3D12Device_CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void**)&fence));
    fence_event = CreateEventA(NULL, FALSE, FALSE, NULL);

    char cap_dir[MAX_PATH]; GetModuleFileNameA(NULL, cap_dir, MAX_PATH);
    snprintf(strrchr(cap_dir, '\\') + 1, 64, "%scapture_%ux%u_%ux%u", (argc > 5 && !strcmp(argv[5], "noise")) ? "noise_" : "", rw, rh, ow, oh);
    CaptureInstall(device, list, cap_dir);

    HMODULE loader = LoadLibraryA("amd_fidelityfx_loader_dx12.dll");
    if (!loader) { fprintf(stderr, "no amd_fidelityfx_loader_dx12.dll\n"); return 1; }
    ffxFunctions ffx; ffxLoadFunctions(&ffx, loader);
    uint64_t chosen = PickVersion(&ffx, device, want);
    if (!chosen) { fprintf(stderr, "no version matching '%s'\n", want); return 1; }

    struct ffxCreateBackendDX12Desc backend = {.header = {.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12}, .device = device};
    struct ffxOverrideVersion override = {.header = {.type = FFX_API_DESC_TYPE_OVERRIDE_VERSION, .pNext = &backend.header}, .versionId = chosen};
    struct ffxCreateContextDescUpscale create = {.header = {.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE, .pNext = &override.header},
        .flags = FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE | FFX_UPSCALE_ENABLE_AUTO_EXPOSURE,
        .maxRenderSize = {ow, oh}, .maxUpscaleSize = {ow, oh}, .fpMessage = Message};
    ffxContext context = NULL;
    if (ffx.CreateContext(&context, &create.header, NULL) != FFX_API_RETURN_OK) return 1;

    ID3D12Resource *color = Texture(DXGI_FORMAT_R16G16B16A16_FLOAT, rw, rh, false);
    ID3D12Resource *depth = Texture(DXGI_FORMAT_R32_FLOAT, rw, rh, false);
    ID3D12Resource *motion = Texture(DXGI_FORMAT_R16G16_FLOAT, rw, rh, false);
    ID3D12Resource *output = Texture(DXGI_FORMAT_R16G16B16A16_FLOAT, ow, oh, true);
    CaptureNoteResource(color, "color"); CaptureNoteResource(depth, "depth"); CaptureNoteResource(motion, "motion"); CaptureNoteResource(output, "output");
    const bool noise = (argc > 5 && !strcmp(argv[5], "noise"));
    if (noise) { Upload(color, rw, rh, 8, FillColor); Upload(motion, rw, rh, 4, FillMotion); Upload(depth, rw, rh, 4, FillDepth); }

    for (int frame = 0; frame < frames; ++frame) {
        char mark[32]; snprintf(mark, sizeof(mark), "frame %d", frame); CaptureMark(mark);
        struct ffxDispatchDescUpscale d = {.header = {.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE}, .commandList = list,
            .color = Resource(color, FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT, FFX_API_RESOURCE_STATE_COMMON),
            .depth = Resource(depth, FFX_API_SURFACE_FORMAT_R32_FLOAT, FFX_API_RESOURCE_STATE_COMMON),
            .motionVectors = Resource(motion, FFX_API_SURFACE_FORMAT_R16G16_FLOAT, FFX_API_RESOURCE_STATE_COMMON),
            .output = Resource(output, FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS),
            .jitterOffset = {0.25f * (float)(frame % 4) - 0.375f, 0.125f}, .motionVectorScale = {1.f, 1.f},
            .renderSize = {rw, rh}, .upscaleSize = {ow, oh}, .enableSharpening = true, .sharpness = 0.5f,
            .frameTimeDelta = 10.f, .preExposure = 1.f, .reset = (frame == 0), .cameraNear = 0.05f, .cameraFar = 3000.f,
            .cameraFovAngleVertical = 0.75f, .viewSpaceToMetersFactor = 1.f};
        if (ffx.Dispatch(&context, &d.header) != FFX_API_RETURN_OK) return 1;
        Submit();
    }
    CaptureMark("end");
    if (noise) SaveOutputRaw(output, ow, oh);
    printf("%d frames done\n", frames);
    ffx.DestroyContext(&context, NULL);
    return 0;
}
