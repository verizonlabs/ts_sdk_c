/**
 * @file
 * ts_service.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The protocol and connectivity services.
 *
 * @details
 * This top-level interface is used to communicate to ThingSpace and support ThingSpace connectivity
 * services, such as, Diagnostics and Firewall-management. There are two version currently available,
 * - TS-JSON: An older deprecated protocol kept for backward compatibility. Do not use unless required to port older applications.
 * - TS-CBOR: The simplified, expressive and compressed ThingSpace protocol.
 *
 * @code
 *
 * 	static TsMessageRef_t sensor;
 *
 *	int main() {
 *
 *		// initialize (implemented per platform)
 *		ts_platform_initialize();
 *
 *		// initialize sensor cache (its contents would come from the actual hardware)
 *		ts_message_create( &sensor );
 *		ts_message_set_float( sensor, "temperature", 50.5 );
 *
 *		// initialize the service
 *		TsServiceRef_t service;
 *		ts_service_create( &service );
 *
 *		// register a message handler
 *		ts_service_dequeue( service, TsServiceActionMaskAll, handler );
 *
 *		// connect to the cloud
 *		ts_service_dial( service, address );
 *
 *		// start a single-threaded run-loop
 *		while( running ) {
 *
 *			// send telemetry at some interval,...
 *			ts_service_enqueue( service, sensor );
 *
 *			// pass control to the SDK for a period of time,...
 *			ts_service_tick( service, 5 * TS_TIME_SEC_TO_USEC );
 *		}
 *
 *		// clean-up
 *		ts_service_disconnect( service );
 *		ts_service_destroy( service );
 *	}
 *
 *	// my message handler
 *	static TsStatus_t handler( TsServiceRef_t service, TsServiceAction_t action, TsMessageRef_t message ) {
 *
 *		// ... act on action and message (not shown)...
 *
 *		return TsStatusOk;
 *	}
 *
 * @endcode
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) run-time. They provide the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */

#ifndef TS_SERVICE_H
#define TS_SERVICE_H

#include "ts_message.h"
#include "ts_transport.h"
#include "ts_firewall.h"
#include "ts_log.h"
#include "ts_cert.h"

#define TS_SERVICE_MAX_HANDLERS 8
#define TS_SERVICE_MAX_PATH_SIZE 256

typedef enum {
	TsServiceEnvelopeVersionOne = 0x01,
} TsServiceEnvelopeVersion_t;

typedef enum {
	TsServiceEnvelopeServiceIdTsCbor = 0x03,
	TsServiceEnvelopeServiceIdZWave = 0x04,
} TsServiceEnvelopeServiceId_t;

// TODO - fix to one enum, and one mapping
typedef enum {

	TsServiceActionGet = 0x0001,
	TsServiceActionSet = 0x0002,
	TsServiceActionActivate = 0x0010,
	TsServiceActionDeactivate = 0x0020,
	TsServiceActionSuspend = 0x0040,
	TsServiceActionResume = 0x0080,
	TsServiceActionMaskAll = 0xFFFF,
	TsServiceActionMaskChannel = 0x000F,
	TsServiceActionMaskProvision = 0x00F0,

} TsServiceAction_t;

typedef enum {

	TsServiceActionGetIndex = 0,
	TsServiceActionSetIndex = 1,
	TsServiceActionActivateIndex = 4,
	TsServiceActionDeactivateIndex = 5,
	TsServiceActionSuspendIndex = 6,
	TsServiceActionResumeIndex = 7,
} TsServiceActionIndex_t;

/**
 * The service object reference
 */
typedef struct TsService * TsServiceRef_t;

/**
 * The message handler (i.e., callback)
 *
 * @param service
 * [in] The service state.
 *
 * @param action
 * [in] The action received, see TsServiceAction_t
 *
 * @param message
 * [in] The message received, and,
 * [out] The optional values returned. This is only used by the "get" action currently.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 *
 * @code
 *
 * 	// main run-loop
 * 	int main() {
 *
 * 		...
 *
 * 		// set the action handler
 * 		ts_service_dequeue( service, TsServiceActionMaskAll, my_handler );
 *
 * 		...
 * 	}
 *
 * 	// my action handler
 * 	TsStatus_t my_handler( TsServiceRef_t service, TsServiceAction_t action, TsMessageRef_t message ) {
 *
 * 		switch( action ) {
 * 		case TsServiceActionSet:
 *
 * 			// ... get the actuator value given in the message ...
 * 			float temperature;
 * 			ts_message_get_float( message, "temperature", &temperature );

 * 			//... set the actuator ...
 * 			break;
 *
 * 		case TsServiceActionGet:
 *
 * 			// ... get the sensor value indicated by the message ...
 * 			temperature = 12.0;
 *
 * 			// ... set the message value found ...
 * 			ts_message_set_float( message, "temperature", temperature );
 * 			break;
 *
 * 		... etc. ...
 * 		}
 * 	}
 *
 * @endcode
 */
typedef TsStatus_t (* TsServiceHandler_t)( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );

/**
 * The service object
 */
typedef struct TsService {
	char                _subscription[TS_SERVICE_MAX_PATH_SIZE];
	TsServiceHandler_t  _handlers[TS_SERVICE_MAX_HANDLERS];
	TsTransportRef_t    _transport;
	TsFirewallRef_t     _firewall;
	TsLogConfigRef_t	_logconfig;
	TsScepConfigRef_t	_scepconfig;
} TsService_t;

/**
 * The service vector table (i.e., the service "class" definition), used to define the service SDK-aspect.
 * See sdk_components/service for available service implementations, or use your own customized implementation.
 */
typedef struct TsServiceVtable {

	/**
	 * Allocate and initialize a new service. This function is typically called from the application.
	 *
	 * @param service
	 * [on/out] The pointer to a pre-existing TsServiceRef_t, which will be initialized with the service state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*create)( TsServiceRef_t * );

	/**
	 * Deallocate the given service object.
	 *
	 * @param service
	 * [in] The service state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*destroy)( TsServiceRef_t );

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
	TsStatus_t (*tick)( TsServiceRef_t, uint32_t );

	/**
	 * Send the given sensor readings to the server.
	 *
	 * @param service
	 * [in] The service state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 *
	 * @code
	 *
	 * 	TsMessageRef_t sensor;
	 * 	ts_message_create( &sensor );
	 *
	 * 	ts_message_set_float( sensor, "temperature", 52.0 );
	 * 	ts_service_enqueue( service, sensor );
	 *
	 * 	ts_message_destroy( sensor );
	 *
	 * @endcode
	 */
	TsStatus_t (*enqueue)( TsServiceRef_t, TsMessageRef_t );


	TsStatus_t (*enqueuetyped)( TsServiceRef_t, char*, TsMessageRef_t );

	/**
	 * Set the callback used for de-queuing messages from the underlying transport, routed by action
	 *
	 * @param service
	 * [in] The service state.
	 *
	 * @param action
	 * [in] The action or set of actions (i.e., TsServiceActionMaskAll) that are routed to the given handler
	 *
	 * @param handler
	 * [in] The function called when one of the actions indicated above is received from the server
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*dequeue)( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

} TsServiceVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsServiceVtable_t * ts_service;

TsStatus_t ts_service_create( TsServiceRef_t * );
TsStatus_t ts_service_destroy( TsServiceRef_t );
TsStatus_t ts_service_tick( TsServiceRef_t, uint32_t );

TsStatus_t ts_service_set_server_cert_hostname( TsServiceRef_t, const char * );
TsStatus_t ts_service_set_server_cert( TsServiceRef_t, const uint8_t *, size_t );
TsStatus_t ts_service_set_client_cert( TsServiceRef_t, const uint8_t *, size_t );
TsStatus_t ts_service_set_client_key( TsServiceRef_t, const uint8_t *, size_t );

TsStatus_t ts_service_dial( TsServiceRef_t, TsAddress_t );
TsStatus_t ts_service_hangup( TsServiceRef_t );

TsStatus_t ts_service_enqueue( TsServiceRef_t, TsMessageRef_t );
TsStatus_t ts_service_dequeue( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );
TsStatus_t ts_service_enqueue_typed( TsServiceRef_t, char*, TsMessageRef_t );
#ifdef __cplusplus
}
#endif

#endif // TS_SERVICE_H
