// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platforms.h"
#include "ts_components.h"

#include "ts_transport.h"

// must compile with,...
//
// TS_TRANSPORT_MQTT
// TS_SECURITY_NONE
// opt TS_CONTROLLER_SOCKET
// opt TS_PLATFORM_UNIX
#if defined(TS_TRANSPORT_MQTT) && defined(TS_SECURITY_NONE)

static TsStatus_t handler( TsTransportRef_t, void *, TsPath_t, const uint8_t *, size_t );

int main() {

	ts_status_set_level(TsStatusDebug);

	TsTransportRef_t transport;
	TsStatus_t status = ts_transport_create( &transport );
	if( status != TsStatusOk ) {
		ts_status_debug("failed to create transport, %s\n", ts_status_string(status));
		return 0;
	}

	status = ts_transport_dial( transport, "m2m.eclipse.org:1883" );
	if( status != TsStatusOk ) {
		ts_status_debug("failed to dial, %s\n", ts_status_string(status));
		return 0;
	}

	const uint8_t id[ TS_CONTROLLER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( transport->_connection, id, TS_CONTROLLER_MAX_ID_SIZE );

	char subscription[256];
	snprintf( subscription, 256, "ThingSpace/%s/ProviderToElement", id );
	status = ts_transport_listen( transport, NULL, subscription, handler, NULL );
	if( status != TsStatusOk ) {
		ts_status_debug("failed to listen, %s\n", ts_status_string(status));
		return 0;
	}

	int count = 0;
	while(++count) {

		char payload[ 256 ];
		sprintf(payload, "{\"kind\":\"ts.event\",\"action\":\"update\",\"fields\":{\"temperature\":%d}}", count);

		ts_status_debug("sending message, '%s'\n", payload);
		TsPath_t topic = (TsPath_t)"ThingSpace/B827EBA15910/ElementToProvider";
		status = ts_transport_speak( transport, topic, (const uint8_t*)payload, strlen(payload) );
		if( status != TsStatusOk ) {
			ts_status_debug("failed to speak, %s\n", ts_status_string(status));
			break;
		}

		status = ts_transport_tick( transport, 5 * TS_TIME_SEC_TO_USEC );
		if( status != TsStatusOk ) {
			ts_status_debug("failed to tick, %s\n", ts_status_string(status));
			break;
		}
	}

	return 0;
}

static TsStatus_t handler( TsTransportRef_t transport, void * data, TsPath_t path, const uint8_t * buffer, size_t buffer_size ) {
	ts_status_debug( "handler: message, '%.*s'\n", buffer_size, buffer );
	return TsStatusOk;
}

#else

int main() {
	ts_status_alarm("missing one or many components, please check compile directives and build again\n");
}

#endif
