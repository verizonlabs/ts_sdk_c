// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdio.h>

#include "ts_status.h"
#include "ts_platform.h"
#include "ts_service.h"

#include "include/cacert.h"
#include "include/client-crt.h"
#include "include/client-key.h"

#if defined(TS_TRANSPORT_MQTT)

// application sensor cache
static TsMessageRef_t sensors;

// forward references
//static TsStatus_t initialize( TsServiceRef_t* );
static TsStatus_t handler( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port );

int main( int argc, char *argv[] ) {

	// initialize platform (see ts_platform.h)
	ts_platform_initialize();

	// initialize status reporting level (see ts_status.h)
	ts_status_set_level( TsStatusLevelDebug );
	ts_status_debug( "simple: initializing,...\n");

	// initialize hostname
	char * hostname_and_port = "simpm.thingspace.verizon.com:8883";
	char * host = "simpm.thingspace.verizon.com";
	char * port = "8883";
#if defined(TS_PLATFORM_UNIX)
	if( usage( argc, argv, &hostname_and_port, &host, &port ) != TsStatusOk ) {
		ts_status_debug( "simple: failed to parse host and port\n" );
		ts_platform_assert(0);
	}
#endif
	ts_status_debug( "simple: hostname(%s), host(%s), port(%s)\n", hostname_and_port, host, port );

	// initialize sensor cache (usually set from hardware)
	ts_message_create( &sensors );
	ts_message_set_float( sensors, "temperature", 50.2 );

	// initialize client (see ts_service.h)
	TsServiceRef_t service;
	ts_service_create( &service );

	// security initialization
	ts_status_debug( "simple: initializing certificates,...\n");
	ts_service_set_server_cert_hostname( service, (const char *)host );
	ts_service_set_server_cert( service, cacert_buf, sizeof( cacert_buf ) );
	ts_service_set_client_cert( service, client_cert, sizeof( client_cert ) );
	ts_service_set_client_key( service, client_key, sizeof( client_key ) );

	// connect to thingspace server
	ts_status_debug( "simple: initializing connection,...\n");
	TsStatus_t status = ts_service_dial( service, hostname_and_port );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}

	//  subscribe to field gets and sets
	ts_status_debug( "simple: initializing callback,...\n");
	status = ts_service_dequeue( service, TsServiceActionMaskAll, handler );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}

	// enter run loop,...
	ts_status_debug( "simple: entering run-loop,...\n");
	uint64_t timestamp = ts_platform_time();
	uint32_t interval = 5 * TS_TIME_SEC_TO_USEC;
	bool running = true;
	do {

		// perform update at particular delta
		if( ts_platform_time() - timestamp > interval ) {

			timestamp = ts_platform_time();
			status = ts_service_enqueue( service, sensors );
			if( status != TsStatusOk ) {
				ts_status_debug( "simple: ignoring failure to enqueue sensor data, %s\n", ts_status_string(status) );
				// do nothing
			}
		}

		// provide client w/some processing power
		// note - this will run continuously until the interval is complete
		//        other options include limiting the interval, and sleeping after
		status = ts_service_tick( service, interval );
		if( status != TsStatusOk ) {
			ts_status_debug( "simple: failed to perform tick, %s, shutting down,...\n", ts_status_string(status) );
			running = false;
		}

	} while( running );
	ts_status_debug( "simple: exited run-loop, cleaning up and exiting...\n");

	// disconnect from thingspace server
	ts_service_hangup( service );

	// clean-up and exit
	ts_service_destroy( service );
	ts_message_destroy( sensors );

	ts_platform_assert(0);
}

static TsStatus_t handler( TsServiceRef_t service, TsServiceAction_t action, TsMessageRef_t message ) {

	switch( action ) {
	case TsServiceActionSet: {

		float temperature;
		if( ts_message_get_float( message, "temperature", &temperature ) == TsStatusOk ) {
			ts_status_debug( "handler(set): temperature, %f\n", temperature );
			ts_message_set_float( sensors, "temperature", temperature );
		}
		break;
	}

	case TsServiceActionGet: {

		TsMessageRef_t object;
		if( ts_message_has( message, "temperature", &object ) == TsStatusOk ) {
			float temperature;
			if( ts_message_get_float( sensors, "temperature", &temperature ) == TsStatusOk ) {
				ts_status_debug( "handler(get): temperature, %f\n", temperature );
				ts_message_set_float( message, "temperature", temperature );
			}
		}
		break;
	}

	case TsServiceActionActivate: {

		float temperature;
		if( ts_message_get_float( sensors, "temperature", &temperature ) == TsStatusOk ) {
			ts_status_debug( "handler(activate): temperature, %f\n", temperature );
			ts_message_set_float( message, "temperature", temperature );
		}
		break;
	}

	case TsServiceActionDeactivate:
	case TsServiceActionSuspend:
	case TsServiceActionResume:

		// not supported by TS-JSON
		// fallthrough

	default:

		// do nothing
		break;
	}

	return TsStatusOk;
}

static char * xhostname_and_port = "simpm.thingspace.verizon.com:8883";
static char xhost[256], xport[6];
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port ) {

	switch( argc ) {
#if defined(TS_SERVICE_TS_CBOR)
	default:
		ts_status_alarm("usage: example_simple [hostname_and_port]\n");
		return TsStatusError;
#else
	case 1:
		// allow default production if ts-json being tested (not ts-cbor)
		*hostname_and_port = xhostname_and_port;
		break;
	default:
		// fallthrough
#endif
	case 2:
		*hostname_and_port = argv[ 1 ];
		break;
	}

	if( ts_address_parse( *hostname_and_port, xhost, xport ) != TsStatusOk ) {
		ts_status_alarm("failed to parse given address, %s\n", hostname_and_port);
		return TsStatusError;
	}

	*host = xhost;
	*port = xport;

	return TsStatusOk;
}
#else

int main() {

	ts_status_alarm("missing one or many components, please check compile directives and build again\n");

}

#endif
