/// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

#include "ts_version.h"
#include "ts_platform.h"

static TsStatus_t _ts_handle_get( TsMessageRef_t fields );

/**
 * Handle a version query message.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_version_handle(TsMessageRef_t message) {

	ts_status_trace("ts_version_handle");
	ts_platform_assert(message != NULL);

	TsStatus_t status;

	char * kind;
	status = ts_message_get_string(message, "kind", &kind);
	if ((status == TsStatusOk) && (strcmp(kind, "ts.event.version") == 0)) {

		char * action;
		status = ts_message_get_string(message, "action", &action);
		if (status == TsStatusOk) {

			TsMessageRef_t fields;
			status = ts_message_get_message(message, "fields", &fields);
			if (status == TsStatusOk) {

				if (strcmp(action, "set") == 0) {

					// this object is read-only
					ts_status_info(
							"ts_version_handle: version info is read-only.\n");
					return TsStatusErrorBadRequest;

				} else if (strcmp(action, "get") == 0) {

					// get the version information
					ts_status_debug(
							"ts_version_handle: delegate to get handler\n");
					status = _ts_handle_get(fields);

				} else {

					ts_status_info(
							"ts_version_handle: message missing valid action.\n");
					status = TsStatusErrorBadRequest;
				}
			} else {

				ts_status_info("ts_version_handle: message missing fields.\n");
				status = TsStatusErrorBadRequest;
			}
		} else {

			ts_status_info("ts_version_handle: message missing action.\n");
			status = TsStatusErrorBadRequest;
		}
	} else {

		ts_status_info("ts_version_handle: message missing correct kind.\n");
		status = TsStatusErrorBadRequest;
	}
	return status;
}

static TsStatus_t _ts_handle_get( TsMessageRef_t fields ) {
	TsMessageRef_t contents;
	if (ts_message_has(fields, "sdk_version", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get sdk_version\n");
		ts_message_set_string(fields, "sdk_version", TS_SDK_VERSION);
	}
	if (ts_message_has(fields, "ods_version", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get ods_version\n");
		ts_message_set_string(fields, "ods_version", TS_ODS_VERSION);
	}
	if (ts_message_has(fields, "hardware_version", &contents) == TsStatusOk) {
		ts_status_debug("_ts_handle_get: get hardware_version\n");
		ts_message_set_string(fields, "hardware_version", TS_HARDWARE_VERSION);
	}
	return TsStatusOk;
}

