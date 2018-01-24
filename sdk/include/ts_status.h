// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_STATUS_H
#define TS_STATUS_H

// TODO - TsProfile (works with diagnostics) vs TsStatus - "report" reports profile information as a TsMessage_t
typedef enum {

	TsStatusStatusMask               = 0x03FF,     // least significant 10-bits

	TsStatusUnknown                  = 0,          // status unknown
	TsStatusOkEnqueue                = 100,        // partial success, subsequent operation is expected (e.g., trying)
	TsStatusOk                       = 200,        // operation complete success

	TsStatusError                    = 400,        // a generic error and the start of the status error section
	TsStatusErrorBadRequest          = 400,        // failed due to wrong parameters values (same value as a generic error)
	TsStatusErrorNotFound            = 404,        // failed due to missing information
	TsStatusErrorPreconditionFailed  = 412,        // failed due to missing or wrong data or parameters
	TsStatusErrorPayloadTooLarge     = 413,        // failed due to large data set(s)
	TsStatusErrorInternalServerError = 500,        // failed due to an unknown critical error (e.g., seg-fault)
	TsStatusErrorNotImplemented      = 501,        // operation not ready for use
	TsStatusErrorBadGateway          = 502,        // gateway connection failed

	// TODO - remove (make monotonically increasing w/lookup table instead)
	TsStatusCodeMask                 = 0xFC00,     // most significant 6-bits

	// TODO - expand for all possible errors - review all cases
	TsStatusErrorOutOfMemory         = (1<<10)+500,  // failed due to soft (detectable) out-of-memory state
	TsStatusErrorIndexOutOfRange     = (2<<10)+500,  // index is out of range
	TsStatusErrorRecursionTooDeep    = (3<<10)+500,  // stack limitation exceeded
	TsStatusErrorExceedTimeBudget    = (4<<10)+500,  // the last operation exceeds the given time budget
	TsStatusErrorNoResourceAvailable = (5<<10)+500,  // there are no hardware resources available for the request
	TsStatusErrorConnectionReset     = (6<<10)+400,  // connection reset

	// NOTE - TsStatusReadPending has a very specific meaning, only return when the read
	//        has returned pending and there isn't data in the buffer, in all other cases
	//        return a valid status with the contents of the current buffer.
	// TODO - should be TsStatusOk...
	TsStatusReadPending              = (7<<10)+100,  // read pending on non-blocking io
	TsStatusWritePending             = (8<<10)+100,  // write pending on non-blocking io

} TsStatus_t;

// TODO - rename values to TsStatusLevel... e.g. TsStatusLevelDebug
typedef enum {
	TsStatusTrace = 0,
	TsStatusDebug,
	TsStatusInfo,
	TsStatusAlarm,
} TsStatusLevel_t;

#ifdef __cplusplus
extern "C" {
#endif

const char *	ts_status_string(TsStatus_t);
int             ts_status_code(TsStatus_t);
int             ts_status_error(TsStatus_t);

void			ts_status_set_level(TsStatusLevel_t);
TsStatusLevel_t	ts_status_get_level();

// TODO - make ts_status private?
void			ts_status(TsStatusLevel_t, const char *, ...);

void			ts_status_trace(const char * format, ...);
void			ts_status_debug(const char * format, ...);
void			ts_status_info(const char * format, ...);
void			ts_status_alarm(const char * format, ...);

#ifdef __cplusplus
}
#endif

#endif // TS_STATUS_H
