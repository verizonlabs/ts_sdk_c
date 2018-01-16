// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PLATFORM_H
#define TS_PLATFORM_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define TS_TIME_SEC_TO_USEC  1000000
#define TS_TIME_MSEC_TO_USEC 1000
#define TS_TIME_USEC_TO_NSEC 1000

typedef struct TsPlatform *TsPlatformRef_t;
typedef struct TsPlatform {
	// TODO - memory profile, hardware specs
} TsPlatform_t;

// TODO - rework to fit convention (e.g., initialize becomes create - enable profiling, enable status returns
// TODO - comment on the time unit (e.g., usec vs msec)

typedef struct TsPlatformVtable {
	void		(*initialize)();
	// TODO - switch to one printf?
	void 		(*printf)(const char *, ...);
	void 		(*vprintf)(const char *, va_list);
	uint64_t	(*time)();
	void		(*sleep)(uint32_t);
	void		(*random)(uint32_t*);
	void *		(*malloc)(size_t);
	void		(*free)(void*,size_t);
	void		(*assertion)(const char *msg, const char *file, int line);
} TsPlatformVtable_t;

extern const TsPlatformVtable_t * ts_platform;

#define ts_platform_initialize	ts_platform->initialize
#define ts_platform_printf		ts_platform->printf
#define ts_platform_vprintf		ts_platform->vprintf
#define ts_platform_time		ts_platform->time
#define ts_platform_sleep		ts_platform->sleep
#define ts_platform_random		ts_platform->random
#define ts_platform_malloc		ts_platform->malloc
#define ts_platform_free		ts_platform->free
#ifdef NDEBUG
#define ts_platform_assert(EX) (void)0
#else
#define ts_platform_assert(EX)  (void)((EX) || (ts_platform->assertion(#EX,__FILE__,__LINE__),0))
#endif

#endif // TS_PLATFORM_H
