/// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

#include "ts_suspend.h"
#include "ts_platform.h"

static TsStatus_t _ts_handle_get( TsMessageRef_t fields );
static TsStatus_t _ts_handle_set( TsMessageRef_t fields );

static bool _suspend_firewall = false;
static bool _suspend_logconfig = false;

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

					// this object is read-only
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
		ts_message_set_bool(fields, "firewall", _suspend_firewall);
	}
	if (ts_message_has(fields, "logconfig", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get logconfig suspend\n");
		ts_message_set_bool(fields, "logconfig", _suspend_logconfig);
	}
	return TsStatusOk;
}

static TsStatus_t _ts_handle_set( TsMessageRef_t fields ) {
	if (ts_message_get_bool(fields, "firewall", &_suspend_firewall) == TsStatusOk) {
		if (_suspend_firewall) {
			ts_status_debug("_ts_handle_set: suspending firewall\n");
		}
		else {
			ts_status_debug("_ts_handle_set: resuming firewall\n");
		}
	}
	if (ts_message_get_bool(fields, "logconfig", &_suspend_logconfig) == TsStatusOk) {
		if (_suspend_logconfig) {
			ts_status_debug("_ts_handle_set: suspending logging\n");
		}
		else {
			ts_status_debug("_ts_handle_set: resuming logging\n");
		}
	}
	return TsStatusOk;
}

bool ts_firewall_suspended() {
	return _suspend_firewall;
}

bool ts_logconfig_suspended() {
	return _suspend_logconfig;
}
