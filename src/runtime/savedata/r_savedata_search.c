/* SPDX-License-Identifier: MIT
 * PS4 SaveData Directory Search and Filter.
 * Single responsibility: SQL-LIKE pattern matching and directory scanning. (~70 LOC)
 */
#include "r_savedata_types.h"

static _Thread_local uint32_t s_sort_key, s_sort_order;

static int like(const char *s, const char *p) {
    if (!*p) return !*s;
    if (*p == '%') { for (;; ++s) { if (like(s, p + 1)) return 1; if (!*s) return 0; } }
    return *s && (*p == '_' || *p == *s) && like(s + 1, p + 1);
}

static int compare(const void *a, const void *b) {
    const Entry *x = a, *y = b;
    int c = s_sort_key == 1 ? (x->param.user_param > y->param.user_param) - (x->param.user_param < y->param.user_param)
          : s_sort_key == 3 ? (x->param.mtime > y->param.mtime) - (x->param.mtime < y->param.mtime)
          : strcmp(x->name, y->name);
    return s_sort_order ? -c : c;
}

ABI int32_t save_search(const SearchCond *cond, SearchResult *result) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!cond || !result) return ERR_PARAMETER;
    char base[600];
    r_savedata_root(cond->user, cond->title ? cond->title->data : NULL, base, sizeof(base));
    Entry *entries = NULL; size_t count = 0, capacity = 0;
    BbDir *d = bb_platform_dir_open(base);
    BbDirEntry e;
    while (d && bb_platform_dir_next(d, &e)) {
        size_t n = strlen(e.name);
        if (e.name[0] == '.' || n >= 32 || (n > 8 && !strcmp(e.name + n - 8, ".sce_sys"))) continue;
        if (cond->dir && cond->dir->data[0] && !like(e.name, cond->dir->data)) continue;
        if (count == capacity) { capacity = capacity ? capacity * 2 : 16; entries = (Entry*)realloc(entries, capacity * sizeof(*entries)); }
        snprintf(entries[count].name, 32, "%s", e.name);
        char meta[680]; snprintf(meta, sizeof(meta), "%s/%s.sce_sys", base, e.name);
        r_savedata_read_param(meta, &entries[count].param);
        ++count;
    }
    if (d) bb_platform_dir_close(d);
    s_sort_key = cond->key; s_sort_order = cond->order;
    if (count) qsort(entries, count, sizeof(*entries), compare);
    result->hits = (uint32_t)count;
    uint32_t set = count < result->names_capacity ? (uint32_t)count : result->names_capacity;
    for (uint32_t i = 0; i < set; ++i) {
        if (result->names) memcpy(result->names[i].data, entries[i].name, 32);
        if (result->params) result->params[i] = entries[i].param;
        if (result->infos) { memset(&result->infos[i], 0, sizeof(SearchInfo)); result->infos[i].blocks = 32768; result->infos[i].free_blocks = 16384; }
    }
    result->set_count = set;
    free(entries);
    return 0;
}
