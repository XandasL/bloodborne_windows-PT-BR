/* SPDX-License-Identifier: MIT
 * PS4 Net & NetCtl Offline Simulation.
 * Single responsibility: Offline networking error stubs and byte conversions. (~70 LOC)
 */
#include "r_serv_types.h"

static int32_t net_errno = 0;

ABI int32_t net_init(void) { return 0; }
ABI int32_t net_term(void) { return 0; }
ABI int32_t *net_errno_loc(void) { return &net_errno; }
ABI int32_t net_pool_create(const char *name, int size, int flags) { (void)name; (void)size; (void)flags; return serv_new_id(); }
ABI int32_t net_pool_destroy(int id) { (void)id; return 0; }
ABI int32_t net_unreachable(void) { net_errno = 51; return SERV_NET_ENETUNREACH; }
ABI int32_t net_epoll_create(const char *name, int flags) { (void)name; (void)flags; return serv_new_id(); }
ABI int32_t net_epoll_destroy(int id) { (void)id; return 0; }
ABI int32_t net_resolver_create(const char *name, int pool, int flags) { (void)name; (void)pool; (void)flags; return serv_new_id(); }
ABI int32_t net_resolver_destroy(int id) { (void)id; return 0; }

ABI uint16_t net_htons(uint16_t v) { return (v >> 8) | (v << 8); }
ABI uint16_t net_ntohs(uint16_t v) { return (v >> 8) | (v << 8); }
ABI uint32_t net_htonl(uint32_t v) {
    return ((v >> 24) & 0xff) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000);
}
ABI uint32_t net_ntohl(uint32_t v) { return net_htonl(v); }

ABI int32_t net_pton(int af, const char *src, void *dst) {
    if (af != 2 || !src || !dst) { net_errno = 47; return SERV_NET_EINVAL; }
    unsigned int a, b, c, d;
    if (sscanf(src, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) return 0;
    unsigned char *out = (unsigned char *)dst;
    out[0] = (unsigned char)a; out[1] = (unsigned char)b; out[2] = (unsigned char)c; out[3] = (unsigned char)d;
    return 1;
}

ABI const char *net_ntop(int af, const void *src, char *dst, uint32_t size) {
    if (af != 2 || !src || !dst) { net_errno = 47; return NULL; }
    const unsigned char *b = (const unsigned char *)src;
    int n = snprintf(dst, size, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
    return (n > 0 && (uint32_t)n < size) ? dst : NULL;
}

ABI int32_t netctl_state(int32_t *state) { if (!state) return SERV_NET_CTL_INVALID_ADDR; *state = 0; return 0; }
ABI int32_t netctl_info(int code, void *info) { (void)code; (void)info; return SERV_NET_CTL_NOT_CONNECTED; }
ABI int32_t netctl_register(void *cb, void *arg, int32_t *cid) {
    (void)cb; (void)arg; if (!cid) return SERV_NET_CTL_INVALID_ADDR; *cid = serv_new_id(); return 0;
}
ABI int32_t netctl_check(void) { return 0; }
ABI int32_t netctl_unregister(int cid) { (void)cid; return 0; }
ABI int32_t netctl_nat(uint32_t *info) {
    if (!info) return SERV_NET_CTL_INVALID_ADDR;
    info[1] = 0; info[2] = 3; info[3] = 0;
    return 0;
}
