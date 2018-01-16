// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platforms.h"
#include "ts_components.h"

#include "ts_transport.h"

#include "cacert.h"
#include "client-crt.h"
#include "client-key.h"

// must compile with,...
//
// TS_TRANSPORT_MQTT
// TS_SECURITY_MBED or MOCANA
// opt TS_CONTROLLER_SOCKET
// opt TS_PLATFORM_UNIX
#if defined(TS_TRANSPORT_MQTT) && ( defined(TS_SECURITY_MBED) || defined(TS_SECURITY_MOCANA) )

static TsStatus_t handler( TsTransportRef_t transport, TsPath_t path, const uint8_t * buffer, size_t buffer_size );

int main() {

	ts_status_set_level(TsStatusDebug);

	// create mqtt transport
	TsTransportRef_t transport;
	TsStatus_t status = ts_transport_create( &transport );
	if( status != TsStatusOk ) {
		ts_status_debug("failed to create transport, %s\n", ts_status_string(status));
		return 0;
	}

	// set connection certs
	TsConnectionRef_t connection;
	ts_transport_get_connection( transport, &connection );
	ts_connection_set_server_cert_hostname( connection, "simpm.thingspace.verizon.com" );
	ts_connection_set_server_cert( connection, cacert_buf, 891 );
	ts_connection_set_client_cert( connection, client_cert, 941 );
	ts_connection_set_client_key( connection, client_key, 605 );

	// connect
	status = ts_transport_dial( transport, "simpm.thingspace.verizon.com:8883" );
	if( status != TsStatusOk ) {
		ts_status_debug("failed to dial, %s\n", ts_status_string(status));
		return 0;
	}

	// subscribe
	TsPath_t subscription = (TsPath_t)"ThingspaceSDK/B827EBA15910/subscription";
	status = ts_transport_listen( transport, NULL, subscription, handler);
	if( status != TsStatusOk ) {
		ts_status_debug("failed to listen, %s\n", ts_status_string(status));
		return 0;
	}

	// publish
	int count = 0;
	while(++count) {

		char payload[ 256 ];
		sprintf(payload, "message number %d", count);

		ts_status_debug("sending message, %d\n", count);
		TsPath_t topic = (TsPath_t)"ThingspaceSDK/B827EBA15910/test";
		status = ts_transport_speak( transport, topic, (const uint8_t*)payload, strlen(payload) );
		if( status != TsStatusOk ) {
			ts_status_debug("failed to speak, %s\n", ts_status_string(status));
			break;
		}

		status = ts_transport_tick( transport, 1000000);
		if( status != TsStatusOk ) {
			ts_status_debug("failed to tick, %s\n", ts_status_string(status));
			break;
		}
	}

	return 0;
}

TsStatus_t handler( TsTransportRef_t transport, TsPath_t path, const uint8_t * buffer, size_t buffer_size ) {
	ts_status_debug("handler called\n");
	return TsStatusOk;
}

#else

int main() {
	ts_status_alarm("missing one or many components, please check compile directives and build again\n");
}

#endif
