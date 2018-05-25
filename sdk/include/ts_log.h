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


/**
 * The log config reference
 */
typedef struct TsLogConfig * TsLogConfigRef_t;

/**
 * The log config object.
 */

typedef struct TsLogConfig {
	bool _enabled;
	int _level;
	int _max_entries;  			// maximum number of entries in the log
	int _min_interval; 			// minimum interval between repeated identical messages, in milliseconds
	int _reporting_interval; 	// interval between logs reported back to the platform, in seconds
} TsLogConfig_t;

typedef enum {
	TsLogLevelInfo = 0, // informational messages
	TsLogLevelWarning,  // warning messages
	TsLogLevelError,    // error messages
	TsLogLevelAlert,    // firewall alert messages
	_TsLogLevelLast,    // not a level, used for index limits
} TsLogLevel_t;

/**
 * Create a log configuration object.
 * @param logconfig
 * [on/out] Pointer to a TsLogConfigRef_t in which the new config will be stored.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_create(TsLogConfigRef_t *);

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
 * [in] Category of message: "security_profile", "firewall", "credential", "diagnostic"
 * @param message
 * [in] The text of the log message.
 */
TsStatus_t ts_log(TsLogConfigRef_t log, int level, char *category, char *message);

#endif /* TS_LOG_H */
