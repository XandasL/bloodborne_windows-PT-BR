/* SPDX-License-Identifier: MIT
 * PS4 System Services Common Types and Error Definitions.
 * Single responsibility: Common service constants and error codes. (~60 LOC)
 */
#ifndef R_SERV_TYPES_H
#define R_SERV_TYPES_H

#include "runtime.h"
#include "platform/time.h"
#include "platform/threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#define SERV_USER_ID 1
#define SERV_ORBIS_OK 0
#define SERV_USER_INVALID_ARGUMENT ((int32_t)0x80960005)
#define SERV_USER_NO_EVENT         ((int32_t)0x80960007)
#define SERV_SYSTEM_NO_EVENT       ((int32_t)0x80A10004)
#define SERV_SYSTEM_PARAMETER      ((int32_t)0x80A10003)
#define SERV_NET_CTL_NOT_CONNECTED ((int32_t)0x80412108)
#define SERV_NET_CTL_INVALID_ADDR  ((int32_t)0x80412107)
#define SERV_NP_SIGNED_OUT         ((int32_t)0x80550006)
#define SERV_NP_INVALID_ARGUMENT   ((int32_t)0x80550003)
#define SERV_NET_ENETUNREACH       ((int32_t)0x80410133)
#define SERV_NET_EINVAL            ((int32_t)0x80410116)
#define SERV_HTTP_NETWORK          ((int32_t)0x80431063)
#define SERV_DIALOG_NOT_INITIALIZED ((int32_t)0x80B80003)
#define SERV_PLAYGO_BAD_HANDLE     ((int32_t)0x80B20009)
#define SERV_PLAYGO_BAD_POINTER    ((int32_t)0x80B2000A)
#define SERV_PLAYGO_BAD_SIZE       ((int32_t)0x80B2000B)
#define SERV_PLAYGO_BAD_CHUNK      ((int32_t)0x80B2000C)
#define SERV_AUDIO_IN_NOT_OPENED   ((int32_t)0x80260109)
#define SERV_TROPHY_INVALID        ((int32_t)0x80551604)
#define SERV_DISC_MAP_NO_BITMAP    ((int32_t)0x81100004)

int serv_new_id(void);
void serv_note(const char *what);

#endif /* R_SERV_TYPES_H */
