/* SPDX-License-Identifier: MIT
 * PS4 SaveData NID Exports and Status Reporting.
 * Single responsibility: Export resolution and diagnostics. (~50 LOC)
 */
#include "r_savedata_types.h"

ABI int32_t save_initialize(const void *param);
ABI int32_t save_terminate(void);
ABI int32_t save_mount(const Mount1 *m, MountResult *result);
ABI int32_t save_mount2(const Mount2 *m, MountResult *result);
ABI int32_t save_umount(const MountPoint *point);
ABI int32_t save_set_param(const MountPoint *point, uint32_t type, const void *buffer, uint64_t size);
ABI int32_t save_icon(const MountPoint *point, const Icon *icon);
ABI int32_t save_delete(const Delete *d);
ABI int32_t save_search(const SearchCond *cond, SearchResult *result);
ABI int32_t memory_setup(int32_t user, uint64_t size, const Param *param);
ABI int32_t memory_get(int32_t user, void *buffer, uint64_t size, int64_t offset);
ABI int32_t memory_set(int32_t user, void *buffer, uint64_t size, int64_t offset);

static const RuntimeExport exports[] = {
    {"sceSaveDataInitialize", (void*)save_initialize}, {"sceSaveDataInitialize2", (void*)save_initialize},
    {"sceSaveDataInitialize3", (void*)save_initialize}, {"sceSaveDataTerminate", (void*)save_terminate},
    {"sceSaveDataMount", (void*)save_mount}, {"sceSaveDataMount2", (void*)save_mount2}, {"sceSaveDataUmount", (void*)save_umount},
    {"sceSaveDataSetParam", (void*)save_set_param}, {"sceSaveDataSaveIcon", (void*)save_icon}, {"sceSaveDataDelete", (void*)save_delete},
    {"sceSaveDataDirNameSearch", (void*)save_search},
    {"sceSaveDataSetupSaveDataMemory", (void*)memory_setup}, {"sceSaveDataGetSaveDataMemory", (void*)memory_get},
    {"sceSaveDataSetSaveDataMemory", (void*)memory_set},
};

uintptr_t runtime_savedata_resolve(const char *name) {
    return RUNTIME_LOOKUP(exports, name);
}

void runtime_savedata_report(void) {
    printf("Runtime: save data mounts=%zu, memory writes=%zu\n", g_savedata_mounts_done, g_savedata_memory_writes);
}
