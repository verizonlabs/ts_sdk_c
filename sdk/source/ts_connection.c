// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#include "ts_platform.h"
#include "ts_connection.h"
#include "ts_security.h"

TsStatus_t ts_connection_create( TsConnectionRef_t * connection ) {

	ts_status_trace( "ts_connection_create\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );

	// first, allocate and clear connection memory
	* connection = (TsConnectionRef_t) ( ts_platform_malloc( sizeof( TsConnection_t )));
	memset( * connection, 0x00, sizeof( TsConnection_t ));

	// then, pass pre-allocated connection to pipeline
	TsStatus_t status = ts_security_create( & (( * connection )->_security ));
	if( status != TsStatusOk ) {
		ts_status_debug( "ts_connection_create: failed in pipeline, cleaning up,...\n" );
		ts_platform_free( * connection, sizeof( TsConnection_t ));
		return status;
	}

	return TsStatusOk;
}

TsStatus_t ts_connection_destroy( TsConnectionRef_t connection ) {

	ts_status_trace( "ts_connection_destroy\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );

	// first, free from pipeline
	TsStatus_t status = ts_security_destroy( connection->_security );
	if( status != TsStatusOk ) {
		ts_status_debug( "ts_connection_destroy: ignoring failure, '%s'\n", ts_status_string( status ));
	}

	// then, free from this
	ts_platform_free( connection, sizeof( TsConnection_t ));

	return TsStatusOk;
}

TsStatus_t ts_connection_tick( TsConnectionRef_t connection, uint32_t budget ) {

	ts_status_trace( "ts_connection_tick\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( budget > 0 );

	uint64_t timestamp = ts_platform_time();
	TsStatus_t status = ts_security_tick( connection->_security, budget );
	if( ts_platform_time() - timestamp > (uint64_t) budget ) {
		ts_status_debug( "ts_connection_tick: ignoring failure, exceeding time budget\n" );
	}

	return status;
}

TsStatus_t ts_connection_get_security( TsConnectionRef_t connection, TsSecurityRef_t * security ) {

	ts_status_trace( "ts_connection_get_spec_mcu\n" );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( security!= NULL );

	*security = connection->_security;

	return TsStatusOk;
}

TsStatus_t ts_connection_get_profile( TsConnectionRef_t connection, TsProfileRef_t * profile ) {

	ts_status_trace( "ts_connection_get_spec_mcu\n" );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( profile != NULL );

	*profile = connection->_profile;

	return TsStatusOk;
}

TsStatus_t ts_connection_get_spec_mcu( TsConnectionRef_t connection, uint32_t* mcu ) {

	ts_status_trace( "ts_connection_get_spec_mcu\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( connection->_security != NULL );
	ts_platform_assert( connection->_security->_controller != NULL );
	ts_platform_assert( connection->_security->_controller->_driver != NULL );
	ts_platform_assert( mcu != NULL );

	*mcu = connection->_security->_controller->_driver->_spec_mcu;

	return TsStatusOk;
}

TsStatus_t ts_connection_get_spec_id( TsConnectionRef_t connection, const uint8_t * id, size_t id_size ) {

	ts_status_trace( "ts_connection_get_spec_id\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( connection->_security != NULL );
	ts_platform_assert( connection->_security->_controller != NULL );
	ts_platform_assert( connection->_security->_controller->_driver != NULL );
	ts_platform_assert( id != NULL );
	ts_platform_assert( id_size <= TS_DRIVER_MAX_ID_SIZE );

	snprintf( (char *)id, id_size, "%s", (const char *)(connection->_security->_controller->_driver->_spec_id) );

	return TsStatusOk;
}

TsStatus_t ts_connection_get_spec_budget( TsConnectionRef_t connection, uint32_t* budget ) {

	ts_status_trace( "ts_connection_get_spec_budget\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( connection->_security != NULL );
	ts_platform_assert( connection->_security->_controller != NULL );
	ts_platform_assert( connection->_security->_controller->_driver != NULL );
	ts_platform_assert( budget != NULL );

	*budget = connection->_security->_controller->_driver->_spec_budget;

	return TsStatusOk;
}

TsStatus_t ts_connection_set_server_cert_hostname( TsConnectionRef_t connection, const char * hostname ) {

	ts_status_trace( "ts_connection_set_server_cert\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );

	return ts_security_set_server_cert_hostname( connection->_security, hostname );
}

TsStatus_t ts_connection_set_server_cert( TsConnectionRef_t connection, const uint8_t * cacert, size_t cacert_size ) {

	ts_status_trace( "ts_connection_set_server_cert\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( cacert != NULL );
	ts_platform_assert( cacert_size > 0 );

	return ts_security_set_server_cert( connection->_security, cacert, cacert_size );
}

TsStatus_t ts_connection_set_client_cert( TsConnectionRef_t connection, const uint8_t * clcert, size_t clcert_size ) {

	ts_status_trace( "ts_connection_set_client_cert\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( clcert != NULL );
	ts_platform_assert( clcert_size > 0 );

	return ts_security_set_client_cert( connection->_security, clcert, clcert_size );
}

TsStatus_t ts_connection_set_client_key( TsConnectionRef_t connection, const uint8_t * clkey, size_t clkey_size ) {

	ts_status_trace( "ts_connection_set_client_key\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( clkey != NULL );
	ts_platform_assert( clkey_size > 0 );

	return ts_security_set_client_key( connection->_security, clkey, clkey_size );
}

TsStatus_t ts_connection_connect( TsConnectionRef_t connection, TsAddress_t address ) {

	ts_status_trace( "ts_connection_connect\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( address != NULL );

	return ts_security_connect( connection->_security, address );
}

TsStatus_t ts_connection_disconnect( TsConnectionRef_t connection ) {

	ts_status_trace( "ts_connection_disconnect\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );

	return ts_security_disconnect( connection->_security );
}

TsStatus_t ts_connection_read( TsConnectionRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_connection_read\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( buffer != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( * buffer_size > 0 );

	// TODO - check budget and alert if exceeded (like tick)
	TsStatus_t status = ts_security_read( connection->_security, buffer, buffer_size, budget );
	switch( status ) {
	case TsStatusOk:

		// do nothing
		break;

	case TsStatusOkReadPending:

		if( * buffer_size > 0 ) {
			ts_status_alarm( "ts_connection_read: read-pending status contains received bytes, %d, data dropped!\n",
			                 * buffer_size );
		}
		break;

	default:

		ts_status_info( "ts_connection_read: error received, %s\n", ts_status_string(status) );
		break;
	}
	return status;
}

TsStatus_t ts_connection_write( TsConnectionRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_connection_write\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( connection != NULL );
	ts_platform_assert( buffer != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( * buffer_size > 0 );

	// TODO - check budget and alert if exceeded (like tick)
	TsStatus_t status = ts_security_write( connection->_security, buffer, buffer_size, budget );
	switch( status ) {
	case TsStatusOk:

		// do nothing
		break;

	default:

		ts_status_info( "ts_connection_write: error received, %d\n", status );
		break;

	case TsStatusOkWritePending:

		// do nothing
		break;
	}
	return status;
}
