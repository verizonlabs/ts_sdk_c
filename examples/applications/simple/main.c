// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdio.h>

#include "ts_status.h"
#include "ts_platform.h"
#include "ts_service.h"

#include "include/cacert.h"
#include "include/client-crt.h"
#include "include/client-key.h"

#if defined(TS_TRANSPORT_MQTT) && ( defined(TS_SECURITY_MBED) || defined(TS_SECURITY_MOCANA) )

// application sensor cache
static TsMessageRef_t sensors;

// forward references
static TsStatus_t initialize( TsServiceRef_t* );
static TsStatus_t handler( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );

int main() {

	// initialize platform (see ts_platform.h)
	// TODO - waiting for ts_platform.h clean-up,...
	//TsStatus_t status = ts_platform_initialize();
	//if( status != TsStatusOk ) {
	//	ts_status_debug( "failed to initialize platform, %s\n", ts_status_string(status) );
	//	return 0;
	//}

	// initialize status reporting level (see ts_status.h)
	ts_status_set_level( TsStatusDebug );
	ts_status_debug( "simple: initializing,...\n");

	// initialize sensor cache (usually set from hardware)
	ts_message_create( &sensors );
	ts_message_set_float( sensors, "temperature", 50.2 );

	// initialize client (see ts_service.h)
	TsServiceRef_t service;
	ts_service_create( &service );

	// security initialization
	ts_status_debug( "simple: initializing certificates,...\n");
	ts_service_set_server_cert_hostname( service, "simpm.thingspace.verizon.com" );
	ts_service_set_server_cert( service, cacert_buf, sizeof( cacert_buf ) );
	ts_service_set_client_cert( service, client_cert, sizeof( client_cert ) );
	ts_service_set_client_key( service, client_key, sizeof( client_key ) );

	// connect to thingspace server
	ts_status_debug( "simple: initializing connection,...\n");
	TsStatus_t status = ts_service_dial( service, "simpm.thingspace.verizon.com:8883" );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		return status;
	}

	//  subscribe to field gets and sets
	ts_status_debug( "simple: initializing callback,...\n");
	status = ts_service_dequeue( service, TsServiceActionMaskAll, handler );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		return status;
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

	return 0;
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

#else

int main() {

	ts_status_alarm("missing one or many components, please check compile directives and build again\n");

}

#endif
