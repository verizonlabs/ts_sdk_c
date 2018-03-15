/**
 * @file
 * ts_platform.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The OS-agnostic abstraction of certain resources, i.e., console, time, memory and random number generation (RNG).
 *
 * @details
 * The SDK depends on this interface to access console, time, memory and RNG resources, which may be
 * implemented differently on different OS' or hardware (e.g., UART console, hardware RNG, etc.)
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) runtime. They provide the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */
#ifndef TS_PLATFORM_H
#define TS_PLATFORM_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define TS_TIME_HOUR_TO_USEC 3600000000L
#define TS_TIME_MIN_TO_USEC  60000000L
#define TS_TIME_SEC_TO_USEC  1000000L
#define TS_TIME_MSEC_TO_USEC 1000L
#define TS_TIME_USEC_TO_NSEC 1000L

/**
 * The platform object reference
 */
typedef struct TsPlatform *TsPlatformRef_t;

/**
 * The platform object
 */
typedef struct TsPlatform {} TsPlatform_t;

/**
 * The platform vector table (i.e., the platform "class" definition), used to define the platform SDK-aspect.
 * See examples/platforms for available platform implementations, or use your own customized implementation.
 */
typedef struct TsPlatformVtable {
	void		(*initialize)();
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
