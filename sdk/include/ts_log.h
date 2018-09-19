/**
 * @file
 * ts_log.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * An interface for storing and transmitting log information on the device.
 * Rather than programmers' trace/debug logging, this is intended for logging security and
 * configuration information of interest to the user/admin (including information on firewall
 * violations), and transmitting it to the platform.
 *
 * @details
 * The "log" connectivity service interface is used by licenced ThingSpace devices to manage the
 * local storage of logging information from the cloud.
 * Implementation is specific to the OS and/or hardware platform.
 *
 */

#ifndef TS_LOG_H
#define TS_LOG_H

#include "ts_status.h"
#include "ts_message.h"

//#define TEST_LOGGING 1
//#define DEBUG_MEMORY_SIZES 1

/**
 * The log config reference
 */
typedef struct TsLogConfig * TsLogConfigRef_t;

typedef enum {
	TsLogLevelInfo = 0, // informational messages
	TsLogLevelWarning,  // warning messages
	TsLogLevelError,    // error messages
	TsLogLevelAlert,    // firewall alert messages
	_TsLogLevelLast,    // not a level, used for index limits
} TsLogLevel_t;

typedef enum {
	TsCategorySecurityProfile = 0, 	// security profile events--not used on device side
	TsCategoryFirewall, 			// firewall alerts and changes
	TsCategoryCredential, 			// device credential events
	TsCategoryDiagnostic,			// diagnostic messages
	_TsCategoryLast,				// not a level, used for index limits
} TsLogCategory_t;

/**
 * A single log entry.
 */
typedef struct TsLogEntry {
	uint64_t time;				// timestamp of the log entry
	TsLogLevel_t level;			// log level
	TsLogCategory_t category;	// message category
	char *body;					// message body
} TsLogEntry_t;

typedef struct TsLogEntry * TsLogEntryRef_t;

#define LOG_MESSAGE_MAX_LENGTH 80
#define REPORT_LENGTH 5

/**
 * The log config object.
 */

typedef struct TsLogConfig {
	bool _enabled;
	bool _suspended;
	bool _log_in_progress;
	int _level;
	int _max_entries;  			// maximum number of entries in the log
	int _min_interval; 			// minimum interval between repeated identical messages, in milliseconds
	int _reporting_interval; 	// interval between logs reported back to the platform, in seconds
	TsLogEntryRef_t _start;		// first entry in memory
	TsLogEntryRef_t _newest;	// newest entry (oldest one, if it exists, should be after this)
	TsLogEntryRef_t _end;		// after the last entry in memory
	uint64_t _last_log_time;	// the last time we recorded a log message
	uint64_t _last_report_time;	// the last time we reported logs back to the provider
	TsStatus_t (*_messageCallback)(TsMessageRef_t, char *);	// pointer to function for sending the log message
} TsLogConfig_t;

/**
 * Create a log configuration object.
 * @param logconfig
 * [on/out] Pointer to a TsLogConfigRef_t in which the new config will be stored.
 * @param messageCallback
 * [in] Pointer to a function that will send a TsMessage with a specified kind.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_create(TsLogConfigRef_t *, TsStatus_t (*messageCallback)(TsMessageRef_t, char *));

/**
 * Destroy a log configuration object.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log config to be destroyed.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_destroy(TsLogConfigRef_t);

/**
 * Handle a log configuration message.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log config to be modified/queried.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_handle(TsLogConfigRef_t, TsMessageRef_t);

/**
 * Provide the log config with time according to the time budget recommendation.
 * This is typically called from ts_service. It may be used to send log messages.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log config to be provided with time.
 * @param budget
 * [in] Time budget in microseconds.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_tick(TsLogConfigRef_t, uint32_t);

/**
 * Attempt to record a log message.
 * @param log
 * [in] TsLogConfigRef_t representing the log.
 * @param level
 * [in] Logging level. 0 = info, 1 = warning, 2 = error, 3 = alert.
 * @param category
 * [in] Category of message: 0 = security_profile, 1 = firewall, 2 = credential, 3 = diagnostic
 * @param message
 * [in] The text of the log message.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_log(TsLogConfigRef_t log, TsLogLevel_t level, TsLogCategory_t category, char *message);

/**
 * Suspend or resume logging.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log.
 * @param suspended
 * [in] Whether to suspend or resume logging.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_set_suspended(TsLogConfigRef_t logconfig, bool suspended);

/**
 * Is logging suspended?
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log.
 * @return
 * True if logging is suspended, false otherwise.
 */
bool ts_logconfig_suspended(TsLogConfigRef_t);


/**
 * Set the logconfig used to log suspension events, and that we'll be suspending/resuming.
 * @param logconfig
 * [in] The log configuration to use for logging suspension events.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_set_logconfig(TsLogConfigRef_t);

#endif /* TS_LOG_H */
