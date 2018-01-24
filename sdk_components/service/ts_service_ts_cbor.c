// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_connection.h"

static TsStatus_t ts_create( TsServiceRef_t * );
static TsStatus_t ts_destroy( TsServiceRef_t );
static TsStatus_t ts_tick( TsServiceRef_t, uint32_t );

static TsStatus_t ts_enqueue( TsServiceRef_t, TsMessageRef_t );
static TsStatus_t ts_dequeue( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

static TsStatus_t handler( TsTransportRef_t, void *, TsPath_t, const uint8_t *, size_t );

TsServiceVtable_t ts_service_ts_cbor = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,
	.enqueue = ts_enqueue,
	.dequeue = ts_dequeue,
};

// TODO - clean-up controller x connection confusion (i.e., ...MAX_ID_SIZE)
static TsStatus_t ts_create( TsServiceRef_t * service ) {

	ts_status_trace("ts_service_create: ts-cbor\n");

	// do nothing

	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsServiceRef_t service ) {

	ts_status_trace("ts_service_destroy\n");

	// do nothing

	return TsStatusOk;
}

static TsStatus_t ts_tick( TsServiceRef_t service, uint32_t budget ) {

	ts_status_trace("ts_service_tick\n");

	// TODO - check diagnostics timeout

	return TsStatusOk;
}

// TODO - allow id?, unit-name and serial-number to be configurable
// TODO - remove malloc and free for buffers
// TODO - add precondition checks
static TsStatus_t ts_enqueue( TsServiceRef_t service, TsMessageRef_t sensor ) {

	ts_status_trace("ts_service_enqueue\n");

	// get device-id from controller (via connection)
	const uint8_t id[ TS_CONTROLLER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_CONTROLLER_MAX_ID_SIZE );

	// create message content
	TsMessageRef_t message, sensors, characteristics;
	ts_message_create(&message);
	ts_message_set_string( message, "kind", "ts.event" );
	ts_message_set_string( message, "action", "update" );
	ts_message_set_message( message, "fields", sensor );

	// encode copy to send buffer
	// i.e., encode and send unsolicited message
	// get mcu from controller (via connection)
	uint32_t mcu;
	ts_connection_get_spec_mcu( service->_transport->_connection, &mcu);

	// allocate data buffer
	uint8_t * buffer = (uint8_t*)ts_platform_malloc( mcu );
	size_t buffer_size = (size_t)mcu;
	ts_message_encode(message, TsEncoderTsCbor, buffer, &buffer_size);

	// TODO - remove test code
	TsMessageRef_t test;
	ts_message_create( &test );
	ts_message_decode( test, TsEncoderTsCbor, buffer, buffer_size );
	ts_message_encode( test, TsEncoderDebug, NULL, 0 );

	// send data
	size_t topic_size = 256;
	char topic[ 256 ];
	snprintf( topic, topic_size, "ThingSpace/%s/ElementToProvider", id );
	ts_status_debug( "ts_service_enqueue: sending (%.*s) on (%s)\n", buffer_size, buffer, topic );
	ts_transport_speak( service->_transport, (TsPath_t)topic, buffer, buffer_size );

	// clean-up and return
	ts_platform_free( buffer, buffer_size );
	ts_message_destroy( message );

	return TsStatusOk;
}

// TODO - add precondition checks
static TsStatus_t ts_dequeue( TsServiceRef_t service, TsServiceAction_t action, TsServiceHandler_t service_handler ) {

	ts_status_trace("ts_service_dequeue\n");

	// get device-id from controller (via connection)
	const uint8_t id[ TS_CONTROLLER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_CONTROLLER_MAX_ID_SIZE );

	// listen to topic
	// TODO - can be called multiple times, but re-check later for another improved impl?
	snprintf( service->_subscription, TS_SERVICE_MAX_PATH_SIZE, "ThingSpace/%s/ProviderToElement", id );
	return ts_transport_listen( service->_transport, NULL, service->_subscription, handler, service );
}

static TsStatus_t handler_get( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_debug("ts_service_handler_get\n");

	// pull sensor handler
	TsServiceHandler_t service_handler = service->_handlers[ TsServiceActionGetIndex ];

	// delegate to service handler
	TsStatus_t status = TsStatusErrorNotFound;
	if( service_handler != NULL ) {

		// prep
		TsMessageRef_t fields, content;
		ts_message_get_message(message, "fields", &fields );

		// delegate
		status = service_handler( service, TsServiceActionGet, fields );

	} else {

		ts_status_debug( "ts_service_handler_get: missing handler or message missing sensor name\n" );
	}

	// return status
	return status;
}

static TsStatus_t handler_set( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_debug( "ts_service_handler_set\n" );

	// pull sensor handler
	TsServiceHandler_t service_handler = service->_handlers[ TsServiceActionSetIndex ];

	TsStatus_t status = TsStatusErrorNotFound;
	if( service_handler != NULL ) {

		// pull actuator name, value and handler
		TsMessageRef_t fields;
		ts_message_get( message, "fields", &fields );

		// delegate
		status = service_handler( service, TsServiceActionSet, fields );

	} else {

		ts_status_debug( "ts_service_handler_set: missing handler or message missing sensor name or value\n" );
	}

	// return status
	return status;
}

static TsStatus_t handler_activate( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_debug("ts_service_handler_activate\n");

	// TODO - delegate

	return TsStatusOk;
}

/**
 * Transport received message handler. Parses and delivers the message to the registered ServiceHandler
 * @param transport
 * @param path
 * @param buffer
 * @param buffer_size
 * @return
 */
static TsStatus_t handler( TsTransportRef_t transport, void * data, TsPath_t path, const uint8_t * buffer, size_t buffer_size ) {

	ts_status_debug("ts_service_handler\n");

	ts_platform_assert( transport != NULL );
	ts_platform_assert( data != NULL );

	// cast data to service pointer (as set in the dequeue function above)
	TsServiceRef_t service = (TsServiceRef_t)data;

	// TODO - forward solicited message to handler (or drop if no handler written)
	// TODO - return response via handler message returned
	TsMessageRef_t message;
	ts_message_create( &message );
	TsStatus_t status = ts_message_decode( message, TsEncoderTsCbor, (uint8_t*)buffer, buffer_size );
	if( status != TsStatusOk ) {

		ts_status_alarm( "ts_service_handler: message muddled, '%.*s'\n", buffer_size, buffer );

	} else {

		char * kind;
		status = ts_message_get_string( message, "kind", &kind );
		switch( status ) {
		case TsStatusOk:

			if( strcmp(kind, "ts.event") == 0 ) {

				char * action;
				status = ts_message_get_string( message, "action", &action );
				if( status != TsStatusOk ) {

					// error
					ts_status_alarm( "ts_service_handler: unexpected message parsing error, %s\n", ts_status_string( status ) );
					status = TsStatusErrorInternalServerError;

				} else {

					if( strcmp( action, "get" ) == 0 ) {

						// get sensor, note that the message will be modified 'in-place'
						// and must be returned with the correct status
						status = handler_get( service, message );

					} else if( strcmp( action, "set" ) == 0 ) {

						// set sensor
						status = handler_set( service, message );

					} else {

						// error
						ts_status_alarm( "ts_service_handler: unknown action, '%s'\n", action );
						status = TsStatusErrorNotImplemented;
					}
				}

			} else if( strcmp( kind, "ts.event.diagnostic" ) == 0 ) {

				// get diagnostics, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				// TODO - TS-CBOR diagnostics query not yet implemented
				ts_status_debug( "ts_service_handler: diagnostics requested, and ignored,...\n" );
				status = TsStatusOk;

			} else if( strcmp( kind, "ts.device" ) == 0 ) {

				// provisioning, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				// TODO - TS-CBOR provisioning not yet implemented
				ts_status_debug( "ts_service_handler: diagnostics requested, and ignored,...\n" );
				status = TsStatusOk;

			} else {

				// error
				ts_status_alarm( "ts_service_handler: unknown kind, '%s'\n", kind );
				status = TsStatusErrorNotImplemented;
			}
			break;

		default:

			// error
			ts_status_alarm( "ts_service_handler: unexpected message parsing error, %s\n", ts_status_string( status ) );
			status = TsStatusErrorInternalServerError;
			break;
		}
	}

	// attempt to respond

	// modify given message to include status data
	ts_message_set_int( message, "status", ts_status_code( status ) );
	if( status != TsStatusOk ) {
		ts_message_set_int( message, "error", ts_status_error( status ) );
	}

	// get device-id from controller (via connection)
	const uint8_t id[ TS_CONTROLLER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_CONTROLLER_MAX_ID_SIZE );

	// encode copy to send buffer
	// i.e., encode and send unsolicited message
	// get mcu from controller (via connection)
	uint32_t mcu;
	ts_connection_get_spec_mcu( transport->_connection, &mcu);

	// allocate data buffer
	uint8_t * response_buffer = (uint8_t*)ts_platform_malloc( mcu );
	size_t response_buffer_size = (size_t)mcu;
	ts_message_encode(message, TsEncoderTsCbor, response_buffer, &response_buffer_size);

	// send data
	size_t topic_size = 256;
	char topic[ 256 ];
	snprintf( topic, topic_size, "ThingSpace/%s/ElementToProvider", id );
	ts_transport_speak( transport, (TsPath_t)topic, response_buffer, response_buffer_size );

	// clean-up and return
	ts_platform_free( response_buffer, response_buffer_size );
	ts_message_destroy( message );
	return TsStatusOk;
}