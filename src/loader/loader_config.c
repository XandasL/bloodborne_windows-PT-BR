/* SPDX-License-Identifier: MIT
 * PS4 Loader App and Content Configuration.
 * Single responsibility: Parsing content profile and configuring file/save/GPU subsystems. (~50 LOC)
 */
#include "loader_types.h"

void loader_configure_content(const char *path) {
    if (!path) return;
    FILE *profile = fopen(path, "rb");
    unsigned char data[28];
    if (!profile) fail("cannot open content profile");
    if (fread(data, 1, sizeof(data), profile) != sizeof(data) || fgetc(profile) != EOF || memcmp(data, "BBCONT01", 8))
        fail("invalid content profile");
    fclose(profile);
    uint32_t values[5];
    for (unsigned i = 0; i < 5; ++i) {
        unsigned char *p = data + 8 + i * 4;
        values[i] = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }
    runtime_content_configure(values);
}

void loader_configure_app(const char *app0, const char *user_dir) {
    if (!app0) return;
    runtime_file_configure(app0, user_dir ? user_dir : "user");
    char sfo[4096], id[16] = "";
    snprintf(sfo, sizeof(sfo), "%s/sce_sys/param.sfo", app0);
    if (sfo_value(sfo, "INSTALL_DIR_SAVEDATA", id, sizeof(id), NULL) ||
        sfo_value(sfo, "TITLE_ID", id, sizeof(id), NULL))
        runtime_savedata_configure(id);
}

void loader_init_gpu(const char *app0, const char *user_dir, uint64_t sdk, uint32_t attributes) {
    char title[128] = "Bloodborne", serial[16] = "UNKNOWN", sfo[4096];
    snprintf(sfo, sizeof(sfo), "%s/sce_sys/param.sfo", app0 ? app0 : ".");
    sfo_value(sfo, "TITLE", title, sizeof(title), NULL);
    sfo_value(sfo, "TITLE_ID", serial, sizeof(serial), NULL);
    sfo_value(sfo, "ATTRIBUTE", NULL, 0, &attributes);
    BbGpuConfig gpu = {title, serial, user_dir ? user_dir : "user", (uint32_t)sdk, attributes, 1920, 1080};
    if (bbgpu_init(&gpu)) fail("GPU initialization failed");
    printf("GPU: window and Vulkan presenter ready; SDK 0x%08x, %u HLE symbols\n", (unsigned)sdk, bbgpu_symbol_count());
}
