// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PLATFORMS_H
#define TS_PLATFORMS_H

#include "ts_platform.h"
#include "ts_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Use the development board along with the platform name to make a decision
// about what platform implementation to use.

//extern TsPlatformVtable_t ts_platform_windows;
//extern TsPlatformVtable_t ts_platform_none_freedom;
//extern TsPlatformVtable_t ts_platform_threadX_renasas_s5;

#if defined(TS_PLATFORM_UNIX)
extern TsPlatformVtable_t ts_platform_unix;
#if defined(TS_DRIVER_SERIAL)
extern TsDriverVtable_t ts_driver_unix_serial;
#else
extern TsDriverVtable_t ts_driver_unix_socket;
#endif
#elif defined(TS_PLATFORM_NONE)
extern TsPlatformVtable_t ts_platform_none_nucleo;
#if defined(TS_DRIVER_UART)
extern TsDriverVtable_t ts_driver_none_uart;
#endif
#elif defined(TS_PLATFORM_CUSTOM)
// Do nothing
#else
#warning "TS_PLATFORM_<TYPE> not defined, options include UNIX, NONE or CUSTOM"
#endif

#ifdef __cplusplus
}
#endif

#endif // TS_PLATFORMS_H
