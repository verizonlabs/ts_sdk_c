// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#include "ts_platforms.h"
#include "ts_components.h"

#include "ts_connection.h"

// must compile with,...
//
// opt TS_TRANSPORT_CUSTOM (i.e., no transport)
// TS_SECURITY_NONE
// opt TS_CONTROLLER_SOCKET
// opt TS_PLATFORM_UNIX
#if defined(TS_SECURITY_NONE)

int main() {

	TsStatus_t status;
	ts_status_set_level(TsStatusDebug);

	// create a connection state struct
	TsConnectionRef_t connection;
	status = ts_connection_create(&connection);
	if (status != TsStatusOk) {
		ts_status_debug("failed create, '%s'\n", ts_status_string(status));
		return 0;
	}

	// connect
	status = ts_connection_connect(connection, "www.google.com:80");
	if (status != TsStatusOk) {
		ts_status_debug("failed connect, '%s'\n", ts_status_string(status));
		return 0;
	}

	// write
	const char *get_root = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
	size_t get_root_size = strlen(get_root);
	status = ts_connection_write(connection, (uint8_t *) get_root, &get_root_size, 50);
	if( status != TsStatusOk ) {
		ts_status_debug("failed write, '%s'\n", ts_status_string(status));
		return 0;
	}

	// read
	uint8_t buffer[4097];
	bool reading = true;
	while (reading) {

		memset(buffer, 0x00, sizeof(buffer));
		size_t buffer_size = sizeof(buffer) - 1;
		status = ts_connection_read(connection, buffer, &buffer_size, 50);
		switch( status ) {
		case TsStatusOk:
			if( buffer_size > 0 ) {
				buffer[buffer_size] = 0x00;
				ts_status_info("%s\n", (char*)buffer);
			}
			// fallthrough
		case TsStatusReadPending:
			// read-pending should never have data returned
			break;
		default:
			ts_status_info("failed read, %s\n", ts_status_string(status));
			reading = false;
			break;
		}
	}

	// close and clean-up
	// TODO - close and free all
}

#else

int main() {
	ts_status_alarm("missing one or many components, please check compile directives and build again\n");
}

#endif
