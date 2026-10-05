/* SPDX-License-Identifier: MIT
 * PS4 PlayGo, DiscMap, and Auxiliary Peripheral Stubs.
 * Single responsibility: Fully installed package reporting and absent peripheral stubs. (~70 LOC)
 */
#include "r_serv_types.h"

static int playgo_handle = 0, playgo_chunks = 1;

ABI int32_t playgo_init(const void *params) {
    (void)params;
    int fd = (int)runtime_file_open("/app0/sce_sys/playgo-chunk.dat", 0, 0);
    unsigned char header[12];
    playgo_chunks = 1;
    if (fd >= 0) {
        if (runtime_file_read(fd, header, sizeof(header)) == sizeof(header))
            playgo_chunks = header[10] | (header[11] << 8);
        runtime_file_close(fd);
    }
    printf("Runtime: PlayGo initialized; %d chunks, all installed locally\n", playgo_chunks);
    return 0;
}

ABI int32_t playgo_open(int32_t *h, const void *p) { (void)p; if (!h) return SERV_PLAYGO_BAD_POINTER; playgo_handle = 1; *h = 1; return 0; }
ABI int32_t playgo_chunk_ids(int32_t h, uint16_t *ids, uint32_t count, uint32_t *out) {
    if (h != playgo_handle || !h) return SERV_PLAYGO_BAD_HANDLE;
    if (!out) return SERV_PLAYGO_BAD_POINTER;
    if (ids && !count) return SERV_PLAYGO_BAD_SIZE;
    if (!ids) { *out = (uint32_t)playgo_chunks; return 0; }
    uint32_t n = count < (uint32_t)playgo_chunks ? count : (uint32_t)playgo_chunks;
    for (uint32_t i = 0; i < n; ++i) ids[i] = (uint16_t)i;
    *out = n; return 0;
}

ABI int32_t playgo_locus(int32_t h, const uint16_t *ids, uint32_t count, int8_t *loci) {
    if (h != playgo_handle || !h) return SERV_PLAYGO_BAD_HANDLE;
    if (!ids || !loci) return SERV_PLAYGO_BAD_POINTER;
    if (!count) return SERV_PLAYGO_BAD_SIZE;
    for (uint32_t i = 0; i < count; ++i) {
        if (ids[i] >= playgo_chunks) return SERV_PLAYGO_BAD_CHUNK;
        loci[i] = 3; /* LocalFast */
    }
    return 0;
}

ABI int32_t playgo_speed(int32_t h, int32_t speed) { (void)speed; return (h == playgo_handle && h) ? 0 : SERV_PLAYGO_BAD_HANDLE; }
ABI int32_t discmap_on_hdd(const char *p, int64_t off, int64_t sz, int32_t *res) { (void)p; (void)off; (void)sz; (void)res; return SERV_DISC_MAP_NO_BITMAP; }
ABI int32_t discmap_8a82(const char *p, int64_t off, int64_t sz, int32_t *fl, int32_t *r1, int32_t *r2) { (void)p; (void)off; (void)sz; (void)fl; (void)r1; (void)r2; return SERV_DISC_MAP_NO_BITMAP; }
ABI int32_t mouse_open(int32_t u, int32_t t, int32_t idx, const void *p) { (void)u; (void)t; (void)idx; (void)p; return serv_new_id(); }
ABI int32_t mouse_read(int32_t h, unsigned char *d, int32_t count) { (void)h; if (!d || count < 1) return (int32_t)0x80DF0001; memset(d, 0, 40); return 1; }
ABI int32_t mouse_close(int32_t h) { (void)h; return 0; }
ABI int32_t audio_in_open(void) { return SERV_AUDIO_IN_NOT_OPENED; }
ABI int32_t voice_port(void *p, uint32_t *port) { (void)p; if (!port) return (int32_t)0x8029000b; *port = (uint32_t)serv_new_id(); return 0; }
ABI int32_t voice_read(uint32_t port, void *data, uint32_t *size) { (void)port; (void)data; if (size) *size = 0; return 0; }
ABI int32_t voice_write(uint32_t port, const void *data, uint32_t *size) { (void)port; (void)data; (void)size; return 0; }
ABI int32_t voice_info(uint32_t port, uint32_t *info) { (void)port; if (!info) return (int32_t)0x8029000b; memset(info, 0, 40); return 0; }
