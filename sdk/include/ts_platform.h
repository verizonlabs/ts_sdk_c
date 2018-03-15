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

	/**
	 * Initialize the hardware configuration. This should be called before any function is called on the TS-SDK.
	 */
	void		(*initialize)();

	/**
	 * A facade for printf, usually routed to _write via stdlib or newlib, then onto a UART
	 */
	void 		(*printf)(const char *, ...);

	/**
	 * A facade for vprintf, it should same implementation as printf above.
	 */
	void 		(*vprintf)(const char *, va_list);

	/**
	 * Return the current GMT time in microseconds
	 */
	uint64_t	(*time)();

	/**
	 * (Deprecated) Sleep for the given amount of time, given in microseconds. Note that this function is being removed.
	 */
	void		(*sleep)(uint32_t);

	/**
	 * Return a crypto-quality random number (typically via hardware)
	 */
	void		(*random)(uint32_t*);

	/**
	 * Return allocated memory of the given size
	 */
	void *		(*malloc)(size_t);

	/**
	 * Free the allocated memory, the size is an estimate for the sake of profiling and can be set to zero.
	 */
	void		(*free)(void*,size_t);

	/**
	 * Handle any assertion, i.e., this function doesnt perform the check, it simply performs the effect, e.g.,
	 * display the given message and halt, etc.
	 */
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
