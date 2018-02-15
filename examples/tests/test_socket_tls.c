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


#define zTEST_CA_CRT_EC                                                  \
"-----BEGIN CERTIFICATE-----\r\n"                                       \
"MIICUjCCAdegAwIBAgIJAMFD4n5iQ8zoMAoGCCqGSM49BAMCMD4xCzAJBgNVBAYT\r\n"  \
"Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\r\n"  \
"QyBDQTAeFw0xMzA5MjQxNTQ5NDhaFw0yMzA5MjIxNTQ5NDhaMD4xCzAJBgNVBAYT\r\n"  \
"Ak5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UEAxMTUG9sYXJzc2wgVGVzdCBF\r\n"  \
"QyBDQTB2MBAGByqGSM49AgEGBSuBBAAiA2IABMPaKzRBN1gvh1b+/Im6KUNLTuBu\r\n"  \
"ww5XUzM5WNRStJGVOQsj318XJGJI/BqVKc4sLYfCiFKAr9ZqqyHduNMcbli4yuiy\r\n"  \
"aY7zQa0pw7RfdadHb9UZKVVpmlM7ILRmFmAzHqOBoDCBnTAdBgNVHQ4EFgQUnW0g\r\n"  \
"JEkBPyvLeLUZvH4kydv7NnwwbgYDVR0jBGcwZYAUnW0gJEkBPyvLeLUZvH4kydv7\r\n"  \
"NnyhQqRAMD4xCzAJBgNVBAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEcMBoGA1UE\r\n"  \
"AxMTUG9sYXJzc2wgVGVzdCBFQyBDQYIJAMFD4n5iQ8zoMAwGA1UdEwQFMAMBAf8w\r\n"  \
"CgYIKoZIzj0EAwIDaQAwZgIxAMO0YnNWKJUAfXgSJtJxexn4ipg+kv4znuR50v56\r\n"  \
"t4d0PCu412mUC6Nnd7izvtE2MgIxAP1nnJQjZ8BWukszFQDG48wxCCyci9qpdSMv\r\n"  \
"uCjn8pwUOkABXK8Mss90fzCfCEOtIA==\r\n"                                  \
"-----END CERTIFICATE-----\r\n"

#define zTEST_CA_CRT_RSA                                                 \
"-----BEGIN CERTIFICATE-----\r\n"                                       \
"MIIDhzCCAm+gAwIBAgIBADANBgkqhkiG9w0BAQUFADA7MQswCQYDVQQGEwJOTDER\r\n"  \
"MA8GA1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwHhcN\r\n"  \
"MTEwMjEyMTQ0NDAwWhcNMjEwMjEyMTQ0NDAwWjA7MQswCQYDVQQGEwJOTDERMA8G\r\n"  \
"A1UEChMIUG9sYXJTU0wxGTAXBgNVBAMTEFBvbGFyU1NMIFRlc3QgQ0EwggEiMA0G\r\n"  \
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDA3zf8F7vglp0/ht6WMn1EpRagzSHx\r\n"  \
"mdTs6st8GFgIlKXsm8WL3xoemTiZhx57wI053zhdcHgH057Zk+i5clHFzqMwUqny\r\n"  \
"50BwFMtEonILwuVA+T7lpg6z+exKY8C4KQB0nFc7qKUEkHHxvYPZP9al4jwqj+8n\r\n"  \
"YMPGn8u67GB9t+aEMr5P+1gmIgNb1LTV+/Xjli5wwOQuvfwu7uJBVcA0Ln0kcmnL\r\n"  \
"R7EUQIN9Z/SG9jGr8XmksrUuEvmEF/Bibyc+E1ixVA0hmnM3oTDPb5Lc9un8rNsu\r\n"  \
"KNF+AksjoBXyOGVkCeoMbo4bF6BxyLObyavpw/LPh5aPgAIynplYb6LVAgMBAAGj\r\n"  \
"gZUwgZIwDAYDVR0TBAUwAwEB/zAdBgNVHQ4EFgQUtFrkpbPe0lL2udWmlQ/rPrzH\r\n"  \
"/f8wYwYDVR0jBFwwWoAUtFrkpbPe0lL2udWmlQ/rPrzH/f+hP6Q9MDsxCzAJBgNV\r\n"  \
"BAYTAk5MMREwDwYDVQQKEwhQb2xhclNTTDEZMBcGA1UEAxMQUG9sYXJTU0wgVGVz\r\n"  \
"dCBDQYIBADANBgkqhkiG9w0BAQUFAAOCAQEAuP1U2ABUkIslsCfdlc2i94QHHYeJ\r\n"  \
"SsR4EdgHtdciUI5I62J6Mom+Y0dT/7a+8S6MVMCZP6C5NyNyXw1GWY/YR82XTJ8H\r\n"  \
"DBJiCTok5DbZ6SzaONBzdWHXwWwmi5vg1dxn7YxrM9d0IjxM27WNKs4sDQhZBQkF\r\n"  \
"pjmfs2cb4oPl4Y9T9meTx/lvdkRYEug61Jfn6cA+qHpyPYdTH+UshITnmp5/Ztkf\r\n"  \
"m/UTSLBNFNHesiTZeH31NcxYGdHSme9Nc/gfidRa0FLOCfWxRlFqAI47zG9jAQCZ\r\n"  \
"7Z2mCGDNMhjQc+BYcdnl0lPXjdDK6V0qCg1dVewhUBcW5gZKzV7e9+DpVA==\r\n"      \
"-----END CERTIFICATE-----\r\n"

const char zmbedtls_test_cas_pem[] = zTEST_CA_CRT_RSA zTEST_CA_CRT_EC;
const size_t zmbedtls_test_cas_pem_len = sizeof( zmbedtls_test_cas_pem );

int main() {

	TsStatus_t status;
	ts_status_set_level(TsStatusLevelDebug);

	// create a connection state struct
	TsConnectionRef_t connection;
	status = ts_connection_create(&connection);
	if (status != TsStatusOk) {
		ts_status_info("failed create, '%s'\n", ts_status_string(status));
		return 0;
	}

	// set ca-cert
	ts_connection_set_server_cert_hostname(connection, "www.google.com" );
	// TODO - removed in trimmed down version of mbedTLS
	ts_connection_set_server_cert(connection, (const uint8_t *)zmbedtls_test_cas_pem, zmbedtls_test_cas_pem_len);

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
		case TsStatusOkReadPending:
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
