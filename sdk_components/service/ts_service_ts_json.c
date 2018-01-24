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

TsServiceVtable_t ts_service_ts_json = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,
	.enqueue = ts_enqueue,
	.dequeue = ts_dequeue,
};

// TODO - clean-up controller x connection confusion (i.e., ...MAX_ID_SIZE)
static TsStatus_t ts_create( TsServiceRef_t * service ) {

	ts_status_trace("ts_service_create: ts-json\n");

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

	const char * unit_name = "unit-name";
	const char * unit_serial_number = "unit-serial-number";

	// get device-id from controller (via connection)
	const uint8_t id[ TS_CONTROLLER_MAX_ID_SIZE ];
	ts_connection_get_spec_id( service->_transport->_connection, id, TS_CONTROLLER_MAX_ID_SIZE );

	// create message content
	TsMessageRef_t message, sensors, characteristics;
	ts_message_create(&message);
	ts_message_set_string(message, "unitName", (char*)unit_name);
	ts_message_set_string(message, "unitMacId", (char*)id );
	ts_message_set_string(message, "unitSerialNo", (char*)unit_serial_number);
	ts_message_create_message(message, "sensor", &sensors);
	ts_message_create_array(sensors, "characteristics", &characteristics);

	// for each field of the message,...
	for (int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++) {

		// the "for-each" behavior terminates on a NULL or max-size
		TsMessageRef_t branch = sensor->value._xfields[i];
		if (branch == NULL) {
			break;
		}

		/* transform into the form expected by the server */
		TsMessageRef_t characteristic;
		ts_message_create(&characteristic);
		ts_message_set_string(characteristic, "characteristicsName", branch->name);
		ts_message_set(characteristic, "currentValue", branch);
		ts_message_set_message_at(characteristics, (size_t)i, characteristic);
		ts_message_destroy(characteristic);
	}

	// encode copy to send buffer
	// i.e., encode and send unsolicited message
	// get mcu from controller (via connection)
	uint32_t mcu;
	ts_connection_get_spec_mcu( service->_transport->_connection, &mcu);

	// allocate data buffer
	uint8_t * buffer = (uint8_t*)ts_platform_malloc( mcu );
	size_t buffer_size = (size_t)mcu;
	ts_message_encode(message, TsEncoderJson, buffer, &buffer_size);

	// send data
	size_t topic_size = 256;
	char topic[ 256 ];
	snprintf( topic, topic_size, "ThingspaceSDK/%s/UNITOnBoard", id );
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
	snprintf( service->_subscription, TS_SERVICE_MAX_PATH_SIZE, "ThingspaceSDK/%s/TSServerPublishCommand", id );
	return ts_transport_listen( service->_transport, NULL, service->_subscription, handler, service );
}

static TsStatus_t handler_get( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_trace("ts_service_handler_get\n");

	// pull sensor name and handler
	char * name;
	ts_message_get_string( message, "characteristicsName", &name );
	TsServiceHandler_t service_handler = service->_handlers[ TsServiceActionGetIndex ];

	// delegate to service handler
	TsStatus_t status = TsStatusErrorNotFound;
	if( ( service_handler != NULL ) && ( name != NULL ) ) {

		// prep
		TsMessageRef_t payload, content;
		ts_message_create_message(message, "payload", &payload);
		ts_message_create_message(payload, name, &content);

		// delegate
		status = service_handler( service, TsServiceActionGet, payload );

	} else {

		ts_status_debug( "ts_service_handler_get: missing handler or message missing sensor name\n" );
	}

	// return status
	return status;
}

static TsStatus_t handler_set( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_trace( "ts_service_handler_set\n" );

	// pull actuator name, value and handler
	char * name;
	TsMessageRef_t data;
	ts_message_get_string( message, "characteristicsName", &name );
	ts_message_get( message, "value", &data );

	TsServiceHandler_t service_handler = service->_handlers[ TsServiceActionSetIndex ];

	TsStatus_t status = TsStatusErrorNotFound;
	if(( service_handler != NULL) && ( name != NULL)) {

		// prep
		TsMessageRef_t payload;
		ts_message_create( &payload );
		ts_message_set( payload, name, data );

		// delegate
		status = service_handler( service, TsServiceActionSet, payload );

		// clean-up local
		ts_message_destroy( payload );

	} else {

		ts_status_debug( "ts_service_handler_set: missing handler or message missing sensor name or value\n" );
	}

	// return status
	return status;
}

static TsStatus_t handler_activate( TsServiceRef_t service, TsMessageRef_t message ) {

	ts_status_trace("ts_service_handler_activate\n");

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

	ts_status_trace("ts_service_handler\n");

	ts_platform_assert( transport != NULL );
	ts_platform_assert( data != NULL );

	// cast data to service pointer (as set in the dequeue function above)
	TsServiceRef_t service = (TsServiceRef_t)data;

	// TODO - forward solicited message to handler (or drop if no handler written)
	// TODO - return response via handler message returned
	TsMessageRef_t message;
	ts_message_create( &message );
	TsStatus_t status = ts_message_decode( message, TsEncoderJson, (uint8_t*)buffer, buffer_size );
	if( status != TsStatusOk ) {

		ts_status_alarm( "ts_service_handler: message muddled, '%.*s'\n", buffer_size, buffer );

	} else {

		char * unit_command;
		status = ts_message_get_string( message, "unitCommand", &unit_command );
		switch( status ) {
		case TsStatusOk:

			if( strcmp(unit_command, "Get") == 0 ) {

				// get sensor, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				status = handler_get( service, message );

			} else if( strcmp( unit_command, "Set" ) == 0 ) {

				// set sensor
				status = handler_set( service, message );

			} else if( strcmp( unit_command, "GetOtp" ) == 0 ) {

				// get diagnostics, note that the message will be modified 'in-place'
				// and must be returned with the correct status
				// TODO - diagnostics query not yet implemented
				ts_status_debug( "ts_service_handler: diagnostics requested, and ignored,...\n" );
				status = TsStatusOk;

			} else {

				// error
				ts_status_alarm( "ts_service_handler: unknowned unit command, '%s'\n", unit_command );
				status = TsStatusErrorNotImplemented;
			}
			break;

		case TsStatusErrorNotFound:

			// onboard, note that the message will be modified 'in-place'
			// and must be returned with the correct status
			status = handler_activate( service, message );
			break;

		default:

			// error
			ts_status_alarm( "ts_service_handler: unexpected message parsing error, %s\n", ts_status_string( status ) );
			status = TsStatusErrorInternalServerError;
			break;
		}
	}

	// attempt to respond
	char * command_uuid;
	ts_message_get_string( message, "commandUUID", &command_uuid );
	if( command_uuid != NULL && strlen(command_uuid) > 0 ) {

		// modify given message to include status data
		ts_message_set_int( message, "statusCode", status );
		ts_message_set_string( message, "statusMsg", (char*)ts_status_string(status) );

		// encode copy to send buffer
		// i.e., encode and send unsolicited message
		// get mcu from controller (via connection)
		uint32_t mcu;
		ts_connection_get_spec_mcu( transport->_connection, &mcu);

		// allocate data buffer
		uint8_t * response_buffer = (uint8_t*)ts_platform_malloc( mcu );
		size_t response_buffer_size = (size_t)mcu;
		ts_message_encode(message, TsEncoderJson, response_buffer, &response_buffer_size);

		// send data
		size_t topic_size = 256;
		char topic[ 256 ];
		snprintf( topic, topic_size, "ThingspaceSDK/%s/UNITCmdResponse", command_uuid );
		ts_transport_speak( transport, (TsPath_t)topic, response_buffer, response_buffer_size );

		// clean-up and return
		ts_platform_free( response_buffer, response_buffer_size );

	} else {

		ts_status_alarm( "ts_service_handler: message command-uuid muddled, '%.*s'\n", buffer_size, buffer );
	}

	// clean-up and return
	ts_message_destroy( message );
	return TsStatusOk;
}