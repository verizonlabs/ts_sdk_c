// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif
#if _POSIX_C_SOURCE < 200809L
#include <sys/time.h>
#endif

// TODO - post compile-time warning if not a unix system

#include "ts_platform.h"

static void ts_initialize();
static void ts_printf(const char *, ...);
static void ts_vprintf(const char *, va_list);
static uint64_t ts_time();
static void ts_sleep(uint32_t);
static void ts_random(uint32_t*);
static void * ts_malloc(size_t);
static void ts_free(void *, size_t);
static void ts_assertion(const char *, const char *, int);

TsPlatformVtable_t ts_platform_unix = {
    .initialize = ts_initialize,
    .printf = ts_printf,
    .vprintf = ts_vprintf,
    .time = ts_time,
    .sleep = ts_sleep,
    .random = ts_random,
    .malloc = ts_malloc,
    .free = ts_free,
    .assertion = ts_assertion,
};

static void ts_initialize() {
    // do nothing
}

static void ts_printf(const char * format, ...) {
    va_list argp;
	va_start(argp, format);
    vprintf(format, argp);
	va_end(argp);
    fflush(stdout);
}

static void ts_vprintf(const char * format, va_list argp) {
    vprintf(format, argp);
    fflush(stdout);
}

// TODO - does this work the same on linux vs mac? monotonic increasing time?
static uint64_t ts_time() {
    uint64_t microseconds;
#if _POSIX_C_SOURCE >= 200809L
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    microseconds = (uint64_t)(spec.tv_sec) * TS_TIME_SEC_TO_USEC + (uint64_t)(spec.tv_nsec) / TS_TIME_USEC_TO_NSEC;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    microseconds = (uint64_t)(tv.tv_sec) * TS_TIME_SEC_TO_USEC + (uint64_t)(tv.tv_usec);
#endif
    return microseconds;
}

static void ts_sleep(uint32_t microseconds) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = microseconds / TS_TIME_SEC_TO_USEC;
    ts.tv_nsec = (microseconds % TS_TIME_SEC_TO_USEC) * TS_TIME_USEC_TO_NSEC;
    nanosleep(&ts, NULL);
#else
    usleep(microseconds);
#endif
}

static void ts_random(uint32_t * number) {
    srand((int)ts_time());
    *number = (uint32_t)(rand()%(2^32-1));
}

static void * ts_malloc(size_t size) {
    return malloc( size );
}

static void ts_free(void * pointer, size_t size) {
    return free( pointer );
}

static void ts_assertion(const char *msg, const char *file, int line) {
    printf("assertion failed, '%s' at %s:%d\n", msg, file, line);
    fflush(stdout);
    exit(0);
}
