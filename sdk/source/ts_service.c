// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_suspend.h"
#include "ts_version.h"

TsStatus_t ts_service_create( TsServiceRef_t * service ) {

	ts_status_trace( "ts_service_create\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_service != NULL );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );

	// first, allocate and initialize transport memory
	TsTransportRef_t transport;
	TsStatus_t status = ts_transport_create( &transport );
	if( status != TsStatusOk ) {
		*service = NULL;
		return status;
	}

	// next, allocate service memory
	*service = (TsServiceRef_t) ( ts_platform_malloc( sizeof( TsService_t )));
	memset( *service, 0x00, sizeof( TsService_t ) );
	(*service)->_transport = transport;
	(*service)->_firewall = NULL;

	// last, complete service initialization via protocol-specific create
	status = ts_service->create( service );
	if( status != TsStatusOk ) {
		ts_transport_destroy( transport );
		ts_platform_free( *service, sizeof( TsService_t ) );
		*service = NULL;
		return status;
	}

	return TsStatusOk;
}

TsStatus_t ts_service_destroy( TsServiceRef_t service ) {

	ts_status_trace( "ts_service_destroy\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_service != NULL );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	ts_service->destroy( service );
	ts_transport_destroy( service->_transport );
	ts_platform_free( service, sizeof( TsService_t ) );

	return TsStatusOk;
}

#ifdef TEST_SUSPEND
	static int ticks = 0;
	bool suspend = false;
#endif

// Get the LogConfigRef_t object associated with this service, if it exists.
// Return null if there is none.
TsLogConfigRef_t ts_service_get_logconfig( TsServiceRef_t service ) {
	return service->_logconfig;
}

TsStatus_t ts_service_tick( TsServiceRef_t service, uint32_t budget ) {

	ts_status_trace( "ts_service_tick\n" );
	ts_platform_assert( ts_service != NULL );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	// perform service tick
	uint64_t timestamp = ts_platform_time();
	TsStatus_t status = ts_service->tick( service, budget );
	if( status != TsStatusOk ) {
		ts_status_alarm( "ts_service_tick: failed protocol phase, %s, ignoring,...\n", ts_status_string(status) );
		// do nothing
	}

	// determine remaining timer budget
	uint32_t interval = (uint32_t)(ts_platform_time() - timestamp);

	if( interval >= budget ) {
		ts_status_debug( "ts_service_tick: after calling service tick, budget exceeded, ignoring,...\n" );
		interval = 0;
	}

	// logging tick
	if (service->_logconfig != NULL) {
		status = ts_logconfig_tick(service->_logconfig, budget - interval);
		interval = (uint32_t)(ts_platform_time() - timestamp);
		if (interval >= budget) {
			ts_status_debug(
						"ts_service_tick: after calling logconfig tick, budget exceeded, ignoring,...\n");
			interval = 0;
		}
	}

	// firewall tick
	if (service->_firewall != NULL) {
		status = ts_firewall_tick(service->_firewall, budget - interval);
		interval = (uint32_t)(ts_platform_time() - timestamp);
		if (interval >= budget) {
			ts_status_debug(
					"ts_service_tick: after calling firewall tick, budget exceeded, ignoring,...\n");
			interval = 0;
		}
	}

	ts_status_trace("After calling firewall tick, remaining timer budget is %d, interval is %d\n", budget, interval);

#ifdef TEST_SUSPEND
	ticks++;
	if (ticks >= 2) {
		suspend = !suspend;
		ts_suspend_test(suspend, suspend);
		ticks = 0;
	}
#endif /* TEST_SUSPEND */

	// perform transport tick within remaining budget
	// TODO - return may require user action, e.g., TsStatusErrorConnectionReset - or add processing here.
	return ts_transport_tick(service->_transport, budget - interval);
}

TsStatus_t ts_service_set_server_cert_hostname( TsServiceRef_t service, const char * hostname ) {

	ts_status_trace( "ts_service_set_server_cert_hostname\n" );
	ts_platform_assert( ts_service != NULL );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );
	ts_platform_assert( service->_transport->_connection != NULL );
	ts_platform_assert( service->_transport->_connection->_security != NULL );

	return ts_security_set_server_cert_hostname( service->_transport->_connection->_security, hostname );
}

TsStatus_t ts_service_set_server_cert( TsServiceRef_t service, const uint8_t * cacert, size_t cacert_size ) {

	ts_status_trace( "ts_service_set_server_cert\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );
	ts_platform_assert( service->_transport->_connection != NULL );
	ts_platform_assert( service->_transport->_connection->_security != NULL );

	return ts_security_set_server_cert( service->_transport->_connection->_security, cacert, cacert_size );
}

TsStatus_t ts_service_set_client_cert( TsServiceRef_t service, const uint8_t * clcert, size_t clcert_size ) {

	ts_status_trace( "ts_service_set_client_cert\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );
	ts_platform_assert( service->_transport->_connection != NULL );
	ts_platform_assert( service->_transport->_connection->_security != NULL );

	return ts_security_set_client_cert( service->_transport->_connection->_security, clcert, clcert_size );
}

TsStatus_t ts_service_set_client_key( TsServiceRef_t service, const uint8_t * clkey, size_t clkey_size ) {

	ts_status_trace( "ts_service_set_client_key\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );
	ts_platform_assert( service->_transport->_connection != NULL );
	ts_platform_assert( service->_transport->_connection->_security != NULL );

	return ts_security_set_client_key( service->_transport->_connection->_security, clkey, clkey_size );
}

TsStatus_t ts_service_dial( TsServiceRef_t service, TsAddress_t address ) {

	ts_status_trace( "ts_service_dial\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	TsStatus_t status = ts_transport_dial( service->_transport, address );

	// Send an update message representing version information
	if (status == TsStatusOk) {
		TsMessageRef_t versionMessage;
		if (ts_version_make_update( &versionMessage ) == TsStatusOk) {
			ts_message_dump(versionMessage);
			ts_service_enqueue_typed(service, "ts.event.version", versionMessage);
			ts_message_destroy(versionMessage);
		}
	}
	return status;
}

TsStatus_t ts_service_hangup( TsServiceRef_t service ) {

	ts_status_trace( "ts_service_dial\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	return ts_transport_hangup( service->_transport );
}

TsStatus_t ts_service_enqueue( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_trace( "ts_service_enqueue\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	return ts_service->enqueue( service, message );
}

TsStatus_t ts_service_enqueue_typed(TsServiceRef_t service, char* type, TsMessageRef_t message ) {
	ts_status_trace( "ts_service_enqueue_typed\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );
	ts_platform_assert(type != NULL);
	return ts_service->enqueuetyped(service, type, message);
}

TsStatus_t ts_service_dequeue( TsServiceRef_t service, TsServiceAction_t action, TsServiceHandler_t handler ) {

	ts_status_trace( "ts_service_dequeue\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_transport != NULL );

	// initialize or overwrite service handler by index
	for( int index = 0; index < TS_SERVICE_MAX_HANDLERS; index++ ) {
		if( 0x0001 & ( action >> index ) ) {
			service->_handlers[index] = handler;
		}
	}

	// allow protocol specific implementation
	return ts_service->dequeue( service, action, handler );
}
