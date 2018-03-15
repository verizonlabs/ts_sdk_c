/**
 * @file
 * ts_transport.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A application layer message broker or simple message passing interface.
 *
 * @details
 * The transport interface allows the user to access the messaging layer that
 * allows communication between applications (i.e., client to/from server).
 *
 * We currently only provide MQTT, but in the future we may add support for
 * CoAP, XMPP, and others. Typically, the developer would access this component
 * via the "service" component (e.g., ts_service_connect...) however some applications
 * may be required to access the transport directly as shown below.
 *
 * @code
 *
 * 	TsTransportRef_t transport;
 * 	ts_transport_create( &transport );
 *
 * 	// connect to the far-end
 * 	ts_transport_connect( transport, "my_mqtt_server.com:1883" );
 *
 * 	// set up a callback for the listened-to topic, called when data is received
 * 	ts_transport_listen( transport, NULL, "My/Incomming/Topic", handler, handler_data );
 *
 * 	// write and wait for reads
 * 	while( listening ) {
 *
 * 		// send something to the far-end
 * 		ts_transport_speak( transport, "My/Outgoing/Topic", buffer, buffer_size );
 *
 * 		// give up processing time to the transport and its dependent components
 * 		ts_transport_tick( transport, 5 * TS_TIME_SEC_TO_USEC );
 * 	}
 *
 * @endcode
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) runtime. They provide the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */
#ifndef TS_TRANSPORT_H
#define TS_TRANSPORT_H

#include "ts_status.h"
#include "ts_address.h"
#include "ts_message.h"
#include "ts_connection.h"

/**
 * The transport object reference
 */
typedef struct TsTransport * TsTransportRef_t;

/**
 * The transport object
 */
typedef TsStatus_t (*TsTransportHandler_t)( TsTransportRef_t, void *, TsPath_t, const uint8_t *, size_t );

typedef struct TsTransport {
	TsConnectionRef_t       _connection;
	TsTransportHandler_t    _handler;
	void *                  _handler_data;
} TsTransport_t;

/**
 * The transport vector table (i.e., the transport "class" definition), used to define the transport SDK-aspect.
 * See sdk_components/transport for available transport implementations, or use your own customized implementation.
 */
typedef struct TsTransportVtable {

	/**
	 * Allocate and initialize a new transport. This function is typically called from ts_service.
	 *
	 * @param transport
	 * [on/out] The pointer to a pre-existing TsTransportRef_t, which will be initialized with the transport state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* create)( TsTransportRef_t * );

	/**
	 * Deallocate the given transport object.
	 *
	 * @param transport
	 * [in] The transport state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* destroy)( TsTransportRef_t );

	/**
	 * Provide the given service object processing time according to the given budget "recommendation".
	 * This function is typically called from the application.
	 *
	 * @param service
	 * [in] The service state.
	 *
	 * @param budget
	 * [in] The recommended time in microseconds budgeted for the function
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* tick)( TsTransportRef_t, uint32_t );

	/**
	 * Return the held ts_connection state object.
	 *
	 * @param transport
	 * [in] The transport object
	 *
	 * @param connection
	 * [in/out] A pointer to a variable that should be filled with this object's connection object
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* get_connection)( TsTransportRef_t, TsConnectionRef_t* );

	/**
	 * Create a connection to the given server adddress and negotiate access to the transport.
	 * Note that this will use the underlying ts_connection component to manage the TCP/IP stack
	 * on the configured platform.
	 *
	 * @param transport
	 * [in] The transport object
	 *
	 * @param address
	 * [in] The destination address. A FQDN may require resolution prior to calling this function based
	 * on the underlying ts_security, ts_controller or ts_driver capabilities, e.g., Mocana TLS expects an IP.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* dial)( TsTransportRef_t, TsAddress_t );

	/**
	 * Destroy (i.e., tear down) the current connection. Note that this will used the underlying
	 * ts_connection component to tear down the connection.
	 *
	 * @param transport
	 * [in] The transport object
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* hangup)( TsTransportRef_t );

	/**
	 * Set a callback used when a message has been received by the transport. Note that
	 * the address parameter is currently not used, and this function may only be called on
	 * a pre-existing connection (i.e., ts_transport_dial must be called first).
	 *
	 * @param transport
	 * [in] The transport state
	 *
	 * @param address
	 * [in] The address to listen to, must be set to NULL in this verison of the SDK.
	 *
	 * @param path
	 * [in] The resource path to listen to, e.g., for MQTT this would be the subscribed queue topic
	 *
	 * @param handler
	 * [in] The function to call when a message has been received
	 *
	 * @param handler_data
	 * [in] A optional pointer sent to the handler when a message has been received
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* listen)( TsTransportRef_t, TsAddress_t, TsPath_t, TsTransportHandler_t, void* );

	/**
	 * Write to the driver (blocking) via the security and controller components.
	 *
	 * @param transport
	 * [in] The transport state
	 *
	 * @param path
	 *
	 * @param buffer
	 * [in] The pre-allocated buffer memory containing the data to be written.
	 *
	 * @param buffer_size
	 * [in] The pre-allocated buffer data size
	 * [out] The actual number of byte written
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* speak)( TsTransportRef_t, TsPath_t, const uint8_t *, size_t );

} TsTransportVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsTransportVtable_t * ts_transport;

#define ts_transport_create ts_transport->create
#define ts_transport_destroy ts_transport->destroy
#define ts_transport_tick   ts_transport->tick

#define ts_transport_get_connection ts_transport->get_connection

#define ts_transport_dial   ts_transport->dial
#define ts_transport_hangup ts_transport->hangup
#define ts_transport_listen ts_transport->listen
#define ts_transport_speak  ts_transport->speak

#ifdef __cplusplus
extern "C" {
#endif

#endif // TS_TRANSPORT_H
