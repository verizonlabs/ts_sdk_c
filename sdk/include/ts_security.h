/**
 * @file
 * ts_security.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The security aspect of a connection as a component.
 *
 * @details
 * The encryption and credentialing implementation used by the connection when connecting
 * or communicating with a server. This is currently implemented by mbedTLS, but a Mocana version
 * may be licenced, or a custom version may be used.
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) run-time. They provide the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */
#ifndef TS_SECURITY_H
#define TS_SECURITY_H

#include <stdint.h>
#include <stdio.h>

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_address.h"
#include "ts_controller.h"

// the server identity (hostname) that must be present in the server
// certificate CN or SubjectAltName.
// TODO - move this out of a compiler define, double check actual setting
#ifndef SSL_HOST
#define SSL_HOST "simpm.thingspace.verizon.com"
#endif
#ifndef SSL_HANDSHAKE_TIMEOUT
#define SSL_HANDSHAKE_TIMEOUT (30 * TS_TIME_SEC_TO_USEC)
#endif
#ifndef SSL_READ_BUDGET
#define SSL_READ_BUDGET (50 * TS_TIME_SEC_TO_USEC)
#endif
#ifndef SSL_WRITE_BUDGET
#define SSL_WRITE_BUDGET (50 * TS_TIME_SEC_TO_USEC)
#endif

/**
 * The security object reference
 */
typedef struct TsSecurity *TsSecurityRef_t;

/**
 * The security object
 */
typedef struct TsSecurity {

	TsControllerRef_t   _controller;
	TsProfileRef_t      _profile;

} TsSecurity_t;

/**
 * The security vector table (i.e., the security "class" definition), used to define the security SDK-aspect.
 * See sdk_components/security for available security implementations, or use your own customized implementation.
 */
typedef struct TsSecurityVtable {

	/**
	 * Perform one-time static initialization of the security subsystem, if any.
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*initialize)();

	/**
	 * Allocate and initialize a new security object. This function is typically called from ts_connection.
	 *
	 * @param security
	 * [on/out] The pointer to a pre-existing TsSecurityRef_t, which will be initialized with the security state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*create)(TsSecurityRef_t *);

	/**
	 * Deallocate the given security object.
	 *
	 * @param security
	 * [in] The security state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*destroy)(TsSecurityRef_t);

	/**
	 * Provide the given security object processing time according to the given budget "recommendation".
	 * This function is typically called from ts_connection.
	 *
	 * @param security
	 * [in] The security state.
	 *
	 * @param budget
	 * [in] The recommended time in microseconds budgeted for the function
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*tick)(TsSecurityRef_t, uint32_t);

	TsStatus_t (*set_server_cert_hostname)(TsSecurityRef_t, const char *);
	TsStatus_t (*set_server_cert)(TsSecurityRef_t, const uint8_t *, size_t);
	TsStatus_t (*set_client_cert)(TsSecurityRef_t, const uint8_t *, size_t);
	TsStatus_t (*set_client_key)(TsSecurityRef_t, const uint8_t *, size_t);

	/**
	 * Return the recommended maximum message size. This value usually comes from the
	 * ts_driver object held by the ts_controller (which, in turn, is held by this ts_security
	 * object, _security), and usually reflects a hardware (e.g., module memory) or
	 * TCP/IP stack limitation on the message size.
	 *
	 * @param security
	 * [in] The security state.
	 *
	 * @param mtu
	 * [in/out] The pointer to the target int being filled with the MTU value.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*get_spec_mtu)( TsSecurityRef_t connection, uint32_t* mtu );

	/**
	 * Return the hardware identifier (e.g., MAC or IMEI) of the network interface used by this connection.
	 *
	 * @param security
	 * [in] The security state.
	 *
	 * @param id
	 * [in/out] The pointer to the target string being filled with the ID value.
	 *
	 * @param id_size
	 * [in] The maxium size of the the target string, id.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*get_spec_id)( TsSecurityRef_t connection, const uint8_t * id, size_t id_size );

	/**
	 * [deprecated] Return the hardware recommended tick budget.
	 *
	 * @param security
	 * [in] The security state.
	 *
	 * @param budget
	 * [in/out] The pointer to the target uint32 being filled with the budget value.
 	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*get_spec_budget)( TsSecurityRef_t connection, uint32_t* budget );

	/**
	 * Negotiate a TLS connection to the given server adddress. Note that this will use the underlying
	 * ts_controller component to manage the TCP/IP stack on the configured platform.
	 *
	 * @param security
	 * [in] The security object
	 *
	 * @param address
	 * [in] The destination address. A FQDN may require resolution prior to calling this function based
	 * on the underlying ts_controller or ts_driver capabilities, e.g., Mocana TLS expects an IP.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*connect)(TsSecurityRef_t, TsAddress_t);

	/**
	 * Destroy (i.e., tear down) the current TCP/IP connection. Note that this will used the underlying
	 * ts_controller component to tear down the connection.
	 *
	 * @param security
	 * [in] The security object
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*disconnect)(TsSecurityRef_t);

	/**
	 * Read and decrypt from the tcp/ip driver (non-blocking) via the controller component.
	 *
	 * @note
	 * TsStatusOkReadPending has a very specific meaning, only return when the read
	 * has returned pending and there isn't data in the buffer, in all other cases
	 * return a valid status with the contents of the current buffer.
	 *
	 * @param security
	 * [in] The security state
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
	TsStatus_t (*read)(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);

	/**
	 * Encrypt and write to the driver (blocking) via the controller components.
	 *
	 * @param security
	 * [in] The security state
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
	TsStatus_t (*write)(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);

} TsSecurityVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

// security depends on ts_controller
extern const TsSecurityVtable_t * ts_security;

#define ts_security_initialize		ts_security->initialize

#define ts_security_create			ts_security->create
#define ts_security_destroy			ts_security->destroy
#define ts_security_tick			ts_security->tick

#define ts_security_set_server_cert_hostname ts_security->set_server_cert_hostname
#define ts_security_set_server_cert	ts_security->set_server_cert
#define ts_security_set_client_cert	ts_security->set_client_cert
#define ts_security_set_client_key	ts_security->set_client_key

#define ts_security_get_spec_mtu    ts_security->get_spec_mtu
#define ts_security_get_spec_id     ts_security->get_spec_id
#define ts_security_get_spec_budget ts_security->get_spec_budget

#define ts_security_connect			ts_security->connect
#define ts_security_disconnect		ts_security->disconnect
#define ts_security_read			ts_security->read
#define ts_security_write			ts_security->write

#ifdef __cplusplus
}
#endif
#endif // TS_SECURITY_H
