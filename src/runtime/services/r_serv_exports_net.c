/* SPDX-License-Identifier: MIT
 * PS4 Net & Http Service Dispatch Table.
 * Single responsibility: Resolving Net, NetCtl, Http, and Ssl symbols. (~70 LOC)
 */
#include "r_serv_types.h"

ABI int32_t net_init(void); ABI int32_t net_term(void); ABI int32_t *net_errno_loc(void);
ABI int32_t net_pool_create(const char*, int, int); ABI int32_t net_pool_destroy(int);
ABI int32_t net_epoll_create(const char*, int); ABI int32_t net_epoll_destroy(int);
ABI int32_t net_resolver_create(const char*, int, int); ABI int32_t net_resolver_destroy(int);
ABI uint16_t net_htons(uint16_t); ABI uint16_t net_ntohs(uint16_t);
ABI uint32_t net_htonl(uint32_t); ABI uint32_t net_ntohl(uint32_t);
ABI int32_t net_pton(int, const char*, void*); ABI const char* net_ntop(int, const void*, char*, uint32_t);
ABI int32_t net_unreachable(void);
ABI int32_t netctl_state(int32_t*); ABI int32_t netctl_info(int, void*);
ABI int32_t netctl_register(void*, void*, int32_t*); ABI int32_t netctl_check(void);
ABI int32_t netctl_unregister(int); ABI int32_t netctl_nat(uint32_t*);
ABI int32_t lib_init_id(void); ABI int32_t ok_void(void); ABI int32_t http_fail(void);
ABI int32_t http_object(void); ABI int32_t http_epoll(int32_t, void**);
ABI int32_t http_wait(void*, void*, int32_t, int64_t);

static const RuntimeExport s_net_exports[] = {
    {"sceNetInit", net_init}, {"sceNetTerm", net_term}, {"sceNetErrnoLoc", net_errno_loc},
    {"sceNetPoolCreate", net_pool_create}, {"sceNetPoolDestroy", net_pool_destroy},
    {"sceNetEpollCreate", net_epoll_create}, {"sceNetEpollDestroy", net_epoll_destroy},
    {"sceNetResolverCreate", net_resolver_create}, {"sceNetResolverDestroy", net_resolver_destroy},
    {"sceNetHtons", net_htons}, {"sceNetNtohs", net_ntohs}, {"sceNetHtonl", net_htonl}, {"sceNetNtohl", net_ntohl},
    {"sceNetInetPton", net_pton}, {"sceNetInetNtop", net_ntop},
    {"sceNetSocket", net_unreachable}, {"sceNetConnect", net_unreachable}, {"sceNetBind", net_unreachable},
    {"sceNetListen", net_unreachable}, {"sceNetAccept", net_unreachable}, {"sceNetSend", net_unreachable},
    {"sceNetSendto", net_unreachable}, {"sceNetRecv", net_unreachable}, {"sceNetRecvfrom", net_unreachable},
    {"sceNetSetsockopt", net_unreachable}, {"sceNetGetsockopt", net_unreachable},
    {"sceNetGetsockname", net_unreachable}, {"sceNetShutdown", net_unreachable},
    {"sceNetSocketClose", net_unreachable}, {"sceNetSocketAbort", net_unreachable},
    {"sceNetEpollControl", net_unreachable}, {"sceNetEpollWait", net_unreachable}, {"sceNetEpollAbort", net_unreachable},
    {"sceNetResolverStartNtoa", net_unreachable}, {"sceNetResolverStartAton", net_unreachable},
    {"sceNetCtlGetState", netctl_state}, {"sceNetCtlGetInfo", netctl_info},
    {"sceNetCtlRegisterCallback", netctl_register}, {"sceNetCtlCheckCallback", netctl_check},
    {"sceNetCtlUnregisterCallback", netctl_unregister}, {"sceNetCtlGetNatInfo", netctl_nat},
    {"sceSslInit", lib_init_id}, {"sceSslTerm", ok_void},
    {"sceHttpInit", lib_init_id}, {"sceHttpTerm", ok_void},
    {"sceHttpCreateTemplate", http_object}, {"sceHttpDeleteTemplate", ok_void},
    {"sceHttpCreateConnectionWithURL", http_object}, {"sceHttpCreateRequestWithURL", http_object},
    {"sceHttpSendRequest", http_fail}, {"sceHttpCreateEpoll", http_epoll},
    {"sceHttpSetNonblock", ok_void}, {"sceHttpSetConnectTimeOut", ok_void},
    {"sceHttpsEnableOption", ok_void}, {"sceHttpsDisableOption", ok_void},
    {"sceHttpAddRequestHeader", ok_void}, {"sceHttpSetRequestContentLength", ok_void},
    {"sceHttpDeleteConnection", ok_void}, {"sceHttpDeleteRequest", ok_void},
    {"sceHttpAbortWaitRequest", ok_void}, {"sceHttpDestroyEpoll", ok_void},
    {"sceHttpSetEpoll", ok_void}, {"sceHttpUnsetEpoll", ok_void}, {"sceHttpWaitRequest", http_wait},
    {"sceHttpGetStatusCode", http_fail}, {"sceHttpGetResponseContentLength", http_fail},
    {"sceHttpReadData", http_fail},
};

uintptr_t serv_resolve_net(const char *name) {
    return RUNTIME_LOOKUP(s_net_exports, name);
}
