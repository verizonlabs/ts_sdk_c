// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PLATFORMS_H
#define TS_PLATFORMS_H

#include "ts_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TsPlatformVtable_t ts_platform_unix;
//extern TsPlatformVtable_t ts_platform_windows;
//extern TsPlatformVtable_t ts_platform_raspberry_pi3;
//extern TsPlatformVtable_t ts_platform_stmicro_neucleo;
//extern TsPlatformVtable_t ts_platform_renasas_s5;
//extern TsPlatformVtable_t ts_platform_nxp;

#ifdef __cplusplus
}
#endif

#endif // TS_PLATFORMS_H
