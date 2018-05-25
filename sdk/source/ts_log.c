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

static TsStatus_t _ts_handle_set( TsLogConfigRef_t logconfig, TsMessageRef_t fields ) {
	if (ts_message_get_bool(fields, "enabled", &(logconfig->_enabled))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: enabled = %d\n", logconfig->_enabled);
	}
	if (ts_message_get_int(fields, "level", &(logconfig->_level))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: level = %d\n", logconfig->_level);
	}
	if (ts_message_get_int(fields, "max_entries", &(logconfig->_max_entries))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: max_entries = %d\n", logconfig->_max_entries);
	}
	if (ts_message_get_int(fields, "min_interval", &(logconfig->_min_interval))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: min_interval = %d\n", logconfig->_min_interval);
	}
	if (ts_message_get_int(fields, "reporting_interval", &(logconfig->_reporting_interval))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: reporting_interval = %d\n", logconfig->_reporting_interval);
	}
	return TsStatusOk;
}

static TsStatus_t _ts_handle_get( TsLogConfigRef_t logconfig, TsMessageRef_t fields ) {
	TsMessageRef_t contents;
	if (ts_message_has(fields, "enabled", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get enabled\n");
		ts_message_set_bool(fields, "enabled", logconfig->_enabled);
	}
	if (ts_message_has(fields, "level", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get level\n");
		ts_message_set_int(fields, "level", logconfig->_level);
	}
	if (ts_message_has(fields, "max_entries", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get max_entries\n");
		ts_message_set_int(fields, "max_entries", logconfig->_max_entries);
	}
	if (ts_message_has(fields, "min_interval", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get min_interval\n");
		ts_message_set_int(fields, "min_interval", logconfig->_min_interval);
	}
	if (ts_message_has(fields, "reporting_interval", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get reporting_interval\n");
		ts_message_set_int(fields, "reporting_interval", logconfig->_reporting_interval);
	}
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

	TsStatus_t status;

	char * kind;
	status = ts_message_get_string(message, "kind", &kind);
	if ((status == TsStatusOk) && (strcmp(kind, "ts.event.logconfig") == 0)) {

		char * action;
		status = ts_message_get_string(message, "action", &action);
		if (status == TsStatusOk) {

			TsMessageRef_t fields;
			status = ts_message_get_message(message, "fields", &fields);
			if (status == TsStatusOk) {

				if (strcmp(action, "set") == 0) {

					// set or update a rule or domain
					ts_status_debug(
							"ts_logconfig_handle: delegate to set handler\n");
					status = _ts_handle_set(logconfig, fields);

				} else if (strcmp(action, "get") == 0) {

					// get a rule or list of rules
					ts_status_debug(
							"ts_logconfig_handle: delegate to get handler\n");
					status = _ts_handle_get(logconfig, fields);

				} else {

					ts_status_info(
							"ts_logconfig_handle: message missing valid action.\n");
					status = TsStatusErrorBadRequest;
				}
			} else {

				ts_status_info("ts_logconfig_handle: message missing fields.\n");
				status = TsStatusErrorBadRequest;
			}
		} else {

			ts_status_info("ts_logconfig_handle: message missing action.\n");
			status = TsStatusErrorBadRequest;
		}
	} else {

		ts_status_info("ts_logconfig_handle: message missing correct kind.\n");
		status = TsStatusErrorBadRequest;
	}
	return status;
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
