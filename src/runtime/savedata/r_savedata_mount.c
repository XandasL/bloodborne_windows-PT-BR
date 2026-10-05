/* SPDX-License-Identifier: MIT
 * PS4 SaveData Mounting and Virtual Mount Points.
 * Single responsibility: Mounting host directories as /savedata0..15. (~70 LOC)
 */
#include "r_savedata_types.h"

int r_savedata_make_dirs(const char *path) {
    char buf[700]; snprintf(buf, sizeof(buf), "%s", path);
    for (char *p = buf + 1; *p; ++p)
        if (*p == '/' || *p == '\\') { char c = *p; *p = 0; bb_platform_mkdir(buf); *p = c; }
    return bb_platform_mkdir(buf);
}

void r_savedata_root(int32_t user, const char *title, char *out, size_t size) {
    snprintf(out, size, "%s/savedata/%d/%s", runtime_file_user_dir(), user, title && *title ? title : g_savedata_title_id);
}

int r_savedata_valid_name(const char *name, size_t max) {
    size_t n = strnlen(name, max);
    if (!n || n == max) return 0;
    for (size_t i = 0; i < n; ++i) if (name[i] == '/' || name[i] == '\\' || (name[i] == '.' && (i == 0 || name[i-1] == '.'))) return 0;
    return 1;
}

int r_savedata_slot_of(const MountPoint *point) {
    if (!point || strncmp(point->data, "/savedata", 9)) return -1;
    int slot = atoi(point->data + 9);
    return slot >= 0 && slot < SLOTS && g_savedata_slots[slot].used ? slot : -1;
}

static int32_t mount_impl(int32_t user, const char *title, const DirName *dir, uint32_t mode, MountResult *result) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (user < 0) return ERR_INVALID_USER;
    if (!dir || !result || !r_savedata_valid_name(dir->data, sizeof(dir->data))) return ERR_PARAMETER;
    char base[600], host[640], meta[680];
    r_savedata_root(user, title, base, sizeof(base));
    snprintf(host, sizeof(host), "%s/%s", base, dir->data);
    snprintf(meta, sizeof(meta), "%s/%s.sce_sys", base, dir->data);
    uint64_t dummy_sz, dummy_mt; bool is_dir = false;
    int exists = (bb_platform_file_stat(host, &dummy_sz, &dummy_mt, &is_dir) == 0 && is_dir);
    if ((mode & MODE_CREATE) && exists) return ERR_EXISTS;
    if (!(mode & (MODE_CREATE | MODE_CREATE2)) && !exists) return ERR_NOT_FOUND;
    r_savedata_lock();
    int slot = -1;
    for (int i = 0; i < SLOTS; ++i) {
        if (g_savedata_slots[i].used && !strcmp(g_savedata_slots[i].host, host)) { r_savedata_unlock(); return ERR_BAD_MOUNTED; }
        if (!g_savedata_slots[i].used && slot < 0) slot = i;
    }
    if (slot < 0) { r_savedata_unlock(); return ERR_MOUNT_FULL; }
    if (!exists && (r_savedata_make_dirs(host) || r_savedata_make_dirs(meta))) { r_savedata_unlock(); return ERR_INTERNAL; }
    if (!exists) { Param empty = {0}; r_savedata_write_param(meta, &empty); }
    g_savedata_slots[slot].used = 1;
    snprintf(g_savedata_slots[slot].host, sizeof(g_savedata_slots[slot].host), "%s", host);
    snprintf(g_savedata_slots[slot].meta, sizeof(g_savedata_slots[slot].meta), "%s", meta);
    memset(result, 0, sizeof(*result));
    snprintf(result->point.data, sizeof(result->point.data), "/savedata%d", slot & 15);
    result->status = exists ? 0 : 1;
    runtime_file_mount(result->point.data, host);
    ++g_savedata_mounts_done;
    r_savedata_unlock();
    return 0;
}

ABI int32_t save_mount(const Mount1 *m, MountResult *res) { return m ? mount_impl(m->user, m->title ? m->title->data : NULL, m->dir, m->mode, res) : ERR_PARAMETER; }
ABI int32_t save_mount2(const Mount2 *m, MountResult *res) { return m ? mount_impl(m->user, NULL, m->dir, m->mode, res) : ERR_PARAMETER; }
ABI int32_t save_umount(const MountPoint *pt) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    r_savedata_lock();
    int s = r_savedata_slot_of(pt);
    if (s >= 0) { runtime_file_unmount(pt->data); g_savedata_slots[s].used = 0; }
    r_savedata_unlock();
    return s >= 0 ? 0 : ERR_NOT_FOUND;
}
