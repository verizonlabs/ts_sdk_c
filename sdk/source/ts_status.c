// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_status.h"

typedef struct {
	int status;
	const char *string;
} IndexToString_t;

// TODO - reorg so that id's are monotonically incressing, and status-code, error-code and message are looked-up via array (by id).

static IndexToString_t _status_to_string[] = {
	{TsStatusUnknown, "status unknown"},
	{TsStatusOkEnqueue, "partial success, subsequent operation is expected (e.g., trying)"},
	{TsStatusOk, "ok"},
	{TsStatusErrorBadRequest, "failed due to wrong parameters values (same value as a generic error)"},
	{TsStatusErrorNotFound, "failed due to missing information"},
	{TsStatusErrorPreconditionFailed, "failed due to missing or wrong data or parameters"},
	{TsStatusErrorPayloadTooLarge, "failed due to large data set(s)"},
	{TsStatusErrorInternalServerError, "failed due to an unknown critical error (e.g., seg-fault)"},
	{TsStatusErrorBadGateway, "connection failed from gateway"},
	{TsStatusErrorNotImplemented, "operation not ready for use"},
	{TsStatusErrorOutOfMemory, "failed due to soft (detectable) out-of-memory state"},
	{TsStatusErrorIndexOutOfRange, "index is out of range"},
	{TsStatusErrorRecursionTooDeep, "failed due to stack limitations"},
	{TsStatusErrorExceedTimeBudget, "the last operation exceeded the given time budget"},
	{TsStatusErrorConnectionReset, "connection reset"},
	{TsStatusReadPending, "pending read expected"},
	{TsStatusWritePending, "pending write expected"},
};
static int _status_to_string_length = sizeof(_status_to_string) / sizeof(IndexToString_t);

static TsStatusLevel_t _level = TsStatusAlarm;

const char * ts_status_string(TsStatus_t status)
{
	for( int i = 0; i < _status_to_string_length; i++ ) {
		if( _status_to_string[i].status == status ) {
			return _status_to_string[i].string;
		}
	}
	return _status_to_string[0].string;
}

void ts_status_set_level(TsStatusLevel_t level) {
	_level = level;
}

TsStatusLevel_t ts_status_get_level() {
	return _level;
}

void ts_status(TsStatusLevel_t level, const char * format, ...) {
	if( level >= _level) {
		va_list argp;
		va_start(argp, format);
		ts_platform->vprintf(format, argp);
		va_end(argp);
	}
}

// TODO - allow compiler directive to include or not

void ts_status_trace(const char * format, ...) {
	if( TsStatusTrace >= _level) {
		va_list argp;
		va_start(argp, format);
		ts_platform->vprintf(format, argp);
		va_end(argp);
	}
}

// TODO - allow compiler directive to include or not

void ts_status_debug(const char * format, ...) {
	if( TsStatusDebug >= _level) {
		va_list argp;
		va_start(argp, format);
		ts_platform->vprintf(format, argp);
		va_end(argp);
	}
}

void ts_status_info(const char * format, ...) {
	if( TsStatusInfo >= _level) {
		va_list argp;
		va_start(argp, format);
		ts_platform->vprintf(format, argp);
		va_end(argp);
	}
}

void ts_status_alarm(const char * format, ...) {
	if( TsStatusAlarm >= _level) {
		va_list argp;
		va_start(argp, format);
		ts_platform->vprintf(format, argp);
		va_end(argp);
	}
}