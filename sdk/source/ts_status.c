// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#include "ts_platform.h"
#include "ts_status.h"

static TsStatusLevel_t _level = TsStatusLevelAlarm;

typedef struct {
	TsStatus_t status;
	int http;
	char * error;
	char * description;
} TsStatusIndexTuple_t;

static TsStatusIndexTuple_t _ts_status_index_map[] = {
	{ TsStatusUnknown,                  500, "internal_error",  "status unknown" },
	{ TsStatusOkEnqueue,                100, "trying",          "partial success, subsequent operation is expected (e.g., trying)" },
	{ TsStatusOk,                       200, "ok",              "operation completed successfully" },
	{ TsStatusOkReadPending,            200, "ok",              "read pending" },

	{ TsStatusError,                    400, "bad_request",     "request precondition not met" },
	{ TsStatusErrorBadRequest,          400, "bad_request",     "request data malformed" },
	{ TsStatusErrorNotFound,            404, "not_found",       "missing information" },
	{ TsStatusErrorPreconditionFailed,  412, "precondition",    "missing or wrong data or configuration parameters" },
	{ TsStatusErrorPayloadTooLarge,     413, "payload",         "payload too large" },

	{ TsStatusErrorInternalServerError, 500, "internal_error",  "unknown critical error" },
	{ TsStatusErrorNotImplemented,      501, "not_implemented", "operation not ready for use" },
	{ TsStatusErrorBadGateway,          502, "bad_gateway",     "gateway connection failed" },
	{ TsStatusErrorOutOfMemory,         500, "out_of_memory",   "soft (detectable) out-of-memory state" },
	{ TsStatusErrorIndexOutOfRange,     500, "out_of_range",    "index is out of range" },
	{ TsStatusErrorRecursionTooDeep,    500, "recursion",       "stack limitation exceeded" },
	{ TsStatusErrorNoResourceAvailable, 500, "no_resource",     "no hardware resources available" },
	{ TsStatusErrorConnectionReset,     500, "connection",      "connection reset" },
};
static int _ts_status_index_map_size = sizeof( _ts_status_index_map )/sizeof( TsStatusIndexTuple_t );

static char * _ts_level_to_string[] = {
	"TRACE",
	"DEBUG",
	"INFO",
	"ALARM",
};

static TsStatusIndexTuple_t * _ts_status_tuple( TsStatus_t status ) {
	for( int i = 0; i < _ts_status_index_map_size; i++ ) {
		if( _ts_status_index_map[i].status == status ) {
			return &(_ts_status_index_map[i]);
		}
	}
	return &(_ts_status_index_map[0]);
}

const char * ts_status_string( TsStatus_t status ) {
	TsStatusIndexTuple_t * tuple = _ts_status_tuple( status );
	return tuple->description;
}

int ts_status_code( TsStatus_t status ) {
	TsStatusIndexTuple_t * tuple = _ts_status_tuple( status );
	return tuple->http;
}

const char * ts_status_error( TsStatus_t status ) {
	TsStatusIndexTuple_t * tuple = _ts_status_tuple( status );
	return tuple->error;
}

void ts_status_set_level( TsStatusLevel_t level ) {
	_level = level;
}

TsStatusLevel_t ts_status_get_level() {
	return _level;
}

void ts_status_printf( TsStatusLevel_t level, const char * file, int line, const char * func, const char * format, ... ) {

	if( level >= _level ) {

		uint64_t timestamp = ts_platform_time();
		va_list argp;
		va_start( argp, format );
		char hlevel[40], hfile[40], htick[40];

		// format level
		ts_platform_assert( level >= 0 );
		ts_platform_assert( level < _TsStatusLevelLast );
		snprintf( hlevel, sizeof(hlevel), "%s", _ts_level_to_string[level] );

		// format header
		char * filename = strrchr((char*)file, '/') ? strrchr((char*)file, '/') + 1 : strrchr((char*)file, '\\') ? strrchr((char*)file, '\\') + 1 : (char*)file;
		snprintf( hfile, sizeof(hfile), "%s:%d", filename, line );

		// format tick
		int hours = (int)(timestamp / ( 60L * 60L * TS_TIME_SEC_TO_USEC ) );
		timestamp = timestamp % ( 60L * 60L * TS_TIME_SEC_TO_USEC );
		int minutes = (int)(timestamp / ( 60L * TS_TIME_SEC_TO_USEC ) );
		timestamp = timestamp % ( 60L * TS_TIME_SEC_TO_USEC );
		int seconds = (int)(timestamp / ( 1L * TS_TIME_SEC_TO_USEC ) );
		int useconds = (int)( timestamp % ( 1L * TS_TIME_SEC_TO_USEC ) );
		snprintf( htick, sizeof(htick), "%d:%02d:%02d.%06d", hours, minutes, seconds, useconds );

		// output status message
		ts_platform->printf( "%-5s : [%-21s][%-27s][%-12s] : ", hlevel, htick, hfile, func );
		ts_platform->vprintf( format, argp );
		va_end( argp );
	}
}
