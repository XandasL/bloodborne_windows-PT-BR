/* SPDX-License-Identifier: MIT
 * PS4 Services Root Dispatcher.
 * Single responsibility: Resolving User, System, Dialog, Trophy, and PlayGo symbols. (~65 LOC)
 */
#include "r_serv_types.h"

ABI int32_t user_initialize(const void*); ABI int32_t user_terminate(void); ABI int32_t user_initial(int32_t*);
ABI int32_t user_list(int32_t*); ABI int32_t user_name(int32_t, char*, uint64_t); ABI int32_t user_event(int32_t*);
ABI int32_t system_param(int32_t, int32_t*); ABI int32_t system_status(unsigned char*); ABI int32_t system_event(void*);
ABI int32_t hide_splash(void); ABI int32_t launch_browser(void); ABI int32_t common_init(void);
ABI int32_t msg_init(void); ABI int32_t msg_open(const void*); ABI int32_t msg_status(void); ABI int32_t msg_term(void);
ABI int32_t save_dlg_init(void); ABI int32_t save_dlg_open(const void*); ABI int32_t save_dlg_status(void); ABI int32_t save_dlg_term(void);
ABI int32_t profile_init(void); ABI int32_t profile_open(const void*); ABI int32_t profile_status(void); ABI int32_t profile_term(void); ABI int32_t profile_result(void*);
ABI int32_t commerce_init(void); ABI int32_t commerce_open(const void*); ABI int32_t commerce_status(void); ABI int32_t commerce_term(void);
ABI int32_t ime_init(const void*, const void*); ABI int32_t ime_status(void); ABI int32_t ime_result(uint32_t*); ABI int32_t ime_term(void);
ABI int32_t trophy_context(int32_t*, int32_t, uint32_t, uint64_t); ABI int32_t trophy_handle(int32_t*);
ABI int32_t trophy_register(int32_t, int32_t, uint64_t); ABI int32_t trophy_unlock(int32_t, int32_t, int32_t, int32_t*);
ABI int32_t trophy_game_info(int32_t, int32_t, void*, void*); ABI int32_t trophy_info(int32_t, int32_t, int32_t, void*, void*);
ABI int32_t playgo_init(const void*); ABI int32_t playgo_open(int32_t*, const void*);
ABI int32_t playgo_chunk_ids(int32_t, uint16_t*, uint32_t, uint32_t*); ABI int32_t playgo_locus(int32_t, const uint16_t*, uint32_t, int8_t*);
ABI int32_t playgo_speed(int32_t, int32_t); ABI int32_t discmap_on_hdd(const char*, int64_t, int64_t, int32_t*);
ABI int32_t discmap_8a82(const char*, int64_t, int64_t, int32_t*, int32_t*, int32_t*);
ABI int32_t mouse_open(int32_t, int32_t, int32_t, const void*); ABI int32_t mouse_read(int32_t, unsigned char*, int32_t);
ABI int32_t mouse_close(int32_t); ABI int32_t audio_in_open(void); ABI int32_t ok_void(void);
ABI int32_t voice_port(void*, uint32_t*); ABI int32_t voice_read(uint32_t, void*, uint32_t*);
ABI int32_t voice_write(uint32_t, const void*, uint32_t*); ABI int32_t voice_info(uint32_t, uint32_t*);
ABI int32_t discmap_on_hdd(const char*, int64_t, int64_t, int32_t*);
ABI int32_t discmap_8a82(const char*, int64_t, int64_t, int32_t*, int32_t*, int32_t*);
ABI int32_t mouse_open(int32_t, int32_t, int32_t, const void*); ABI int32_t mouse_read(int32_t, unsigned char*, int32_t);
ABI int32_t mouse_close(int32_t); ABI int32_t audio_in_open(void);
ABI int32_t voice_port(void*, uint32_t*); ABI int32_t voice_read(uint32_t, void*, uint32_t*);
ABI int32_t voice_write(uint32_t, const void*, uint32_t*); ABI int32_t voice_info(uint32_t, uint32_t*);
ABI int32_t ok_void(void);

uintptr_t serv_resolve_net(const char *name);
uintptr_t serv_resolve_np_extended(const char *name);

static const RuntimeExport s_core_serv_exports[] = {
    {"sceUserServiceInitialize", user_initialize}, {"sceUserServiceTerminate", user_terminate},
    {"sceUserServiceGetInitialUser", user_initial}, {"sceUserServiceGetLoginUserIdList", user_list},
    {"sceUserServiceGetUserName", user_name}, {"sceUserServiceGetEvent", user_event},
    {"sceSystemServiceParamGetInt", system_param}, {"sceSystemServiceGetStatus", system_status},
    {"sceSystemServiceReceiveEvent", system_event}, {"sceSystemServiceHideSplashScreen", hide_splash},
    {"sceSystemServiceLaunchWebBrowser", launch_browser},
    {"sceCommonDialogInitialize", common_init},
    {"sceMsgDialogInitialize", msg_init}, {"sceMsgDialogOpen", msg_open},
    {"sceMsgDialogUpdateStatus", msg_status}, {"sceMsgDialogTerminate", msg_term},
    {"sceSaveDataDialogInitialize", save_dlg_init}, {"sceSaveDataDialogOpen", save_dlg_open},
    {"sceSaveDataDialogUpdateStatus", save_dlg_status}, {"sceSaveDataDialogTerminate", save_dlg_term},
    {"sceNpProfileDialogInitialize", profile_init}, {"sceNpProfileDialogOpen", profile_open},
    {"sceNpProfileDialogUpdateStatus", profile_status}, {"sceNpProfileDialogTerminate", profile_term},
    {"sceNpProfileDialogGetResult", profile_result},
    {"sceNpCommerceDialogInitialize", commerce_init}, {"sceNpCommerceDialogOpen", commerce_open},
    {"sceNpCommerceDialogUpdateStatus", commerce_status}, {"sceNpCommerceDialogTerminate", commerce_term},
    {"sceImeDialogInit", ime_init}, {"sceImeDialogGetStatus", ime_status}, {"sceImeDialogGetResult", ime_result},
    {"sceImeDialogTerm", ime_term}, {"sceImeDialogAbort", ime_term},
    {"sceNpTrophyCreateContext", trophy_context}, {"sceNpTrophyCreateHandle", trophy_handle},
    {"sceNpTrophyRegisterContext", trophy_register}, {"sceNpTrophyUnlockTrophy", trophy_unlock},
    {"sceNpTrophyGetGameInfo", trophy_game_info}, {"sceNpTrophyGetTrophyInfo", trophy_info},
    {"scePlayGoInitialize", playgo_init}, {"scePlayGoOpen", playgo_open}, {"scePlayGoGetChunkId", playgo_chunk_ids},
    {"scePlayGoGetLocus", playgo_locus}, {"scePlayGoSetInstallSpeed", playgo_speed},
    {"sceMouseInit", ok_void}, {"sceMouseOpen", mouse_open}, {"sceMouseRead", mouse_read}, {"sceMouseClose", mouse_close},
    {"sceAudioInOpen", audio_in_open}, {"sceAudioInInput", audio_in_open}, {"sceAudioInClose", audio_in_open},
    {"sceDiscMapIsRequestOnHDD", discmap_on_hdd}, {"sceDiscMap_8A828CAEE7EDD5E9", discmap_8a82},
    {"sceVoiceInit", ok_void}, {"sceVoiceEnd", ok_void}, {"sceVoiceCreatePort", voice_port},
    {"sceVoiceDeletePort", ok_void}, {"sceVoiceConnectIPortToOPort", ok_void},
    {"sceVoiceDisconnectIPortFromOPort", ok_void}, {"sceVoiceStart", ok_void}, {"sceVoiceStop", ok_void},
    {"sceVoiceGetPortInfo", voice_info}, {"sceVoiceReadFromOPort", voice_read}, {"sceVoiceWriteToIPort", voice_write},
};

uintptr_t runtime_services_resolve(const char *name) {
    uintptr_t res = RUNTIME_LOOKUP(s_core_serv_exports, name);
    return res ? res : (res = serv_resolve_net(name)) ? res : serv_resolve_np_extended(name);
}
