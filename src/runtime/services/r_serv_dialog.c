/* SPDX-License-Identifier: MIT
 * PS4 Common Dialog Subsystems.
 * Single responsibility: Emulated immediate completion of system dialogs. (~65 LOC)
 */
#include "r_serv_types.h"

typedef struct { const char *name; int initialized, status; } Dialog;
static Dialog dialogs[] = {
    {"CommonDialog", 0, 0}, {"MsgDialog", 0, 0}, {"SaveDataDialog", 0, 0},
    {"NpProfileDialog", 0, 0}, {"NpCommerceDialog", 0, 0}, {"ImeDialog", 0, 0}
};

static int common_initialized = 0;
ABI int32_t common_init(void) { common_initialized = 1; return 0; }

static int32_t dialog_init(int i) {
    if (dialogs[i].initialized) return (int32_t)0x80B80004;
    dialogs[i].initialized = 1; dialogs[i].status = 1; return 0;
}
static int32_t dialog_open(int i) {
    if (!dialogs[i].initialized) return SERV_DIALOG_NOT_INITIALIZED;
    printf("Runtime: %s opened; completed immediately (no dialog UI yet)\n", dialogs[i].name);
    dialogs[i].status = 3; return 0;
}
static int32_t dialog_status(int i) { return dialogs[i].status; }
static int32_t dialog_term(int i) {
    if (!dialogs[i].initialized) return SERV_DIALOG_NOT_INITIALIZED;
    dialogs[i].initialized = 0; dialogs[i].status = 0; return 0;
}

#define DIALOG_DEF(tag, i) \
    ABI int32_t tag##_init(void) { return dialog_init(i); } \
    ABI int32_t tag##_open(const void *p) { (void)p; return dialog_open(i); } \
    ABI int32_t tag##_status(void) { return dialog_status(i); } \
    ABI int32_t tag##_term(void) { return dialog_term(i); }

DIALOG_DEF(msg, 1)
DIALOG_DEF(save_dlg, 2)
DIALOG_DEF(profile, 3)
DIALOG_DEF(commerce, 4)

ABI int32_t profile_result(void *result) {
    if (result) memset(result, 0, 4);
    return 0;
}
