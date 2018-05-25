// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

#include "ts_platform.h"
#include "ts_log.h"

/**
 * Create a log configuration object.
 * @param logconfig
 * [on/out] Pointer to a TsLogConfigRef_t in which the new config will be stored.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_create(TsLogConfigRef_t *logconfig) {
	ts_status_trace("ts_logconfig_create");
	ts_platform_assert(logconfig != NULL);
	*logconfig = (TsLogConfigRef_t)ts_platform_malloc(sizeof(TsLogConfig_t));
	(*logconfig)->_enabled = false;
	(*logconfig)->_level = 0;
	(*logconfig)->_max_entries = 100;
	(*logconfig)->_min_interval = 1000;
	(*logconfig)->_reporting_interval = 3600;
	return TsStatusOk;
}

/**
 * Destroy a log configuration object.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log config to be destroyed.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_destroy(TsLogConfigRef_t logconfig) {
	ts_status_trace("ts_logconfig_destroy");
	ts_platform_assert(logconfig != NULL);
	ts_platform_free(logconfig);
	return TsStatusOk;
}

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
TsStatus_t ts_logconfig_handle(TsLogConfigRef_t logconfig, TsMessageRef_t message) {
	ts_status_trace("ts_logconfig_handle");
	ts_platform_assert(logconfig != NULL);
	ts_platform_assert(message != NULL);
	return TsStatusOk;
}

/**
 * Provide the log config with time according to the time budget recommendation.
 * This is typically called from ts_service. It may be used to send log messages.
 * @param logconfig
 * [in] TsLogConfigRef_t representing the log config to be provided with time.
 * @param budget
 * [in] Recommended time budget in microseconds.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_logconfig_tick(TsLogConfigRef_t logconfig, uint32_t budget) {
	ts_status_trace("ts_logconfig_tick");
	ts_platform_assert(logconfig != NULL);
	return TsStatusOk;
}


/**
 * Attempt to record a log message. (It will only be recorded if log level
 * and time/space parameters allow.)
 * @param log
 * [in] TsLogConfigRef_t representing the log.
 * @param level
 * [in] Logging level. 0 = info, 1 = warning, 2 = error, 3 = alert.
 * @param category
 * [in] Category of message: "security_profile", "firewall", "credential", "diagnostic"
 * @param message
 * [in] The text of the log message.
 */
TsStatus_t ts_log(TsLogConfigRef_t log, TsLogLevel_t level, char *category, char *message) {
	ts_status_trace("ts_log");
	ts_platform_assert(level >= 0 && level < _TsLogLevelLast);
	ts_platform_assert(log != NULL);
	ts_platform_assert(category != NULL);
	ts_platform_assert(message != NULL);

	if (level < log->_level) {
		// We're not recording messages of this level
		return TsStatusOk;
	}
	return TsStatusOk;
}
