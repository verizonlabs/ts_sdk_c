/**
 * @file
 * ts_connection.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A TCP IP connection.
 *
 * @details
 * The "connection" encapsulates three components that make up the behavior of a simple TCP connection.
 * That is, A connection initializes, and forwards calls through a componentized pipeline,
 * i.e., this --> ts_security --> ts_controller --> ts_driver.
 *
 * @code
 *
 * 	TsConnectionRef_t connection;
 * 	ts_connection_create( &connection );
 * 	ts_connection_connect( connection, "verizon.com:443" );
 * 	ts_connection_write( connection, buffer, &buffer_size );
 * 	while( reading ) {
 * 		ts_connection_read( connection, buffer, &buffer_size );
 * 		ts_status_debug( "%.*s\n", buffer_size, buffer );
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
#ifndef TS_CONNECTION_H
#define TS_CONNECTION_H

#include <stdint.h>
#include <stdio.h>

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_address.h"
#include "ts_security.h"
#include "ts_controller.h"

/**
 * The connection object reference.
 */
typedef struct TsConnection * TsConnectionRef_t;

/**
 * The connection object, which holds the ts_security state, _security and any profile
 * information (such as diagnostics), _profile.
 */
typedef struct TsConnection {

	TsSecurityRef_t _security;
	TsProfileRef_t _profile;

} TsConnection_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate and initialize a new connection. This function will also
 * create a ts_security object.
 *
 * @param connection
 * [in/out] The pointer to a pre-existing TsConnectionRef_t, which will be initialized by this
 * function with the connection state.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_create( TsConnectionRef_t * connection );

/**
 * Deallocate connection resources of the given connection. This fuction will
 * deallocate the referenced ts_security object.
 *
 * @param connection
 * [in] The connection object
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_destroy( TsConnectionRef_t connection );

/**
 * Provide the given connection some processing time according to the given budget "recommendation".
 *
 * @param connection
 * [in] The connection object
 *
 * @param budget
 * [in] The recommended time in microseconds budgeted for the function
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_tick( TsConnectionRef_t connection, uint32_t budget );

/**
 * Return the held ts_security state object.
 *
 * @param connection
 * [in] The connection object
 *
 * @param security
 * [in/out] A pointer to a reference variable that should be filled with this object's security object
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_get_security( TsConnectionRef_t connection, TsSecurityRef_t * security );

/**
 * Return the profile of the connection, this function will automatically request additional
 * profile information from the contained ts_security object, _security.
 *
 * @param connection
 * [in] The connection object
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_get_profile( TsConnectionRef_t connection, TsProfileRef_t * profile );

/**
 * Return the recommended maximum message size. This value usually comes from the
 * ts_driver object held by the ts_controller (which, in turn, is held by this ts_security
 * object, _security), and usually reflects a hardware (e.g., module memory) or
 * TCP/IP stack limitation on the message size.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_get_spec_mtu( TsConnectionRef_t connection, uint32_t* mtu );

/**
 * Return the hardware identifier (e.g., MAC or IMEI) of the network interface used by this connection.
 *
 * @param connection
 * [in] The connection object
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
TsStatus_t ts_connection_get_spec_id( TsConnectionRef_t connection, const uint8_t * id, size_t id_size );

/**
 * [deprecated] Return the hardware recommended tick budget.
 *
 * @param connection
 * [in] The connection object
 *
 * @param budget
 * [in/out] The pointer to the target uint32 being filled with the budget value.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_get_spec_budget( TsConnectionRef_t connection, uint32_t* budget );

/**
 * Set the hostname expected on the returned server certificate during the TLS handshake.
 *
 * @param connection
 * [in] The connection object
 *
 * @param hostname
 * [in] The host FQDN expected (and verified) by the TLS handshake
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_set_server_cert_hostname( TsConnectionRef_t connection, const char * hostname );

/**
 * Set the server certificate for x.509 verification during the TLS handshake
 *
 * @param connection
 * [in] The connection object
 *
 * @param cert
 * The server certificate (DER).
 *
 * @param cert_size
 * The server certificate size.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_set_server_cert( TsConnectionRef_t connection, const uint8_t * cert, size_t cert_size );

/**
 * Set the client credential certificate used for verification during the TLS handshake
 *
 * @param connection
 * [in] The connection object
 *
 * @param cert
 * The client certificate (DER).
 *
 * @param cert_size
 * The client certificate size.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_set_client_cert( TsConnectionRef_t connection, const uint8_t * cert, size_t cert_size );

/**
 * Set the client credential key used for verification during the TLS handshake
 *
 * @param connection
 * [in] The connection object
 *
 * @param cert
 * The client key
 *
 * @param cert_size
 * The client key size
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_set_client_key( TsConnectionRef_t connection, const uint8_t * key, size_t key_size );

/**
 * Create a TCP/IP connection to the given address. Note that this will use the underlying ts_security
 * component to establish a encrypted connection if necessary.
 *
 * @param connection
 * [in] The connection object
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
TsStatus_t ts_connection_connect( TsConnectionRef_t connection, TsAddress_t address );

/**
 * Destroy (i.e., tear down) the current TCP/IP connection. Note that this will used the underlying ts_security
 * component to tear down the connection.
 *
 * @param connection
 * [in] The connection object
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_connection_disconnect( TsConnectionRef_t connection );

/**
 * Read from the tcp/ip driver (non-blocking) via the security and controller components.
 *
 * @note
 * TsStatusOkReadPending has a very specific meaning, only return when the read
 * has returned pending and there isn't data in the buffer, in all other cases
 * return a valid status with the contents of the current buffer.
 *
 * @param connection
 * [in] The connection state
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
TsStatus_t ts_connection_read( TsConnectionRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget );

/**
 * Write to the driver (blocking) via the security and controller components.
 *
 * @param connection
 * [in] The connection state
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
TsStatus_t ts_connection_write( TsConnectionRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget );

#ifdef __cplusplus
}
#endif

#endif // TS_CONNECTION_H
