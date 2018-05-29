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
	(*logconfig)->_min_interval = 1000;
	(*logconfig)->_reporting_interval = 3600;

	// Allocate some space for messages
	_ts_log_create(*logconfig, 100);

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
	int new_max_entries;
	if (ts_message_get_bool(fields, "enabled", &(logconfig->_enabled))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: enabled = %d\n", logconfig->_enabled);
	}
	if (ts_message_get_int(fields, "level", &(logconfig->_level))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: level = %d\n", logconfig->_level);
	}
	if (ts_message_get_int(fields, "max_entries", &(new_max_entries))
			== TsStatusOk) {
		ts_status_debug("_ts_handle_set: max_entries = %d\n", new_max_entries);
		if (new_max_entries > 0) {
			_ts_log_resize(logconfig, new_max_entries);
		}
		else {
			return TsStatusErrorBadRequest;
		}
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
 * [in] Category of message: 0 = security_profile, 1 = firewall, 2 = credential, 3 = diagnostic
 * @param message
 * [in] The text of the log message.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_log(TsLogConfigRef_t log, TsLogLevel_t level, TsLogCategory_t category, char *message) {
	ts_status_trace("ts_log");
	ts_status_debug("ts_log: level = %d, category = %d, message = %s\n", level, category, message);
	ts_platform_assert(level >= 0 && level < _TsLogLevelLast);
	ts_platform_assert(log != NULL);
	ts_platform_assert(category >= 0 && category < _TsCategoryLast);
	ts_platform_assert(message != NULL);

	if (!(log->_enabled) || log->_max_entries < 1) {
		// We're not logging at all
		return TsStatusOk;
	}

	if (level < log->_level) {
		// We're not recording messages of this level
		return TsStatusOk;
	}

	uint64_t time = ts_platform_time();

	// Check to see if we're spamming identical messages too frequently
	if (log->_newest >= log->_start) {
		uint64_t previous = log->_newest->time;
		if ((time - previous > 0) && (time - previous < log->_min_interval)) {
			if (log->_newest->level == level && log->_newest->category == category) {
				if (strncmp(log->_newest->body, message, LOG_MESSAGE_MAX_LENGTH) == 0) {
					ts_status_debug("ts_log: Log messages identical within interval %d\n", log->_min_interval);
					return TsStatusOk;
				}
			}
		}
	}

	// Can we get the space for the message body?
	int length = strnlen(message, LOG_MESSAGE_MAX_LENGTH);
	char *new_body = (char *)platform_malloc(length);
	if (new_body == NULL) {
		return TsStatusErrorOutOfMemory;
	}

	// We're good; record away
	strncpy(new_body, message, LOG_MESSAGE_MAX_LENGTH);

	TsLogEntryRef_t new = log->_newest + 1;
	if (new >= log->_start + log->_max_entries) {
		// Circular log with a hard limit
		new = log->_start;
	}

	new->category = category;
	new->level = level;
	new->time = time;

	char * old_body = new->body;
	new->body = new_body;

	log->_newest = new;

	if (new >= log->_end) {
		// We are expanding the actual size of the log; update the end vector
		log->_end = new + 1;
	}
	else {
		// There was a message here before; free its body
		platform_free(old_body);
	}
	return TsStatusOk;
}

/**
 * Allocate space for the log entries (not for the bodies).
 * @param start Pointer to the TsLogEntryRef_t that will contain the first entry.
 * @param max_entries Initial number of entries to allocate.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t _ts_log_alloc(TsLogEntryRef_t *start, int max_entries) {
	ts_platform_assert(max_entries > 0);
	*start = (TsLogEntryRef_t) platform_malloc(sizeof(TsLogEntry_t) * max_entries);
	if (*start == NULL) {
		return TsStatusErrorOutOfMemory;
	}
	return TsStatusOk;
}

TsStatus_t _ts_log_create(TsLogConfigRef_t log, int new_max_entries) {
	ts_platform_assert(new_max_entries > 0);
	TsStatus_t result = _ts_log_alloc(&(log->_start), new_max_entries);
	if (result != TsStatusOk) {
		return result;
	}
	// TODO: create persistent storage?
	log->_max_entries = new_max_entries;
	log->_end = log->_start;
	log->_newest = log->_start - 1; // back this up so the next message will be first
	return TsStatusOk;
}

void _ts_log_shallow_copy(TsLogEntryRef_t src, TsLogEntryRef_t dest) {
	src->time = dest->time;
	src->level = dest->level;
	src->category = dest->category;
	src->body = dest->body;
}

TsStatus_t _ts_log_resize(TsLogConfigRef_t log, int new_max_entries) {
	ts_platform_assert(new_max_entries > 0);
	if (new_max_entries == log->_max_entries) {
		return TsStatusOk;
	}

	TsLogEntryRef_t new;
	TsStatus_t result = _ts_log_alloc(&new, new_max_entries);
	if (result != TsStatusOk) {
		return result;
	}
	// TODO: resize persistent storage?
	int old_entries = log->_end - log->_start;
	if (old_entries > new_max_entries) {
		// We need to truncate the old list.
		// Work backwards from the newest entry,
		// and fill the new list starting at the very end.

		TsLogEntryRef_t new_current = new + new_max_entries - 1;
		TsLogEntryRef_t old_current = log->_newest;

		for (; new_current >= new; new_current--, old_current--) {
			if (old_current < log->_start) {
				old_current = log->_end - 1;
			}
			_ts_log_shallow_copy(old_current, new_current);
		}
		// Free the remaining old message bodies.
		for (; old_current != log->_newest; old_current--) {
			if (old_current < log->_start) {
				old_current = log->_end - 1;
			}
			platform_free(old_current->body);
		}
		// Set the new log parameters. The log starts out full.
		TsLogEntryRef_t old_start = log->_start;

		log->_max_entries = new_max_entries;
		log->_start = new;
		log->_end = new + new_max_entries;
		log->_newest = log->_end - 1;

		platform_free(old_start);
	} else {
		// There is enough room to hold all the old messages.
		// Work forwards from the oldest entry,
		// and fill the new list starting from the beginning.

		TsLogEntryRef_t new_current = new;
		TsLogEntryRef_t old_current = log->_newest + 1;
		int i;
		for (i = 0; i < old_entries; i++, new_current++, old_current++) {
			if (old_current == log->_end) {
				old_current = log->_start;
			}
			_ts_log_shallow_copy(old_current, new_current);
		}

		// Set the new log parameters. The log is not necessarily full.
		TsLogEntryRef_t old_start = log->_start;

		log->_max_entries = new_max_entries;
		log->_start = new;
		log->_end = new + old_entries;
		log->_newest = log->_end - 1;

		platform_free(old_start);
	}

	return TsStatusOk;
}
