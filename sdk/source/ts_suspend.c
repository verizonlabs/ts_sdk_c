/// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

#include "ts_suspend.h"
#include "ts_platform.h"
#include "ts_log.h"
#include "ts_firewall.h"

static TsStatus_t _ts_handle_get( TsMessageRef_t fields );
static TsStatus_t _ts_handle_set( TsMessageRef_t fields );

static TsFirewallRef_t _firewall = NULL;
static TsLogConfigRef_t _logconfig = NULL;

#define SUSPEND_LOG(s) if (_logconfig != NULL) { ts_log(_logconfig, TsLogLevelInfo, TsCategoryDiagnostic, s); }

/**
 * Set the logconfig used to log suspension events, and that we'll be suspending/resuming.
 * @param config
 * [in] The log configuration to use for logging suspension events.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_set_logconfig(TsLogConfigRef_t config) {
	_logconfig = config;
	return TsStatusOk;
}


/**
 * Set the firewall that we'll be suspending/resuming.
 * @param firewall
 * [in] The firewall that we'll be suspending/resuming.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_set_firewall(TsFirewallRef_t firewall) {
	_firewall = firewall;
	return TsStatusOk;
}

#ifdef TEST_SUSPEND
/**
 * Generate and handle a test suspension message.
 * @param firewall true to suspend firewall, false to resume
 * @param logconfig true to suspend logging, false to resume
 * @return
 */
TsStatus_t ts_suspend_test(bool firewall, bool logconfig) {
	TsMessageRef_t testMessage;
	ts_message_create(&testMessage);
	ts_message_set_string(testMessage, "action", "update");
	ts_message_set_string(testMessage, "kind", "ts.event.suspend");
	TsMessageRef_t fields;
	ts_message_create_message(testMessage, "fields", &fields);
	ts_message_set_bool(fields, "firewall", firewall);
	ts_message_set_bool(fields, "logconfig", logconfig);

	TsStatus_t result = ts_suspend_handle(testMessage);
	if (result == TsStatusOk) {
		ts_status_debug("ts_handle_test_suspension: successfully handled message with firewall = %d, logconfig = %d\n");
	} else {
		ts_status_debug("ts_handle_test_suspension: handle returned error, %s\n", ts_status_string(result));
	}

	ts_message_destroy(testMessage);
	return result;
}
#endif /*TEST_SUSPEND*/

/**
 * Handle a suspension message.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_handle(TsMessageRef_t message) {

	ts_status_trace("ts_suspend_handle");
	ts_platform_assert(message != NULL);

	TsStatus_t status;

	char * kind;
	status = ts_message_get_string(message, "kind", &kind);
	if ((status == TsStatusOk) && (strcmp(kind, "ts.event.suspend") == 0)) {

		char * action;
		status = ts_message_get_string(message, "action", &action);
		if (status == TsStatusOk) {

			TsMessageRef_t fields;
			status = ts_message_get_message(message, "fields", &fields);
			if (status == TsStatusOk) {

				if (strcmp(action, "set") == 0) {

					// set the suspend fields
					ts_status_info(
							"ts_suspend_handle: delegate to set handler\n");
					return _ts_handle_set(fields);

				} else if (strcmp(action, "get") == 0) {

					// get the suspend information
					ts_status_debug(
							"ts_suspend_handle: delegate to get handler\n");
					status = _ts_handle_get(fields);

				} else {

					ts_status_info(
							"ts_suspend_handle: message missing valid action.\n");
					status = TsStatusErrorBadRequest;
				}
			} else {

				ts_status_info("ts_suspend_handle: message missing fields.\n");
				status = TsStatusErrorBadRequest;
			}
		} else {

			ts_status_info("ts_suspend_handle: message missing action.\n");
			status = TsStatusErrorBadRequest;
		}
	} else {

		ts_status_info("ts_suspend_handle: message missing correct kind.\n");
		status = TsStatusErrorBadRequest;
	}
	return status;
}

static TsStatus_t _ts_handle_get( TsMessageRef_t fields ) {
	TsMessageRef_t contents;
	if (ts_message_has(fields, "firewall", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get firewall suspend\n");
		ts_message_set_bool(fields, "firewall", ts_firewall_suspended(_firewall));
	}
	if (ts_message_has(fields, "logconfig", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get logconfig suspend\n");
		ts_message_set_bool(fields, "logconfig", ts_logconfig_suspended(_logconfig));
	}
	return TsStatusOk;
}

static TsStatus_t _ts_handle_set( TsMessageRef_t fields ) {
	bool firewall_suspend;
	if (ts_message_get_bool(fields, "firewall", &firewall_suspend) == TsStatusOk) {
		if (firewall_suspend) {
			ts_status_debug("_ts_handle_set: suspending firewall\n");
			SUSPEND_LOG("Firewall suspended\n");
		}
		else {
			ts_status_debug("_ts_handle_set: resuming firewall\n");
			SUSPEND_LOG("Firewall resumed\n");
		}
		ts_firewall_set_suspended(_firewall, firewall_suspend);
	}
	bool log_suspend;
	if (ts_message_get_bool(fields, "logconfig", &log_suspend) == TsStatusOk) {
		if (log_suspend) {
			ts_status_debug("_ts_handle_set: suspending logging\n");
			// log before suspending the logging
			SUSPEND_LOG("Logging suspended\n");
			ts_logconfig_set_suspended(_logconfig, log_suspend);
		}
		else {
			ts_status_debug("_ts_handle_set: resuming logging\n");
			// log after resuming the logging
			ts_logconfig_set_suspended(_logconfig, log_suspend);
			SUSPEND_LOG("Logging resumed\n");
		}
	}
	return TsStatusOk;
}

