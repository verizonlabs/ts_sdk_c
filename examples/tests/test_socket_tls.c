// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#ifndef MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PEM_PARSE_C
#endif

#include "mbedtls/certs.h"

#include "ts_platforms.h"
#include "ts_components.h"

#include "ts_connection.h"

// must compile with,...
//
// opt TS_TRANSPORT_CUSTOM (i.e., no transport)
// TS_SECURITY_MBED
// opt TS_CONTROLLER_SOCKET
// opt TS_PLATFORM_UNIX
#if defined(TS_SECURITY_MBED) || defined(TS_SECURITY_MOCANA)

int main() {

	TsStatus_t status;
	ts_status_set_level(TsStatusDebug);

	// create a connection state struct
	TsConnectionRef_t connection;
	status = ts_connection_create(&connection);
	if (status != TsStatusOk) {
		ts_status_info("failed create, '%s'\n", ts_status_string(status));
		return 0;
	}

	// set ca-cert
	ts_connection_set_server_cert_hostname(connection, "www.google.com" );
	ts_connection_set_server_cert(connection, (const uint8_t *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);

	// connect
	status = ts_connection_connect(connection, "www.google.com:443");
	if (status != TsStatusOk) {
		ts_status_info("failed connect, '%s'\n", ts_status_string(status));
		return 0;
	}

	// write
	const char *get_root = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
	size_t get_root_size = strlen(get_root);
	status = ts_connection_write(connection, (uint8_t *) get_root, &get_root_size, 50);
	if( status != TsStatusOk ) {
		ts_status_info("failed write, '%s'\n", ts_status_string(status));
		return 0;
	}

	// read
	ts_status_info("begin reading\n");
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
			break;
		case TsStatusReadPending:
			// read-pending should never have data returned
			if( buffer_size > 0 ) {
				ts_status_info("unexpected data contained in pending-read, %d\n", buffer_size);
			}
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
