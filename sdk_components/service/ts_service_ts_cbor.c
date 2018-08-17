// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_firewall.h"
#include "ts_log.h"
#include "ts_version.h"
#include "ts_cert.h"
#include "ts_util.h"
#include "ts_suspend.h"
#include <stdio.h>
#include <string.h>

static TsStatus_t ts_create( TsServiceRef_t * );
static TsStatus_t ts_destroy( TsServiceRef_t );
static TsStatus_t ts_tick( TsServiceRef_t, uint32_t );

static TsStatus_t ts_enqueue( TsServiceRef_t, TsMessageRef_t );
static TsStatus_t ts_enqueue_typed( TsServiceRef_t service, char* type, TsMessageRef_t data);
static TsStatus_t ts_dequeue( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

static TsStatus_t handler( TsTransportRef_t, void *, TsPath_t, const uint8_t *, size_t );

TsServiceVtable_t ts_service_ts_cbor = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,
	.enqueue = ts_enqueue,
	.enqueuetyped = ts_enqueue_typed,
	.dequeue = ts_dequeue,
};

// Callback used by ts_firewall and ts_log to issue alert messages over the connection.

static TsServiceRef_t _messageSendingService;
static TsStatus_t _send_message_callback( TsMessageRef_t message, char *kind ) {
	if (_messageSendingService != NULL && message != NULL) {
		return ts_enqueue_typed( _messageSendingService, kind, message );
	} else {
		return TsStatusErrorPreconditionFailed;
	}
}

static TsStatus_t ts_create( TsServiceRef_t * service ) {

	ts_status_trace("ts_service_create: ts-cbor\n");
	ts_platform_assert( service != NULL );
	ts_platform_assert( *service != NULL );

	_messageSendingService = *service;

	// create logconfig
	TsStatus_t status = ts_logconfig_create(&((*service)->_logconfig) , _send_message_callback);
	if ( status != TsStatusOk ) {
		ts_status_alarm( "ts_service_create: failed to create log config, '%s'\n", ts_status_string(status));
	}

	// create scepconfig
	status = ts_scepconfig_create(&((*service)->_scepconfig) , _send_message_callback);
	if ( status != TsStatusOk ) {
		ts_status_alarm( "ts_service_create: failed to create scep config, '%s'\n", ts_status_string(status));
	}

	// create firewall if supported
	if( ts_firewall != NULL ) {
		TsStatus_t status = ts_firewall_create( &((*service)->_firewall) , _send_message_callback);
		if( status != TsStatusOk ) {
			ts_status_alarm( "ts_service_create: failed to create installed firewall, '%s'\n", ts_status_string(status));
		}
		ts_firewall_set_log((*service)->_logconfig);
	}

	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsServiceRef_t service ) {

	ts_status_trace("ts_service_destroy\n");
	ts_platform_assert( service != NULL );

	// destroy firewall, if already created
	if( service->_firewall != NULL ) {
		ts_firewall_destroy( service->_firewall );
	}

	// destroy log config, if already created
	if ( service->_logconfig != NULL ) {
		ts_logconfig_destroy( service->_logconfig );
	}
	return TsStatusOk;
}

#ifdef TEST_LOGGING
static int tick_counter = 0;
#endif

static TsStatus_t ts_tick( TsServiceRef_t service, uint32_t budget ) {

	ts_status_trace("ts_service_tick\n");

	// TODO - check diagnostics timeout

#ifdef TEST_LOGGING
	tick_counter++;
	if (tick_counter >= 5) {
		ts_log(service->_logconfig, TsLogLevelInfo, TsCategoryDiagnostic, "This is a test message");
		tick_counter = 0;
	}
#endif

	return TsStatusOk;
}

#define MIN_MTU 512

static TsStatus_t ts_encode_and_send_message(TsServiceRef_t service, const uint8_t id[ TS_DRIVER_MAX_ID_SIZE ], TsMessageRef_t message) {
		// encode copy to send buffer
		// i.e., encode and send unsolicited message
		// get mtu from controller (via connection)
		size_t mtu;
		ts_connection_get_spec_mtu( service->_transport->_connection, (uint32_t *) &mtu);

		// allocate data buffer and encode data
		uint8_t * buffer = ts_get_buffer( &mtu, MIN_MTU );
		if (buffer == NULL) {
			ts_status_alarm("ts_encode_and_send_message: could not allocate buffer\n");
			return TsStatusErrorOutOfMemory;
		}

		size_t buffer_size = mtu - 4;
		ts_message_encode(message, TsEncoderTsCbor, buffer + 4, &buffer_size);

		// encode envelope
		buffer[ 0 ] = TsServiceEnvelopeVersionOne;
		buffer[ 1 ] = TsServiceEnvelopeServiceIdTsCbor;
		buffer[ 2 ] = (uint8_t)(buffer_size >> 8);
		buffer[ 3 ] = (uint8_t)(buffer_size & 0xff);
		buffer_size = buffer_size + 4;

		// send data
		size_t topic_size = 256;
		char topic[ 256 ];
		snprintf( topic, topic_size, "ThingSpace/%s/ElementToProvider", id );

		// TODO - check return codes - may have disconnected.
		ts_transport_speak( service->_transport, (TsPath_t)topic, buffer, buffer_size );

		// clean-up and return

		ts_platform_free( buffer, mtu );
		return TsStatusOk;
}

static TsStatus_t ts_enqueue_typed( TsServiceRef_t service, char* type, TsMessageRef_t data) {

	ts_status_trace("ts_service_enqueue_typed\n");

	// get device-id from controller (via connection)
	const uint8_t id[ TS_DRIVER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_DRIVER_MAX_ID_SIZE );

	if (strcmp(type, "ts.event.firewall.alert") == 0
			|| strcmp(type, "ts.event.log") == 0
			|| strcmp(type, "ts.event.cert") == 0
			|| strcmp(type, "ts.event.version") == 0) {
		// The message is ready-made in this case. The alert callback caller owns it.
		// TODO: Should some of the logic to generate UUID, kind, etc. be up here? Stats case could use the UUID generator
		return ts_encode_and_send_message(service, id, data);
	}

	// create message content
	TsMessageRef_t message;
	ts_message_create(&message);
	//"transactionid": 620158454135351354682660134707491,
	ts_message_set_string( message, "action", "update" );

	if (strcmp(type, "ts.event.firewall.statistics") == 0) {
		ts_message_set_string( message, "kind", "ts.event.firewall.statistics" );
		ts_message_set_message( message, "fields", data );
	}

	ts_encode_and_send_message(service, id, message);
	ts_message_destroy( message );

	return TsStatusOk;
}

// TODO - allow id?, unit-name and serial-number to be configurable
// TODO - remove malloc and free for buffers
// TODO - add precondition checks
static TsStatus_t ts_enqueue( TsServiceRef_t service, TsMessageRef_t sensor ) {

	ts_status_trace("ts_service_enqueue\n");

	// get device-id from controller (via connection)
	const uint8_t id[ TS_DRIVER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_DRIVER_MAX_ID_SIZE );

	// create message content
	TsMessageRef_t message;
	ts_message_create(&message);
	ts_message_set_string( message, "kind", "ts.event" );
	ts_message_set_string( message, "action", "update" );
	ts_message_set_message( message, "fields", sensor );

	// encode copy to send buffer
	// i.e., encode and send unsolicited message
	// get mtu from controller (via connection)
	size_t mtu;
	ts_connection_get_spec_mtu( service->_transport->_connection, (uint32_t *) &mtu);

	// allocate data buffer and encode data
	uint8_t * buffer = ts_get_buffer( &mtu, MIN_MTU );
	if (buffer == NULL) {
		ts_status_alarm("ts_enqueue: could not allocate buffer\n");
		return TsStatusErrorOutOfMemory;
	}

	size_t buffer_size = (size_t)(mtu - 4);
	ts_message_encode(message, TsEncoderTsCbor, buffer + 4, &buffer_size);

	// encode envelope
	buffer[ 0 ] = TsServiceEnvelopeVersionOne;
	buffer[ 1 ] = TsServiceEnvelopeServiceIdTsCbor;
	buffer[ 2 ] = (uint8_t)(buffer_size >> 8);
	buffer[ 3 ] = (uint8_t)(buffer_size & 0xff);
	buffer_size = buffer_size + 4;
	
	// send data
	size_t topic_size = 256;
	char topic[ 256 ];
	snprintf( topic, topic_size, "ThingSpace/%s/ElementToProvider", id );

	// TODO - check return codes - may have disconnected.
	ts_transport_speak( service->_transport, (TsPath_t)topic, buffer, buffer_size );

	// clean-up and return
	ts_platform_free( buffer, mtu );
	ts_message_destroy( message );

	return TsStatusOk;
}

// TODO - add precondition checks
static TsStatus_t ts_dequeue( TsServiceRef_t service, TsServiceAction_t action, TsServiceHandler_t service_handler ) {

	ts_status_trace("ts_service_dequeue\n");

	// get device-id from controller (via connection)
	const uint8_t id[ TS_DRIVER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_DRIVER_MAX_ID_SIZE );

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
		TsMessageRef_t fields;
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

/**
 * Transport received message handler. Parses and delivers the message to the registered ServiceHandler
 * @param transport
 * @param path
 * @param buffer
 * @param buffer_size
 * @return
 */
static TsStatus_t handler( TsTransportRef_t transport, void * state, TsPath_t path, const uint8_t * buffer, size_t buffer_size ) {

	ts_status_debug("ts_service_handler\n");

	ts_platform_assert( transport != NULL );
	ts_platform_assert( state != NULL );

	// cast data to service pointer (as set in the dequeue function above)
	TsServiceRef_t service = (TsServiceRef_t)state;

	// decode envelope
	uint8_t * data = (uint8_t*)buffer;
	size_t data_size = buffer_size;
	if( data_size < 4 ) {
		ts_status_alarm( "ts_service_handler: envelope size too small, %d\n", data_size );
		return TsStatusErrorBadRequest;
	}
	if( data[ 0 ] != TsServiceEnvelopeVersionOne ) {
		ts_status_alarm( "ts_service_handler: envelope version not supported, %02x\n", data[0]);
		return TsStatusErrorNotImplemented;
	}
	if( data[ 1 ] != TsServiceEnvelopeServiceIdTsCbor ) {
		ts_status_alarm( "ts_service_handler: envelope service-id not supported, %02x\n", data[1]);
		return TsStatusErrorNotImplemented;
	}
	size_t msb = data[ 2 ];
	size_t lsb = data[ 3 ];
	if( data_size - 4 != ( (msb << 8) | lsb) ) {
		ts_status_debug( "ts_service_handler: envelope size mismatch, %d vs %d, ignoring,...\n", data_size - 4, (msb << 8) | lsb );
	}
	data = data + 4;
	data_size = data_size - 4;

	// TODO - forward solicited message to handler (or drop if no handler written)
	// TODO - return response via handler message returned
	// decode message
	TsMessageRef_t message;
	ts_message_create( &message );
	TsStatus_t status = ts_message_decode( message, TsEncoderTsCbor, (uint8_t*)data, data_size );
	if( status != TsStatusOk ) {

		ts_status_alarm( "ts_service_handler: message muddled, '%.*s'\n", data_size, data );

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

				// diagnostics, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				// TODO - TS-CBOR diagnostics query not yet implemented
				ts_status_debug( "ts_service_handler: diagnostics requested, and ignored,...\n" );
				status = TsStatusOk;

			} else if( strcmp( kind, "ts.event.firewall" ) == 0 ) {

				// firewall, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				if( service->_firewall != NULL ) {

					status = ts_firewall_handle( service->_firewall, message );

				} else {

					ts_status_alarm( "ts_service_handler: firewall request on unavailable service,...\n" );
					status = TsStatusErrorNotImplemented;
				}

			} else if( strcmp( kind, "ts.event.logconfig" ) == 0 ) {

				// logconfig, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				if( service->_logconfig != NULL ) {

					status = ts_logconfig_handle( service->_logconfig, message );

				} else {

					ts_status_alarm( "ts_service_handler: logconfig request on unavailable service,...\n" );
					status = TsStatusErrorNotImplemented;
				}

			} else if( strcmp( kind, "ts.event.credential" ) == 0 ) {
					ts_status_debug( "ts_service_handler: scepconfig request...\n" );

				// scepconfig, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				if( service->_scepconfig != NULL ) {

					status = ts_scepconfig_handle( service->_scepconfig, message );

				} else {

					ts_status_alarm( "ts_service_handler: scepconfig request on unavailable service,...\n" );
					status = TsStatusErrorNotImplemented;
				}
			} else if( strcmp( kind, "ts.event.cert" ) == 0 ) {
				ts_status_debug("ts_event.cert: ts-cbor\n");

				// cert acknowledgement
				// and must be returned with the correct status

				status = ts_handle_certack( message );
				return status;

			} else if( strcmp( kind, "ts.event.cert.renew" ) == 0 ) {
				ts_status_debug("ts_event.cert.renew: ts-cbor\n");

				// cert renew request, note that the message will be modified 'in-place'
				// and must be returned with the correct status

				status = ts_certrenew_handle( message );

			} else if( strcmp( kind, "ts.event.cert.rewoke" ) == 0 ) {
				ts_status_debug("ts_event.cert.rewoke: ts-cbor\n");

				// cert rewoke request, note that the message will be modified 'in-place'
				// and must be returned with the correct status

				status = ts_certrewoke_handle( message );

			} else if( strcmp( kind, "ts.event.version" ) == 0 ) {

				// version request, note that the message will be modified 'in-place'
				// and must be returned with the correct status

				status = ts_version_handle( message );

			} else if ( strcmp( kind, "ts.event.suspend" ) == 0 ) {

				// ODS suspend messages, note that the message will be modified 'in-place'
				// and must be returned with the correct status

				status = ts_suspend_handle( message );
			}
			else if( ( strcmp( kind, "ts.device" ) == 0 ) || ( strcmp( kind, "ts.element" ) == 0 ) ) {

				// provisioning, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				// TODO - TS-CBOR provisioning not yet implemented
				ts_status_debug( "ts_service_handler: provisioning requested, and ignored,...\n" );
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
		ts_message_set_int( message, "error", status );
	}

	// get device-id from controller (via connection)
	const uint8_t id[ TS_DRIVER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_DRIVER_MAX_ID_SIZE );

	// encode copy to send buffer
	// i.e., encode and send unsolicited message
	// get mtu from controller (via connection)
	uint32_t mtu;
	ts_connection_get_spec_mtu( transport->_connection, &mtu);

	// allocate data buffer
	uint8_t * response_buffer = ts_get_buffer( (size_t *)&mtu, MIN_MTU );
	if (response_buffer == NULL) {
		ts_status_alarm("ts_service_handler: could not allocate response buffer\n");
		return TsStatusErrorOutOfMemory;
	}

	size_t response_buffer_size = (size_t)(mtu - 4);

	// encode response message
	ts_message_encode(message, TsEncoderTsCbor, response_buffer + 4, &response_buffer_size);

	// encode response envelop
	response_buffer[ 0 ] = TsServiceEnvelopeVersionOne;
	response_buffer[ 1 ] = TsServiceEnvelopeServiceIdTsCbor;
	response_buffer[ 2 ] = (uint8_t)(response_buffer_size >> 8);
	response_buffer[ 3 ] = (uint8_t)(response_buffer_size & 0xff);
	response_buffer_size = response_buffer_size + 4;

	// send data
	size_t topic_size = 256;
	char topic[ 256 ];
	snprintf( topic, topic_size, "ThingSpace/%s/ElementToProvider", id );
	ts_transport_speak( transport, (TsPath_t)topic, response_buffer, response_buffer_size );

	// clean-up and return
	ts_platform_free( response_buffer, mtu );
	ts_message_destroy( message );
	return TsStatusOk;
}
