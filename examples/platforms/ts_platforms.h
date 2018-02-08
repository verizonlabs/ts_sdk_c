// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PLATFORMS_H
#define TS_PLATFORMS_H

#include "ts_platform.h"
#include "ts_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TsPlatformVtable_t ts_platform_unix;
//extern TsPlatformVtable_t ts_platform_windows;
//extern TsPlatformVtable_t ts_platform_unix_raspberry_pi3;
//extern TsPlatformVtable_t ts_platform_none;
//extern TsPlatformVtable_t ts_platform_threadX_renasas_s5;

extern TsDriverVtable_t ts_driver_unix_socket;
extern TsDriverVtable_t ts_driver_unix_serial;
extern TsDriverVtable_t ts_driver_none_uart;

#ifdef __cplusplus
}
#endif

#endif // TS_PLATFORMS_H
