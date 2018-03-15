/**
 * @file
 * ts_driver.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A hardware or software driver to an underlying TCP/IP stack (e.g., typically UART access to a cellular modem).
 *
 * @details
 * The "driver" is usually called from the ts_controller component, and encapsulates the hardware or software
 * access to a TCP/IP stack implementation. The developer usually doesnt use this component directly, but WILL
 * implement their own driver for their hardware platform (see examples/platforms for examples).
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) runtime. They define the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */
#ifndef TS_DRIVER_H
#define TS_DRIVER_H

#include "ts_status.h"
#include "ts_profile.h"

/**
 * Maximum size allowed for the hardware id, i.e., MAC address or IMEI.
 */
#define TS_DRIVER_MAX_ID_SIZE 36

/**
 * The driver object reference
 */
typedef struct TsDriver *TsDriverRef_t;

/**
 * The "reader" callback, used by interrupts to pass buffered data received by the
 * hardware (e.g., modem) back to the controller (which, in turn, buffers it for the
 * of the next read call made by the ts_security component).
 */
typedef TsStatus_t (*TsDriverReader_t)(TsDriverRef_t, void*, const uint8_t *, size_t);

/**
 * The driver object.
 */
typedef struct TsDriver {

	TsProfileRef_t  _profile;
	TsAddress_t		_address;
	TsDriverReader_t _reader;
	void *          _reader_state;

	// advertised driver specifications
	uint32_t        _spec_budget;   // read/write timer budget in microseconds
	// TODO - these values may not belong here - need to resolve all spec parameters at connection level?
	uint32_t        _spec_mtu;      // maximum buffer size in bytes
	uint8_t         _spec_id[TS_DRIVER_MAX_ID_SIZE]; // zero terminated device id

} TsDriver_t;

/**
 * The driver vector table (i.e., the driver "type"), used to configure the SDK driver aspect.
 * See examples/platforms for available drivers, or use your own customized implementation.
 */
typedef struct TsDriverVtable {

	/**
	 * Allocate and initialize a new driver. This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [on/out] The pointer to a pre-existing TsDriverRef_t, which will be initialized with the driver state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*create)(TsDriverRef_t * driver);

	/**
	 * Deallocate the given driver. This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*destroy)(TsDriverRef_t driver);

	/**
	 * Provide the given driver processing time according to the given budget "recommendation".
	 * This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
	 *
	 * @param budget
	 * [in] The recommended time in microseconds budgeted for the function
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*tick)(TsDriverRef_t driver, uint32_t budget);

	/**
	 * Create a TCP/IP connection to the given address.
	 * This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
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
	TsStatus_t (*connect)(TsDriverRef_t driver, TsAddress_t address);

	/**
	 * Destroy (i.e., tear down) the current TCP/IP connection.
	 * This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*disconnect)(TsDriverRef_t driver);

	/**
	 * Read from the TCP/IP stack connection (non-blocking).
	 * This function is typically called from ts_controller.
	 *
	 * @note
	 * TsStatusOkReadPending has a very specific meaning, only return when the read
	 * has returned pending and there isn't data in the buffer, in all other cases
	 * return a valid status with the contents of the current buffer.
	 *
	 * @param driver
	 * [in] The driver state.
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
	TsStatus_t (*read)(TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget);

	/**
	 * Set the (optional) callback which should occur when data has been read from the underlying
	 * hardware. This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
	 *
	 * @param callback_data
	 * [in] A predefined pointer that is passed to the callback when called.
	 *
	 * @param callback
	 * [in] The callback function receiving the newly read data.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*reader)(TsDriverRef_t driver, void* callback_data, TsDriverReader_t callback);

	/**
	 * Write to the driver (blocking) to TCP/IP connection (blocking).
	 * This function is typically called from ts_controller.
	 *
	 * @param driver
	 * [in] The driver state.
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
	TsStatus_t (*write)(TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget);

	void       (*reset)(TsDriverRef_t driver);

} TsDriverVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsDriverVtable_t * ts_driver;

#define ts_driver_create    ts_driver->create
#define ts_driver_destroy   ts_driver->destroy
#define ts_driver_tick		ts_driver->tick

#define ts_driver_connect	ts_driver->connect
#define ts_driver_disconnect ts_driver->disconnect
#define ts_driver_read		ts_driver->read
#define ts_driver_reader    ts_driver->reader
#define ts_driver_write		ts_driver->write
#define ts_driver_reset		ts_driver->reset

#ifdef __cplusplus
}
#endif

#endif // TS_DRIVER_H
