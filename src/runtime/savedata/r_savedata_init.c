/* SPDX-License-Identifier: MIT
 * PS4 SaveData Subsystem Initialization and Sound Flag Patch.
 * Single responsibility: Subsystem state, configuration, and title audio fix. (~70 LOC)
 */
#include "r_savedata_types.h"

int g_savedata_initialized = 0;
char g_savedata_title_id[16] = "UNKNOWN";
SaveSlot g_savedata_slots[SLOTS];
size_t g_savedata_mounts_done = 0, g_savedata_memory_writes = 0;
static BbMutex *save_lock;

static void ensure_lock(void) {
    if (!save_lock) {
        BbMutex *m = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
        if (!__sync_bool_compare_and_swap(&save_lock, NULL, m)) bb_platform_mutex_destroy(m);
    }
}
void r_savedata_lock(void) { ensure_lock(); bb_platform_mutex_lock(save_lock); }
void r_savedata_unlock(void) { if (save_lock) bb_platform_mutex_unlock(save_lock); }

static void bloodborne_sound_hack(void) {
    static const char *const ids[] = {"CUSA00207", "CUSA00208", "CUSA00299", "CUSA00900", "CUSA01363", "CUSA03014", "CUSA03023", "CUSA03173"};
    const char *env = getenv("BB_SOUND_HACK");
    if (env && env[0] == '0') return;
    int bloodborne = 0;
    for (size_t i = 0; i < sizeof(ids) / sizeof(*ids); ++i)
        if (!strcmp(g_savedata_title_id, ids[i])) bloodborne = 1;
    if (!bloodborne) return;
    char users[700]; snprintf(users, sizeof(users), "%s/savedata", runtime_file_user_dir());
    BbDir *d = bb_platform_dir_open(users);
    if (!d) return;
    BbDirEntry entry;
    while (bb_platform_dir_next(d, &entry)) {
        if (entry.name[0] == '.') continue;
        char path[1100];
        snprintf(path, sizeof(path), "%s/%s/%s/SPRJ0005/userdata0010", users, entry.name, g_savedata_title_id);
        FILE *f = fopen(path, "r+b");
        if (!f) continue;
        int old = fseek(f, 0x204E, SEEK_SET) ? EOF : fgetc(f);
        if (old != EOF && old != 1 && !fseek(f, 0x204E, SEEK_SET)) {
            fputc(1, f);
            printf("Runtime: Bloodborne sound flag set in %s (was %d)\n", path, old);
        }
        fclose(f);
    }
    bb_platform_dir_close(d);
}

void runtime_savedata_configure(const char *title) {
    if (title && *title) snprintf(g_savedata_title_id, sizeof(g_savedata_title_id), "%s", title);
    bloodborne_sound_hack();
}

ABI int32_t save_initialize(const void *param) { (void)param; g_savedata_initialized = 1; return 0; }
ABI int32_t save_terminate(void) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    g_savedata_initialized = 0; return 0;
}
