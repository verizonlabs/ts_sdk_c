// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platforms.h"

// platform binding
#if defined(TS_PLATFORM_UNIX)
const TsPlatformVtable_t *  ts_platform = &(ts_platform_unix);
#if defined(TS_DRIVER_SERIAL)
const TsDriverVtable_t *    ts_driver = &(ts_driver_unix_serial);
#else
const TsDriverVtable_t *    ts_driver = &(ts_driver_unix_socket);
#endif
#elif defined(TS_PLATFORM_NONE)	// TODO: Make decision based on board
const TsPlatformVtable_t * ts_platform = &(ts_platform_none_nucleo);
#if defined(TS_DRIVER_UART)
const TsDriverVtable_t * ts_driver = &(ts_driver_none_uart);
#endif
#elif defined(TS_PLATFORM_CUSTOM)
// do nothing
#else
#warning "TS_PLATFORM_<TYPE> not defined, options include UNIX, NONE or CUSTOM"
#endif
