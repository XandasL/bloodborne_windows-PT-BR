/* SPDX-License-Identifier: MIT
 * PS4 Services NP Extended Resolver.
 * Single responsibility: NpMatching2, Signaling, and WebApi export tables. (~70 LOC)
 */
#include "r_serv_types.h"

ABI int32_t np_signed_out(void);
ABI int32_t ok_void(void);

static const RuntimeExport s_np_exports[] = {
    {"sceNpWebApiCreateContext", np_signed_out}, {"sceNpWebApiCreateRequest", np_signed_out},
    {"sceNpWebApiSendRequest", np_signed_out}, {"sceNpWebApiDeleteRequest", ok_void},
    {"sceNpWebApiAbortRequest", ok_void}, {"sceNpWebApiDeleteContext", ok_void},
    {"sceNpWebApiReadData", np_signed_out}, {"sceNpWebApiGetHttpStatusCode", np_signed_out},
    {"sceNpWebApiGetHttpResponseHeaderValue", np_signed_out},
    {"sceNpWebApiGetHttpResponseHeaderValueLength", np_signed_out},
    {"sceNpWebApiCreatePushEventFilter", np_signed_out}, {"sceNpWebApiDeletePushEventFilter", ok_void},
    {"sceNpWebApiRegisterPushEventCallback", np_signed_out}, {"sceNpWebApiUnregisterPushEventCallback", ok_void},
    {"sceNpWebApiUtilityParseNpId", np_signed_out},
    {"sceNpMatching2ContextStart", np_signed_out}, {"sceNpMatching2ContextStop", ok_void},
    {"sceNpMatching2DestroyContext", ok_void}, {"sceNpMatching2RegisterContextCallback", ok_void},
    {"sceNpMatching2RegisterLobbyEventCallback", ok_void}, {"sceNpMatching2RegisterRoomEventCallback", ok_void},
    {"sceNpMatching2RegisterSignalingCallback", ok_void}, {"sceNpMatching2SetDefaultRequestOptParam", ok_void},
    {"sceNpMatching2CreateJoinRoom", np_signed_out}, {"sceNpMatching2JoinRoom", np_signed_out},
    {"sceNpMatching2LeaveRoom", np_signed_out}, {"sceNpMatching2SearchRoom", np_signed_out},
    {"sceNpMatching2GetServerId", np_signed_out}, {"sceNpMatching2GetWorldInfoList", np_signed_out},
    {"sceNpMatching2GetLobbyInfoList", np_signed_out}, {"sceNpMatching2JoinLobby", np_signed_out},
    {"sceNpMatching2LeaveLobby", np_signed_out}, {"sceNpMatching2GrantRoomOwner", np_signed_out},
    {"sceNpMatching2KickoutRoomMember", np_signed_out}, {"sceNpMatching2SetRoomDataExternal", np_signed_out},
    {"sceNpMatching2SetRoomDataInternal", np_signed_out}, {"sceNpMatching2SetRoomMemberDataInternal", np_signed_out},
    {"sceNpMatching2SignalingGetConnectionStatus", np_signed_out}, {"sceNpMatching2SignalingGetPingInfo", np_signed_out},
    {"sceNpSignalingDeleteContext", ok_void}, {"sceNpSignalingActivateConnection", np_signed_out},
    {"sceNpSignalingDeactivateConnection", ok_void}, {"sceNpSignalingGetConnectionStatus", np_signed_out},
    {"sceNpScoreCensorComment", np_signed_out}, {"sceNpScoreSanitizeComment", np_signed_out},
    {"sceNpScoreGetBoardInfo", np_signed_out}, {"sceNpScoreGetGameData", np_signed_out},
    {"sceNpScoreGetRankingByNpIdPcId", np_signed_out}, {"sceNpScoreGetRankingByRange", np_signed_out},
    {"sceNpScoreRecordGameData", np_signed_out}, {"sceNpScoreRecordScore", np_signed_out},
    {"sceNpScoreSetPlayerCharacterId", ok_void},
};

uintptr_t serv_resolve_np_extended(const char *name) {
    return RUNTIME_LOOKUP(s_np_exports, name);
}
