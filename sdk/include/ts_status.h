/**
 * @file
 * ts_status.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * Used for logging and error reporting.
 *
 * @details
 * The entire SDK uses types defined in this file for all errors and debug messages
 * as well as alarming. The error codes shown below are considered a "living" set of
 * errors returned by the SDK back to the server, they may be extended at any time in
 * the future.
 */
#ifndef TS_STATUS_H
#define TS_STATUS_H

// TODO - TsProfile (works with diagnostics) vs TsStatus - "report" reports profile information as a TsMessage_t
typedef enum {

	TsStatusUnknown = 0,                // status unknown

	// Ok
	TsStatusOkTrying,                  // partial success, subsequent tick operation is expected
	TsStatusOk,                         // operation complete success

	// NOTE - TsStatusOkReadPending has a very specific meaning, only return when the read
	//        has returned pending and there isn't data in the buffer, in all other cases
	//        return a valid status with the contents of the current buffer.
	TsStatusOkReadPending,              // read pending on non-blocking io
	TsStatusOkWritePending,             // write pending on non-blocking io

	// Error
	TsStatusError,                      // a generic error and the start of the status error section
	TsStatusErrorBadRequest,            // failed due to wrong parameters values (same value as a generic error)
	TsStatusErrorNotFound,              // failed due to missing information
	TsStatusErrorPreconditionFailed,    // failed due to missing or wrong data or parameters
	TsStatusErrorPayloadTooLarge,       // failed due to large data set(s)
	TsStatusErrorInternalServerError,   // failed due to an unknown critical error (e.g., seg-fault)
	TsStatusErrorNotImplemented,        // operation not ready for use
	TsStatusErrorBadGateway,            // gateway connection failed
	TsStatusErrorOutOfMemory,           // failed due to soft (detectable) out-of-memory state
	TsStatusErrorIndexOutOfRange,       // index is out of range
	TsStatusErrorRecursionTooDeep,      // stack limitation exceeded
	TsStatusErrorExceedTimeBudget,      // the last operation exceeds the given time budget
	TsStatusErrorNoResourceAvailable,   // there are no hardware resources available for the request
	TsStatusErrorConnectionReset,       // connection reset

	// File related
	TsStatusErrorBootError, //
	TsStatusErrorMediaInvalid, //
	TsStatusErrorFatReadError, //
	TsStatusErrorNotAFile, //
	TsStatusErrorAccessError, //
	TsStatusErrorNotOpen, //
	TsStatusErrorFileCorrupt, //
	TsStatusErrorEndOfFile, //
	TsStatusErrorNoMoreSpace, //
	TsStatusErrorAlreadyCreated, //
	TsStatusErrorInvalidName, //
	TsStatusErrorInvalidPath, //
	TsStatusErrorNotDirectory, //
	TsStatusErrorNoMoreEntries, //
	TsStatusErrorDirNotEmpty, //
	TsStatusErrorMediaNotOpened, //
	TsStatusErrorPtrError, //
	TsStatusErrorInvalidAttr, //
	TsStatusErrorCallerError, //
	TsStatusErrorBufferError, //
	TsStatusErrorWriteProtect, //
	TsStatusErrorInvalidOption, //
	TsStatusErrorSectorInvalid, //
	TsStatusErrorIo_Error, //
	TsStatusErrorNotEnoughMemory, //
	TsStatusErrorErrorFixed, //
	TsStatusErrorErrorNotFixed, //
	TsStatusErrorNotAvailable, //
	TsStatusErrorInvalidChecksum, //
	TsStatusErrorReadContinue, //
	TsStatusErrorInvalidState //


} TsStatus_t;

typedef enum {

	TsStatusLevelTrace = 0, // verbose messages
	TsStatusLevelDebug,     // detailed messages
	TsStatusLevelInfo,      // informative (not debug, not alarms) messages
	TsStatusLevelAlarm,     // alarm message

	_TsStatusLevelLast,      // not a message level, used for index limits
} TsStatusLevel_t;

#ifdef __cplusplus
extern "C" {
#endif

const char *	ts_status_string(TsStatus_t);
int             ts_status_code(TsStatus_t);
const char *    ts_status_error(TsStatus_t);

void			ts_status_set_level(TsStatusLevel_t);
TsStatusLevel_t	ts_status_get_level();
void            ts_status_printf( TsStatusLevel_t, const char *, int, const char *, const char *, ... );

#ifdef NDEBUG
#define ts_status_trace(...)    (void)0
#define ts_status_debug(...)    (void)0
#define ts_status_info(...)     (void)0
#define ts_status_alarm(...)    (void)0
#else
#define ts_status_trace(...)    do { ts_status_printf( TsStatusLevelTrace, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } while(0)
#define ts_status_debug(...)    do { ts_status_printf( TsStatusLevelDebug, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } while(0)
#define ts_status_info(...)     do { ts_status_printf( TsStatusLevelInfo, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } while(0)
#define ts_status_alarm(...)    do { ts_status_printf( TsStatusLevelAlarm, __FILE__, __LINE__, __func__, __VA_ARGS__ ); } while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif // TS_STATUS_H
