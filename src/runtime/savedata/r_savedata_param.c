/* SPDX-License-Identifier: MIT
 * PS4 SaveData Parameters and Directory Deletion.
 * Single responsibility: Metadata read/write, icon export, and save deletion. (~70 LOC)
 */
#include "r_savedata_types.h"

void r_savedata_write_param(const char *meta, const Param *p) {
    char path[700]; snprintf(path, sizeof(path), "%s/param.bin", meta);
    FILE *f = fopen(path, "wb");
    if (f) { Param copy = *p; copy.mtime = time(NULL); fwrite(&copy, sizeof(copy), 1, f); fclose(f); }
}

int r_savedata_read_param(const char *meta, Param *p) {
    char path[700]; snprintf(path, sizeof(path), "%s/param.bin", meta);
    memset(p, 0, sizeof(*p));
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    size_t n = fread(p, sizeof(*p), 1, f); fclose(f);
    uint64_t sz, mtime; bool is_dir;
    if (!bb_platform_file_stat(path, &sz, &mtime, &is_dir)) p->mtime = (int64_t)mtime;
    return n == 1 ? 0 : -1;
}

ABI int32_t save_set_param(const MountPoint *point, uint32_t type, const void *buffer, uint64_t size) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!buffer) return ERR_PARAMETER;
    r_savedata_lock();
    int slot = r_savedata_slot_of(point);
    if (slot < 0) { r_savedata_unlock(); return ERR_NOT_MOUNTED; }
    Param p; r_savedata_read_param(g_savedata_slots[slot].meta, &p);
    switch (type) {
    case 0: if (size < sizeof(Param)) goto bad; memcpy(&p, buffer, sizeof(p)); break;
    case 1: snprintf(p.title, sizeof(p.title), "%.*s", (int)size, (const char*)buffer); break;
    case 2: snprintf(p.subtitle, sizeof(p.subtitle), "%.*s", (int)size, (const char*)buffer); break;
    case 3: snprintf(p.detail, sizeof(p.detail), "%.*s", (int)size, (const char*)buffer); break;
    case 4: if (size < 4) goto bad; memcpy(&p.user_param, buffer, 4); break;
    default: goto bad;
    }
    r_savedata_write_param(g_savedata_slots[slot].meta, &p);
    r_savedata_unlock();
    return 0;
bad:
    r_savedata_unlock();
    return ERR_PARAMETER;
}

ABI int32_t save_icon(const MountPoint *point, const Icon *icon) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!icon || !icon->buffer) return ERR_PARAMETER;
    r_savedata_lock();
    int slot = r_savedata_slot_of(point);
    int32_t r = ERR_NOT_MOUNTED;
    if (slot >= 0) {
        char path[700]; snprintf(path, sizeof(path), "%s/icon0.png", g_savedata_slots[slot].meta);
        FILE *f = fopen(path, "wb");
        r = f && fwrite(icon->buffer, 1, icon->data_size, f) == icon->data_size ? 0 : ERR_INTERNAL;
        if (f) fclose(f);
    }
    r_savedata_unlock();
    return r;
}

ABI int32_t save_delete(const Delete *d) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!d || !d->dir || !r_savedata_valid_name(d->dir->data, sizeof(d->dir->data))) return ERR_PARAMETER;
    char base[600], host[640], meta[680];
    r_savedata_root(d->user, d->title ? d->title->data : NULL, base, sizeof(base));
    snprintf(host, sizeof(host), "%s/%s", base, d->dir->data);
    snprintf(meta, sizeof(meta), "%s/%s.sce_sys", base, d->dir->data);
    uint64_t dummy_sz, dummy_mt; bool is_dir;
    if (bb_platform_file_stat(host, &dummy_sz, &dummy_mt, &is_dir) != 0) return ERR_NOT_FOUND;
    bb_platform_remove(host);
    bb_platform_remove(meta);
    return 0;
}
