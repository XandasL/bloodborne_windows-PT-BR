/* SPDX-License-Identifier: MIT
 * PS4 Loader Param.sfo Parser.
 * Single responsibility: Reading title, content, and attribute keys from param.sfo. (~45 LOC)
 */
#include "loader_types.h"

int sfo_value(const char *path, const char *key, char *text, size_t text_size, uint32_t *number) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    unsigned char data[65536];
    size_t n = fread(data, 1, sizeof(data), f);
    fclose(f);
    if (n < 20 || memcmp(data, "\0PSF", 4)) return 0;
    uint32_t keys, values, count;
    memcpy(&keys, data + 8, 4); memcpy(&values, data + 12, 4); memcpy(&count, data + 16, 4);
    for (uint32_t i = 0; i < count && 20 + i * 16 + 16 <= n; ++i) {
        const unsigned char *e = data + 20 + i * 16;
        uint16_t key_offset, format; uint32_t length, offset;
        memcpy(&key_offset, e, 2); memcpy(&format, e + 2, 2);
        memcpy(&length, e + 4, 4); memcpy(&offset, e + 12, 4);
        if (keys + key_offset >= n || values + offset + length > n ||
            strcmp((const char*)data + keys + key_offset, key)) continue;
        if (format == 0x0404 && number && length >= 4) { memcpy(number, data + values + offset, 4); return 1; }
        if (text && text_size) {
            size_t copy = length < text_size - 1 ? length : text_size - 1;
            memcpy(text, data + values + offset, copy);
            text[copy] = 0;
        }
        return 1;
    }
    return 0;
}
