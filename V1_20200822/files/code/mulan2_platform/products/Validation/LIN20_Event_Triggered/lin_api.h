/*
 * Standard LIN C API
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#ifndef LIN_API_H_
#define LIN_API_H_

#define LIN_VERSION 20
#define HAS_EVENT_TRIGGERED_FRAMES

/* Node configuration services */
#define HAS_ASSIGN_NAD_SERVICE

#define HAS_CONDITIONAL_CHANGE_NAD_SERVICE

/* #define HAS_SAVE_CONFIGURATION_SERVICE */

#define HAS_SERIAL_NUMBER_CALLOUT
/* #define HAS_READ_BY_ID_CALLOUT */

/* Order of includes is important */
#include <syslib.h>
#include <lin.h>
#include "lin_core.h"
#include "lin_cfg.h"
#include "lin_signals.h"
#if LIN_VERSION >= 20
#include "lin_diag.h"
#endif
#endif /* LIN_API_H_ */
