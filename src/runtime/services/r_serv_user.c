/* SPDX-License-Identifier: MIT
 * PS4 UserService and SystemService HLE Implementation.
 * Single responsibility: Local user session and console parameters. (~65 LOC)
 */
#include "r_serv_types.h"

static _Atomic int s_login_pending = 1;

int serv_new_id(void) {
    static _Atomic int s_next_id = 1;
    return atomic_fetch_add(&s_next_id, 1);
}

void serv_note(const char *what) { printf("Runtime: %s\n", what); }
static int language(void) { const char *v = getenv("BB_LANGUAGE"); return v ? atoi(v) : 1; }

ABI int32_t user_initialize(const void *params) {
    (void)params; serv_note("UserService initialized (user 1 logged in)"); return 0;
}
ABI int32_t user_terminate(void) { return 0; }

ABI int32_t user_initial(int32_t *id) {
    if (!id) return SERV_USER_INVALID_ARGUMENT;
    *id = SERV_USER_ID; return 0;
}

ABI int32_t user_list(int32_t *ids) {
    if (!ids) return SERV_USER_INVALID_ARGUMENT;
    ids[0] = SERV_USER_ID; ids[1] = ids[2] = ids[3] = -1; return 0;
}

ABI int32_t user_name(int32_t id, char *name, uint64_t size) {
    if (id != SERV_USER_ID || !name) return SERV_USER_INVALID_ARGUMENT;
    const char *value = getenv("BB_USER_NAME") ? getenv("BB_USER_NAME") : "Hunter";
    if (strlen(value) + 1 > size) return (int32_t)0x8096000a;
    strcpy(name, value); return 0;
}

ABI int32_t user_event(int32_t *event) {
    if (!event) return SERV_USER_INVALID_ARGUMENT;
    int expected = 1;
    if (!atomic_compare_exchange_strong(&s_login_pending, &expected, 0)) return SERV_USER_NO_EVENT;
    event[0] = 0; event[1] = SERV_USER_ID; return 0;
}

ABI int32_t system_param(int32_t id, int32_t *value) {
    if (!value) return SERV_SYSTEM_PARAMETER;
    switch (id) {
    case 1: *value = language(); break;
    case 2: case 3: case 1000: *value = 1; break;
    case 4: *value = (int32_t)(bb_platform_time_tz_offset_us(0) / 60000000LL); break;
    case 5: case 7: *value = 0; break;
    default: fprintf(stderr, "STOP: unsupported system parameter %d\n", id); exit(21);
    }
    return 0;
}

ABI int32_t system_status(unsigned char *status) {
    if (!status) return SERV_SYSTEM_PARAMETER;
    memset(status, 0, 12); return 0;
}

ABI int32_t system_event(void *event) { (void)event; return SERV_SYSTEM_NO_EVENT; }
ABI int32_t hide_splash(void) { serv_note("SystemService: splash screen hidden"); return 0; }
ABI int32_t launch_browser(void) { serv_note("SystemService: web browser request ignored (offline)"); return 0; }
