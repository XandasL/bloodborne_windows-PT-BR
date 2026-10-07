/* SPDX-License-Identifier: MIT
 * PS4 Loader Main Entry and Program Orchestration.
 * Single responsibility: Command-line parsing, initialization, and game kickoff. (~50 LOC)
 */
#include "loader_types.h"

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc == 2 && !strcmp(argv[1], "--vulkan-only")) return vulkan_smoke();
    if (argc < 2) {
        fprintf(stderr, "Usage: %s boot.bin [--cpu-only] [--strict-imports] "
                        "[--content-profile file] [--app0 dir] [--user dir] "
                        "[--patches file] [--timeout seconds] | --vulkan-only\n", argv[0]);
        return 1;
    }
    int cpu_only = 0, strict_imports = 0;
    const char *content_profile = NULL, *app0 = NULL, *user_dir = NULL, *patch_file = NULL;
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--cpu-only")) cpu_only = 1;
        else if (!strcmp(argv[i], "--strict-imports")) strict_imports = 1;
        else if (!strcmp(argv[i], "--content-profile") && i + 1 < argc) content_profile = argv[++i];
        else if (!strcmp(argv[i], "--app0") && i + 1 < argc) app0 = argv[++i];
        else if (!strcmp(argv[i], "--user") && i + 1 < argc) user_dir = argv[++i];
        else if (!strcmp(argv[i], "--patches") && i + 1 < argc) patch_file = argv[++i];
        else if (!strcmp(argv[i], "--timeout") && i + 1 < argc) ++i;
        else { fprintf(stderr, "Unknown option: %s\n", argv[i]); return 1; }
    }
    puts("BOOT: configure AppContent");
    loader_configure_content(content_profile);
    puts("BOOT: configure app mounts");
    loader_configure_app(app0, user_dir);
    puts("BOOT: register GPU kernel");
    bbgpu_register_kernel();
    puts("BOOT: initialize fault handling");
    bb_platform_faults_init();
    puts("BOOT: open prepared boot image");
    FILE *f = fopen(argv[1], "rb");
    if (!f) fail("cannot open boot file; run prepare.py first");
    char magic[8];
    if (fread(magic, 1, 8, f) != 8 || (memcmp(magic, "BBPROBE1", 8) && memcmp(magic, "BBPROBE2", 8) &&
        memcmp(magic, "BBPROBE3", 8) && memcmp(magic, "BBPROBE4", 8) && memcmp(magic, "BBPROBE5", 8)))
        fail("bad boot file signature");
    uint64_t entry = 0;
    puts("BOOT: load prepared image");
    loader_load_boot(f, magic, &entry, patch_file);
    fclose(f);
    if (!cpu_only) {
        puts("BOOT: initialize Vulkan/GPU");
        loader_init_gpu(app0, user_dir, 0, 0);
    }
    printf("Entering original x86-64 code at guest offset 0x%" PRIx64 "\n", entry);
    struct { uint64_t argc; const char *argv[2]; } params = {1, {"/app0/eboot.bin", NULL}};
    enter_guest(g_loader_image + entry, &params, NULL);
    return 0;
}
