/**
 * @file
 * ts_controller.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A access controller to an underlying TCP/IP stack (e.g., typically for AT-command-based cellular modems)
 *
 * @details
 * The "controller" is usually called from the ts_security component, and encapsulates the ts_driver component
 * which "holds" the TCP/IP stack implementation. The developer usually doesnt use this component directly, nor
 * implement their own controller, unless they are customizing the SDK to use a new modem.
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) runtime. They define the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */
#ifndef TS_CONTROLLER_H
#define TS_CONTROLLER_H

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_driver.h"

/**
 * The controller object reference
 */
typedef struct TsController * TsControllerRef_t;

/**
 * The controller object, which holds the ts_driver state, _driver and any profile
 * information (such as diagnostics), _profile.
 */
typedef struct TsController {

	TsDriverRef_t _driver;
	TsProfileRef_t _profile;

} TsController_t;

/**
 * The controller vector table (i.e., the controller "type"), used to configure the SDK controller aspect.
 * See sdk_components/controller for available controllers, or use your own customized implementation.
 */
typedef struct TsControllerVtable {

	/**
	 * Allocate and initialize a new controller. This function will also
	 * create a ts_driver object. This function is typically called from ts_security.
	 *
	 * @param controller
	 * [in/out] The pointer to the target controller object variable being set.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* create)( TsControllerRef_t * controller );

	/**
	 * Deallocate the given controller. This function will also destroy the underlying
	 * ts_driver object. This function is typically called from ts_security.
	 *
	 * @param controller
 	 * [in] The controller object
 	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* destroy)( TsControllerRef_t controller );

	/**
	 * Provide the given controller some processing time according to the given budget "recommendation".
	 * This function will call the underlying ts_driver component. This function is typically called
	 * from ts_security.
	 *
	 * @param controller
	 * [in] The controller object
	 *
	 * @param budget
	 * [in] The recommended time in microseconds budgeted for the function
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* tick)( TsControllerRef_t controller, uint32_t );

	/**
	 * Create a TCP/IP connection to the given address. Note that this will use the underlying ts_driver
	 * component to establish the connection. This function is typically called from ts_security.
	 *
	 * @param controller
	 * [in] The controller object
	 *
	 * @param address
	 * [in] The destination address. A FQDN may require resolution prior to calling this function based
	 * on the underlying ts_driver capabilities (e.g., NimbeLink can resolve the FQDN itself).
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* connect)( TsControllerRef_t controller, TsAddress_t address );

	/**
	 * Destroy (i.e., tear down) the current TCP/IP connection. Note that this will use the underlying
	 * ts_driver component to tear down the connection. This function is typically called from ts_security.
	 *
	 * @param controller
	 * [in] The controller object
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (* disconnect)( TsControllerRef_t controller );

	/**
	 * Read from the tcp/ip driver (non-blocking) via the ts_driver component.
	 * This function is typically called from ts_security. Note that this will use
	 * the underlying read on the ts_driver component.
	 *
	 * @note
	 * TsStatusOkReadPending has a very specific meaning, only return when the read
	 * has returned pending and there isn't data in the buffer, in all other cases
	 * return a valid status with the contents of the current buffer.
	 *
	 * @param controller
	 * [in] The controller object
	 *
	 * @param buffer
	 * [in] The pre-allocated buffer memory
	 *
	 * @param buffer_size
	 * [in] The pre-allocated buffer memory size
	 * [out] The actual number of byte read
	 *
	 * @param budget
	 * [in] Recommended allotment of time in microseconds allowed wait for received bytes.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk, *buffer_size > 0
	 * 		- Successfully returned the given amount of read data
	 * - TsStatusOk, *buffer_size = 0
	 * 		- Indicates an end-of-file condition
	 * - TsStatusOkPendingRead
	 * 		- Indicates a blocking condition exists (and was avoided), note, *buffer_size is guaranteed to be zero when this condition occurs.
	 * - TsStatusErrorConnectionReset
	 * 		- Indicates that the connection was disrupted
	 * - TsStatusError[Code]
	 * 		- Additional errors are defined in, ts_status.h
	 */
	TsStatus_t (* read)( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget );

	/**
	 * Write to the driver (blocking) via the ts_driver component.
	 * This function is typically called from ts_security. Note that this will use
	 * the underlying write on the ts_driver component.
	 *
	 * @param controller
	 * [in] The controller state
	 *
	 * @param buffer
	 * [in] The pre-allocated buffer memory containing the data to be written.
	 *
	 * @param buffer_size
	 * [in] The pre-allocated buffer data size
	 * [out] The actual number of byte written
	 *
	 * @param budget
	 * [in] Recommended allotment of time in microseconds allowed to send bytes.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk, *buffer_size > 0
	 * 		- Successfully returned the given amount of written data
	 * - TsStatusOkPendingWrite
	 * 		- Indicates a blocking condition exists (and was avoided), note, *buffer_size is guaranteed to be zero when this condition occurs.
	 * - TsStatusErrorConnectionReset
	 * 		- Indicates that the connection was disrupted
	 * - TsStatusError[Code]
	 * 		- Additional errors are defined in, ts_status.h
	 */
	TsStatus_t (* write)( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget );

	TsStatus_t (* get_id)( TsControllerRef_t, char * );
	TsStatus_t (* get_rssi)( TsControllerRef_t, char * );
	TsStatus_t (* get_ipv4_addr)( TsControllerRef_t, char * );
	TsStatus_t (* get_iccid)( TsControllerRef_t, char * );
	TsStatus_t (* get_date_and_time)( TsControllerRef_t, char * );
	TsStatus_t (* get_imsi)( TsControllerRef_t, char * );
	TsStatus_t (* get_manufacturer)( TsControllerRef_t, char * );
	TsStatus_t (* get_module_name)( TsControllerRef_t, char * );
	TsStatus_t (* get_firmware_version)( TsControllerRef_t, char * );

} TsControllerVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The one and only "plug-in", used by the SDK to reference the controller aspect.
 * The definition of this global is performed in sdk_components/ts_components.c and
 * is based on compile-time configuration.
 */
extern const TsControllerVtable_t * ts_controller;

/**
 * By convention, the following #defines are the only means used by the SDK to reference
 * method calls on the controller component (e.g., ts_controller->create, etc. is never used).
 */

#define ts_controller_create        ts_controller->create
#define ts_controller_destroy       ts_controller->destroy
#define ts_controller_tick          ts_controller->tick

#define ts_controller_connect       ts_controller->connect
#define ts_controller_disconnect    ts_controller->disconnect
#define ts_controller_read          ts_controller->read
#define ts_controller_write         ts_controller->write

/**
 * The following #defines are deprecated, the _profile will be used in future for diagnostics.
 */
#define ts_controller_get_id        ts_controller->get_id
#define ts_controller_get_rssi      ts_controller->get_rssi
#define ts_controller_get_ipv4_addr ts_controller->get_ipv4_addr
#define ts_controller_get_iccid     ts_controller->get_iccid
#define ts_controller_get_date_and_time ts_controller->get_date_and_time
#define ts_controller_get_imsi      ts_controller->get_imsi
#define ts_controller_get_manufacturer ts_controller->get_manufacturer
#define ts_controller_get_module_name ts_controller->get_module_name
#define ts_controller_get_firmware_version ts_controller->get_firmware_version

#ifdef __cplusplus
}
#endif

#endif // TS_CONTROLLER_H
