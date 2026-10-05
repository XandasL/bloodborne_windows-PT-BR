/* SPDX-License-Identifier: MIT
 * PS4 HLE path translation and mount prefix resolution.
 * Single responsibility: Guest path normalization & mount mapping. (~70 LOC)
 */
#include "r_fs_types.h"
#include <string.h>
#include <stdio.h>

size_t r_fs_get_mounts(Mount** out);

int r_fs_is_game_path(const char* p) {
    if (!p || !*p) return 0;
    if (*p != '/') return 1;
    return (!strncmp(p, "/app0", 5) && (!p[5] || p[5] == '/')) ||
           (!strncmp(p, "/hostapp", 8) && (!p[8] || p[8] == '/'));
}

int r_fs_translate(const char* guest, char* out, size_t size) {
    if (!guest || !*guest) return 2; /* ENOENT */
    char buffer[1024];
    if (guest[0] != '/') snprintf(buffer, sizeof(buffer), "/app0/%s", guest);
    else snprintf(buffer, sizeof(buffer), "%s", guest);

    /* Guard against directory traversal attacks */
    for (const char* p = buffer; (p = strstr(p, "..")); p += 2) {
        if ((p == buffer || p[-1] == '/' || p[-1] == '\\') &&
            (p[2] == 0 || p[2] == '/' || p[2] == '\\')) {
            return 13; /* EACCES */
        }
    }

    Mount* mounts = NULL;
    size_t count = r_fs_get_mounts(&mounts);
    size_t best = 0;
    const Mount* m = NULL;

    for (size_t i = 0; i < count; ++i) {
        size_t n = strlen(mounts[i].guest);
        if (!strncmp(buffer, mounts[i].guest, n) &&
            (buffer[n] == '/' || buffer[n] == '\\' || !buffer[n]) && n > best) {
            best = n;
            m = &mounts[i];
        }
    }

    if (!m) return 2; /* ENOENT */
    if ((size_t)snprintf(out, size, "%s%s", m->host, buffer + best) >= size) {
        return 63; /* ENAMETOOLONG */
    }
    return 0;
}

int runtime_file_translate(const char* guest, char* out, size_t size) {
    return r_fs_translate(guest, out, size);
}
