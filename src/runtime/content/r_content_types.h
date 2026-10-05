/* SPDX-License-Identifier: MIT
 * PS4 AppContent & Module Loader Types.
 * Single responsibility: Content profile and module error constants. (~35 LOC)
 */
#ifndef R_CONTENT_TYPES_H
#define R_CONTENT_TYPES_H

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_INVALID_ID ((int32_t)0x805a1000)
#define CONTENT_NOT_LOADED ((int32_t)0x805a1001)
#define CONTENT_PARAMETER  ((int32_t)0x80d90002)
#define CONTENT_BUSY       ((int32_t)0x80d90003)

extern unsigned g_content_refs;
extern unsigned g_content_initialized;
extern unsigned g_content_configured;
extern uint32_t g_content_parameters[5];
extern unsigned g_content_queries;
extern unsigned g_content_lists;

void require_provider(void);
void require_loaded(void);
void require_initialized(void);

#endif /* R_CONTENT_TYPES_H */
