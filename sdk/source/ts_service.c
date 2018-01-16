// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_service.h"

TsStatus_t ts_service_create( TsServiceRef_t * service ) {

	ts_status_trace( "ts_service_create\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );

	TsProtocolRef_t protocol;
	TsStatus_t status = ts_protocol_create( &protocol );
	if( status != TsStatusOk ) {
		*service = NULL;
		return status;
	}

	// first, allocate and clear connection memory
	*service = (TsServiceRef_t) ( ts_platform_malloc( sizeof( TsService_t )));
	(*service)->_protocol = protocol;

	return TsStatusOk;
}

TsStatus_t ts_service_destroy( TsServiceRef_t service ) {

	ts_status_trace( "ts_service_destroy\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );

	ts_protocol_destroy( service->_protocol );
	ts_platform_free( service, sizeof( TsService_t ) );

	return TsStatusOk;
}

TsStatus_t ts_service_tick( TsServiceRef_t service, uint32_t budget ) {

	ts_status_trace( "ts_service_tick\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );

	// TODO - add diagnostic interval processing
	return ts_protocol_tick( service->_protocol, budget );
}

TsStatus_t ts_service_set_server_cert_hostname( TsServiceRef_t service, const char * hostname ) {

	ts_status_trace( "ts_service_set_server_cert_hostname\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection->_security != NULL );

	return ts_security_set_server_cert_hostname( service->_protocol->_transport->_connection->_security, hostname );
}

TsStatus_t ts_service_set_server_cert( TsServiceRef_t service, const uint8_t * cacert, size_t cacert_size ) {

	ts_status_trace( "ts_service_set_server_cert\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection->_security != NULL );

	return ts_security_set_server_cert( service->_protocol->_transport->_connection->_security, cacert, cacert_size );
}

TsStatus_t ts_service_set_client_cert( TsServiceRef_t service, const uint8_t * clcert, size_t clcert_size ) {

	ts_status_trace( "ts_service_set_client_cert\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection->_security != NULL );

	return ts_security_set_client_cert( service->_protocol->_transport->_connection->_security, clcert, clcert_size );
}

TsStatus_t ts_service_set_client_key( TsServiceRef_t service, const uint8_t * clkey, size_t clkey_size ) {

	ts_status_trace( "ts_service_set_client_key\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection != NULL );
	ts_platform_assert( service->_protocol->_transport->_connection->_security != NULL );

	return ts_security_set_client_key( service->_protocol->_transport->_connection->_security, clkey, clkey_size );
}

TsStatus_t ts_service_dial( TsServiceRef_t service, TsAddress_t address ) {

	ts_status_trace( "ts_service_dial\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );

	return ts_transport_dial( service->_protocol->_transport, address );
}

TsStatus_t ts_service_hangup( TsServiceRef_t service ) {

	ts_status_trace( "ts_service_dial\n" );
	ts_platform_assert( ts_transport != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );
	ts_platform_assert( service->_protocol->_transport != NULL );

	return ts_transport_hangup( service->_protocol->_transport );
}

TsStatus_t ts_service_enqueue( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_trace( "ts_service_enqueue\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );

	return ts_protocol_enqueue( service->_protocol, message );
}

TsStatus_t ts_service_dequeue( TsServiceRef_t service, TsServiceAction_t action, TsServiceHandler_t handler ) {

	ts_status_trace( "ts_service_enqueue\n" );
	ts_platform_assert( ts_protocol != NULL );
	ts_platform_assert( service != NULL );
	ts_platform_assert( service->_protocol != NULL );

	return TsStatusErrorNotImplemented;
}