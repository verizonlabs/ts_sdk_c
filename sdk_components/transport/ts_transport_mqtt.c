// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_status.h"
#include "ts_connection.h"
#include "ts_transport.h"
#include "ts_transport_mqtt.h"

/* The following #define was added for the sake of removing
 * the duplicate symbol, MQTTIsConnected (inline) generated
 * when building using, cc (xcode). Note MQTTClient.h had to
 * be modified to support this compiler directive.
 */
#define TS_REMOVE_INLINE_MQTTISCONNECTED

#include "MQTTClient.h"

static TsStatus_t ts_create( TsTransportRef_t * );
static TsStatus_t ts_destroy( TsTransportRef_t );
static TsStatus_t ts_tick( TsTransportRef_t, uint32_t );

static TsStatus_t ts_get_connection( TsTransportRef_t, TsConnectionRef_t * );

static TsStatus_t ts_dial( TsTransportRef_t, TsAddress_t );
static TsStatus_t ts_hangup( TsTransportRef_t );
static TsStatus_t ts_listen( TsTransportRef_t, TsAddress_t, TsPath_t, TsTransportHandler_t, void * );
static TsStatus_t ts_speak( TsTransportRef_t, TsPath_t, const uint8_t *, size_t );

TsTransportVtable_t ts_transport_mqtt = {

	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.get_connection = ts_get_connection,

	.dial = ts_dial,
	.hangup = ts_hangup,
	.listen = ts_listen,
	.speak = ts_speak,

};

static int paho_mqtt_read( Network *, unsigned char *, int, int );
static int paho_mqtt_write( Network *, unsigned char *, int, int );
static void paho_mqtt_disconnect( Network * );
static void paho_mqtt_callback( MessageData * );

static MQTTPacket_connectData default_connection = MQTTPacket_connectData_initializer;

typedef struct TsTransportMqttHandler {
	TsTransportHandler_t handler;
	TsTransportRef_t transport;
	void * data;
} TsTransportMqttHandler_t;

// TODO - there is only one handler - which forces only one listener and one transport of this type
static TsTransportMqttHandler_t _default_handler = { NULL, NULL, NULL };

typedef struct TsTransportMqtt * TsTransportMqttRef_t;
typedef struct TsTransportMqtt {

	// inheritance by encapsulation; must be the first
	// attribute in order to treat this struct as a
	// TsTransport struct
	TsTransport_t _transport;

	// mqtt
	Network _network;
	MQTTClient _client;
	MQTTPacket_connectData _connection;
	uint8_t * _read_buffer;
	uint8_t * _write_buffer;
	uint32_t _read_write_buffer_size;
	char _id[TS_DRIVER_MAX_ID_SIZE];

	// mqtt spec parameters
	enum QoS _spec_qos;

} TsTransportMqtt_t;

static TsStatus_t ts_create( TsTransportRef_t * transport ) {

	ts_status_trace( "ts_transport_create: mqtt\n" );
	ts_platform_assert( transport != NULL );

	// create related connection
	TsConnectionRef_t connection;
	TsStatus_t status = ts_connection_create( &connection );
	if( status != TsStatusOk ) {
		*transport = NULL;
		return status;
	}

	// create and initialize this transport state
	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) ( ts_platform_malloc( sizeof( TsTransportMqtt_t )));
	*transport = (TsTransportRef_t) mqtt;

	// NOTE - the transport attribute, "handler" isn't used due to
	// (poort) paho design, we have to use a local static variable,
	// i.e., '_default_handler'.

	mqtt->_transport._connection = connection;
	mqtt->_network._connection = connection;
	mqtt->_network._last_status = TsStatusOk;
	mqtt->_network.mqttread = paho_mqtt_read;
	mqtt->_network.mqttwrite = paho_mqtt_write;
	mqtt->_network.disconnect = paho_mqtt_disconnect;

	// get controller specifications for id, timeout and buffer sizes
	uint32_t mtu, budget;
	ts_connection_get_spec_id( connection, (const uint8_t *) &( mqtt->_id ), TS_DRIVER_MAX_ID_SIZE );
	ts_connection_get_spec_budget( connection, &budget );
	ts_connection_get_spec_mtu( connection, &mtu );

	// initialize mqtt connection configuration (used for MQTTConnect)
	memcpy( &( mqtt->_connection ), &default_connection, sizeof( MQTTPacket_connectData ));
	mqtt->_connection.willFlag = 0;                 // no will
	mqtt->_connection.MQTTVersion = 3;              // version 3
	mqtt->_connection.clientID.cstring = mqtt->_id; // client-id is the device-id
	mqtt->_connection.username.cstring = NULL;      // not used
	mqtt->_connection.password.cstring = NULL;      // not used
	mqtt->_connection.keepAliveInterval = 60;       // keep-alive is 60 seconds
	mqtt->_connection.cleansession = 0;             // no clean-session

	// initialize mqtt intermediate buffers
	mqtt->_read_buffer = ts_platform_malloc( mtu );
	if (mqtt->_read_buffer == NULL) {
		ts_status_alarm("ts_transport_create: could not allocate read buffer\n");
		return TsStatusErrorOutOfMemory;
	}
	mqtt->_write_buffer = ts_platform_malloc( mtu );
	if (mqtt->_write_buffer == NULL) {
		ts_status_alarm("ts_transport_create: could not allocate write buffer\n");
		return TsStatusErrorOutOfMemory;
	}
	mqtt->_read_write_buffer_size = mtu;

	// initialize mqtt spec parameters
	mqtt->_spec_qos = QOS1;

	// initialize mqtt client
	MQTTClientInit(
		&( mqtt->_client ),
		&( mqtt->_network ),
		budget/1000,
		mqtt->_write_buffer,
		mqtt->_read_write_buffer_size,
		mqtt->_read_buffer,
		mqtt->_read_write_buffer_size
	);

	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsTransportRef_t transport ) {

	ts_status_trace( "ts_transport_destroy\n" );
	ts_platform_assert( transport != NULL );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;

	ts_connection_destroy( mqtt->_transport._connection );
	ts_platform_free( mqtt->_read_buffer, mqtt->_read_write_buffer_size );
	ts_platform_free( mqtt->_write_buffer, mqtt->_read_write_buffer_size );
	ts_platform_free( mqtt, sizeof( TsTransportMqtt_t ));

	return TsStatusOk;
}

static TsStatus_t ts_tick( TsTransportRef_t transport, uint32_t budget ) {

	ts_status_debug( "ts_transport_tick\n" );
	ts_platform_assert( transport != NULL );
	ts_platform_assert( budget > 0 );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;
	uint64_t timestamp = ts_platform_time();

	// provide connection tick
	ts_connection_tick( mqtt->_transport._connection, budget );

	// provide mqtt tick
	if(( mqtt->_client.isconnected ) && ( ts_platform_time() - timestamp < budget )) {
		int code = MQTTYield( &( mqtt->_client ), budget/TS_TIME_MSEC_TO_USEC );
		if( code != 0 ) {
			ts_status_alarm( "ts_transport_tick: mqtt yield failed, %d, ignoring,...\n", code );
		}
	} else {
		ts_status_alarm( "ts_transport_tick: not connected.\n" );
	}

	// report budget status and return
	timestamp = ts_platform_time() - timestamp;
	if( timestamp > budget + TS_TIME_MSEC_TO_USEC ) {
		ts_status_alarm( "ts_transport_tick: exceeded time budget, %d msec\n", timestamp/TS_TIME_MSEC_TO_USEC );
	}

	return TsStatusOk;
}

static TsStatus_t ts_get_connection( TsTransportRef_t transport, TsConnectionRef_t * connection ) {

	ts_status_trace( "ts_transport_tick\n" );
	ts_platform_assert( transport != NULL );
	ts_platform_assert( connection != NULL );

	*connection = transport->_connection;

	return TsStatusOk;
}

/**
 * Dial establishes a network connection. Use "dial" to establish the host connection, then
 * "listen" to subscribe, and "speak" to publish on that connection.
 * @param transport
 * @param address
 * @return
 */
static TsStatus_t ts_dial( TsTransportRef_t transport, TsAddress_t address ) {

	ts_status_trace( "ts_transport_dial\n" );
	ts_platform_assert( transport != NULL );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;

	if( mqtt->_client.isconnected ) {
		ts_status_debug( "ts_transport_dial: failed, mqtt already connected\n" );
		return TsStatusErrorPreconditionFailed;
	}

	// NOTE - the transport attribute, "handler" isn't used due to
	// (poor) paho design, we have to use a local static variable,
	// i.e., '_default_handler'.
	ts_status_debug( "ts_transport_dial: connecting to, '%s'\n", address );
	TsStatus_t status = ts_connection_connect( mqtt->_transport._connection, address );
	if( status != TsStatusOk ) {
		ts_status_debug("ts_transport_dial: TLS handshake failed\n");
		return status;
	}
	ts_status_debug( "ts_transport_dial: connected to server\n" );

	mqtt->_network._last_status = TsStatusOk;
	int code = MQTTConnect( &( mqtt->_client ), &( mqtt->_connection ));
	if( code < 0 ) {
		ts_status_debug( "ts_transport_dial: failed due to mqtt error, %d\n", code );
		ts_connection_disconnect( mqtt->_transport._connection );
		return mqtt->_network._last_status == TsStatusOk ? TsStatusErrorInternalServerError : mqtt->_network._last_status;
	}

	return TsStatusOk;
}

static TsStatus_t ts_hangup( TsTransportRef_t transport ) {

	ts_status_trace( "ts_transport_hangup\n" );
	ts_platform_assert( transport != NULL );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;

	MQTTDisconnect( &( mqtt->_client ));
	ts_connection_disconnect( mqtt->_transport._connection );

	return TsStatusOk;
}

/**
 * Listen will subscribe to a particular path and deliver messages to the caller via the given handler. Note
 * that this transport doesnt have a sense of "listen" without dialing first (i.e., server), so the address
 * parameters is ignored.
 * @param transport
 * @param address
 * Ignored for MQTT, NULL is a valid value.
 * @param path
 * @param handler
 * @return
 */
static TsStatus_t ts_listen( TsTransportRef_t transport, TsAddress_t address, TsPath_t path, TsTransportHandler_t handler, void * handler_data ) {

	ts_status_debug( "ts_transport_listen\n" );
	ts_platform_assert( transport != NULL );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;

	if( !( mqtt->_client.isconnected )) {
		ts_status_debug( "ts_transport_listen: failed, mqtt not connected\n" );
		return TsStatusErrorPreconditionFailed;
	}

	// TODO - change so that we can target a handler by address and path
	// NOTE - the transport attribute, "handler" isn't used due to
	// (poor) paho design, we have to use a local static variable,
	// i.e., '_default_handler'.
	_default_handler.transport = transport;
	_default_handler.handler = handler;
	_default_handler.data = handler_data;

	// subscribe
	ts_status_debug( "ts_transport_listen: listening to, '%s'\n", path );
	mqtt->_network._last_status = TsStatusOk;
	int code = MQTTSubscribe( &( mqtt->_client ), (char *) path, mqtt->_spec_qos, paho_mqtt_callback );
	if( code < 0 ) {
		ts_status_debug( "ts_transport_listen: failed due to mqtt error, %d\n", code );
		return mqtt->_network._last_status == TsStatusOk ? TsStatusErrorInternalServerError : mqtt->_network._last_status;
	}
	return TsStatusOk;
}

static TsStatus_t ts_speak( TsTransportRef_t transport, TsPath_t path, const uint8_t * buffer, size_t buffer_size ) {

	ts_status_trace( "ts_transport_speak\n" );
	ts_platform_assert( transport != NULL );

	TsTransportMqttRef_t mqtt = (TsTransportMqttRef_t) transport;

	if( !( mqtt->_client.isconnected )) {
		ts_status_debug( "ts_transport_speak: failed, mqtt not connected\n" );
		return TsStatusErrorPreconditionFailed;
	}

	MQTTMessage message;
	message.dup = 0;
	message.id = 0;
	message.payload = (void *) buffer;
	message.payloadlen = buffer_size;
	message.qos = mqtt->_spec_qos;
	message.retained = 0;
	mqtt->_network._last_status = TsStatusOk;
	int code = MQTTPublish( &( mqtt->_client ), (const char *) path, &message );
	if( code != 0 ) {
		ts_status_debug( "ts_speak: failed due to mqtt error, %d\n", code );
		return mqtt->_network._last_status == TsStatusOk ? TsStatusErrorInternalServerError : mqtt->_network._last_status;
	}

	return TsStatusOk;
}

void TimerInit( Timer * timer ) {
	timer->end_time = 0;
}

char TimerIsExpired( Timer * timer ) {
	uint64_t now = ts_platform_time();
	return timer->end_time <= now;
}

void TimerCountdownMS( Timer * timer, unsigned int ms ) {
	uint64_t now = ts_platform_time();
	timer->end_time = now + ms*1000;
}

void TimerCountdown( Timer * timer, unsigned int sec ) {
	uint64_t now = ts_platform_time();
	timer->end_time = now + sec*1000000;
}

int TimerLeftMS( Timer * timer ) {
	uint64_t now = ts_platform_time();
	return timer->end_time <= now ? 0 : (int) ( timer->end_time - now )/1000;
}

static int paho_mqtt_read( Network * network, unsigned char * buffer, int buffer_size, int budget_ms ) {

	ts_status_trace( "paho_mqtt_read\n" );
	ts_platform_assert( network != NULL );
	ts_platform_assert( network->_connection != NULL );

	int index = 0;
	bool reading = true;

	// Paho gives us time budgets in milliseconds
	uint32_t budget = (uint32_t)budget_ms * TS_TIME_MSEC_TO_USEC;
	uint64_t timestamp = ts_platform_time();
	do {
		size_t xbuffer_size = (size_t) ( buffer_size - index );
		ts_connection_tick(network->_connection, 100 * TS_TIME_MSEC_TO_USEC);
		TsStatus_t status = ts_connection_read( network->_connection, (uint8_t *) ( buffer + index ), &xbuffer_size, budget );
		switch( status ) {
		default:
			ts_status_debug( "paho_mqtt_read: %s\n", ts_status_string( status ));
			network->_last_status = status;
			return -1;

		case TsStatusErrorConnectionReset:
			ts_status_debug( "paho_mqtt_read: %s\n", ts_status_string( status ));
			network->_last_status = status;
			return -1;

		case TsStatusOkReadPending:
			// continue reading till budget exhausted
			xbuffer_size = 0;
			// fallthrough

		case TsStatusOk:
			break;
		}

		index = index + (int) xbuffer_size;
		if( index >= buffer_size ) {
			reading = false;
		} else if( ts_platform_time() - timestamp > budget ) {
			// ts_status_debug( "paho_mqtt_read: budget exhausted, returning control to caller,...\n" );
			reading = false;
		}

	} while( reading );
	return index;
}

static int paho_mqtt_write( Network * network, unsigned char * buffer, int buffer_size, int budget_ms ) {

	ts_status_trace( "paho_mqtt_write\n" );
	ts_platform_assert( network != NULL );
	ts_platform_assert( network->_connection != NULL );

	int index = 0;
	bool writing = true;

	// Paho gives us time budgets in milliseconds
	uint32_t budget = (uint32_t)budget_ms * TS_TIME_MSEC_TO_USEC;
	uint64_t timestamp = ts_platform_time();
	do {
		size_t xbuffer_size = (size_t) ( buffer_size - index );
		TsStatus_t status = ts_connection_write( network->_connection, (uint8_t *) ( buffer + index ), &xbuffer_size, budget );
		ts_connection_tick(network->_connection, 100 * TS_TIME_MSEC_TO_USEC);
		switch( status ) {
		default:
			ts_status_debug( "paho_mqtt_write: %s\n", ts_status_string( status ));
			network->_last_status = status;
			return -1;

		case TsStatusErrorConnectionReset:
			ts_status_debug( "paho_mqtt_write: %s\n", ts_status_string( status ));
			network->_last_status = status;
			return -1;

		case TsStatusOkWritePending:
			// continue writing till budget exhausted
			xbuffer_size = 0;
			// fallthrough

		case TsStatusOk:
			break;
		}

		index = index + (int) xbuffer_size;
		if( index >= buffer_size ) {
			writing = false;
		} else if( ts_platform_time() - timestamp > budget ) {
			ts_status_debug( "paho_mqtt_write: budget exhausted, returning control to caller,...\n" );
			writing = false;
		}

	} while( writing );
	return index;
}

static void paho_mqtt_disconnect( Network * network ) {

	ts_status_debug( "paho_mqtt_disconnect\n" );
	ts_platform_assert( network != NULL );
	ts_platform_assert( network->_connection != NULL );

	// network disconnect?
}

static void paho_mqtt_callback( MessageData * data ) {

	ts_status_debug( "paho_mqtt_callback\n" );
	ts_platform_assert( data != NULL );
	ts_platform_assert( _default_handler.handler != NULL );

	MQTTMessage * message = data->message;
	ts_status_debug( "paho_mqtt_callback: %.*s, '%.*s'\n",
		data->topicName->lenstring.len, data->topicName->lenstring.data,
		(int) message->payloadlen, (char *) message->payload );

	// NOTE - the transport attribute, "handler" isn't used due to
	// (poort) paho design, we have to use a local static variable,
	// i.e., '_default_handler'.
	_default_handler.handler( _default_handler.transport, _default_handler.data, (TsPath_t) ( data->topicName ), message->payload, message->payloadlen );
}
