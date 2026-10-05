/* SPDX-License-Identifier: MIT
 * PS4 SaveData Subsystem Types and Internal Signatures.
 * Single responsibility: SaveData structure definitions and prototypes. (~70 LOC)
 */
#ifndef R_SAVEDATA_TYPES_H
#define R_SAVEDATA_TYPES_H

#include "runtime.h"
#include "platform/bb_common.h"
#include "platform/sync.h"
#include "platform/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#define ERR_PARAMETER ((int32_t)0x809F0000)
#define ERR_NOT_INITIALIZED ((int32_t)0x809F0001)
#define ERR_NOT_MOUNTED ((int32_t)0x809F0004)
#define ERR_EXISTS ((int32_t)0x809F0007)
#define ERR_NOT_FOUND ((int32_t)0x809F0008)
#define ERR_INTERNAL ((int32_t)0x809F000B)
#define ERR_MOUNT_FULL ((int32_t)0x809F000C)
#define ERR_BAD_MOUNTED ((int32_t)0x809F000D)
#define ERR_INVALID_USER ((int32_t)0x809F0011)
#define ERR_MEMORY_NOT_READY ((int32_t)0x809F0012)
#define MODE_RDONLY 1
#define MODE_CREATE 4
#define MODE_COPY_ICON 16
#define MODE_CREATE2 32
#define SLOTS 16

typedef struct { char data[10]; char pad[6]; } TitleId;
typedef struct { char data[32]; } DirName;
typedef struct { char data[16]; } MountPoint;
typedef struct { char title[128], subtitle[128], detail[1024]; uint32_t user_param; int32_t pad; int64_t mtime; uint8_t reserved[32]; } Param;
typedef struct { int32_t user, pad; const TitleId *title; const DirName *dir; const void *fingerprint; uint64_t blocks; uint32_t mode; uint8_t reserved[32]; } Mount1;
typedef struct { int32_t user, pad; const DirName *dir; uint64_t blocks; uint32_t mode; uint8_t reserved[32]; int32_t pad2; } Mount2;
typedef struct { MountPoint point; uint64_t required_blocks; uint32_t unused, status; uint8_t reserved[28]; int32_t pad; } MountResult;
typedef struct { int32_t user, pad; const TitleId *title; const DirName *dir; uint32_t unused; uint8_t reserved[32]; int32_t pad2; } Delete;
typedef struct { int32_t user, pad; const TitleId *title; const DirName *dir; uint32_t key, order; uint8_t reserved[32]; } SearchCond;
typedef struct { uint64_t blocks, free_blocks; uint8_t reserved[32]; } SearchInfo;
typedef struct { uint32_t hits; int32_t pad; DirName *names; uint32_t names_capacity, set_count; Param *params; SearchInfo *infos; uint8_t reserved[12]; int32_t pad2; } SearchResult;
typedef struct { const void *buffer; uint64_t buffer_size, data_size; uint8_t reserved[32]; } Icon;
typedef struct { char name[32]; Param param; } Entry;
typedef struct { int used; char host[700], meta[700]; } SaveSlot;

void r_savedata_lock(void);
void r_savedata_unlock(void);
int r_savedata_make_dirs(const char *path);
void r_savedata_root(int32_t user, const char *title, char *out, size_t size);
int r_savedata_valid_name(const char *name, size_t max);
void r_savedata_write_param(const char *meta, const Param *p);
int r_savedata_read_param(const char *meta, Param *p);
int r_savedata_slot_of(const MountPoint *point);

extern int g_savedata_initialized;
extern char g_savedata_title_id[16];
extern SaveSlot g_savedata_slots[SLOTS];
extern size_t g_savedata_mounts_done, g_savedata_memory_writes;

#endif /* R_SAVEDATA_TYPES_H */
