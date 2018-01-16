// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#if defined(__unix__) || defined(__unix) || ( defined(__APPLE__) && defined(__MACH__))

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>

#include "ts_platform.h"
#include "ts_controller.h"

static TsStatus_t ts_create( TsControllerRef_t * );
static TsStatus_t ts_destroy( TsControllerRef_t );
static TsStatus_t ts_tick( TsControllerRef_t, uint32_t );
static TsStatus_t ts_report( TsControllerRef_t );

static TsStatus_t ts_connect( TsControllerRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsControllerRef_t );
static TsStatus_t ts_read( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_write( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );

TsControllerVtable_t ts_controller_socket = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

typedef struct TsControllerSocket * TsControllerSocketRef_t;
typedef struct TsControllerSocket {

	// inheritance by encapsulation; must be the first
	// attribute in order to treat this struct as a
	// TsController struct
	TsController_t _controller;

	int _fd;
	uint64_t _last_read_timestamp;

} TsControllerSocket_t;

static TsStatus_t ts_create( TsControllerRef_t * controller ) {

	ts_status_trace( "ts_controller_create: socket\n" );
	ts_platform_assert( controller != NULL );

	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( ts_platform_malloc( sizeof( TsControllerSocket_t )));
	sock->_controller._address = "";
	sock->_controller._driver = NULL;
	sock->_controller._profile = NULL;
	sock->_controller._spec_budget = 60 * TS_TIME_SEC_TO_USEC;
	sock->_controller._spec_mcu = 2048;
	// TODO - should provide mac address here
	// TODO - currently using my own mac-id - need to change this asap.
	snprintf( (char *)(sock->_controller._spec_id), TS_CONTROLLER_MAX_ID_SIZE, "%s", "B827EBA15910" );
	sock->_fd = -1;

	*controller = (TsControllerRef_t) sock;

	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsControllerRef_t controller ) {
	ts_status_trace( "ts_controller_destroy\n" );
	ts_platform_assert( controller != NULL );

	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( controller );
	ts_platform->free( sock, sizeof( TsControllerSocket_t ));

	return TsStatusOk;
}

static TsStatus_t ts_tick( TsControllerRef_t controller, uint32_t budget ) {

	ts_status_trace( "ts_controller_tick\n" );
	ts_platform_assert( controller != NULL );

	// do nothing

	return TsStatusOk;
}

static TsStatus_t ts_connect( TsControllerRef_t controller, TsAddress_t address ) {

	ts_status_trace( "ts_controller_connect\n" );
	ts_platform_assert( controller != NULL );

	// TODO - init sockets?
	// signal( SIGPIPE, SIG_IGN );

	// init address hints
	struct addrinfo hints;
	memset( &hints, 0x00, sizeof( struct addrinfo ));
	hints.ai_family = AF_UNSPEC;
	// TODO - allow UDP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// decode and resolve address
	char host[TS_ADDRESS_MAX_HOST_SIZE];
	char port[TS_ADDRESS_MAX_PORT_SIZE];
	if( ts_address_parse( address, host, port ) != TsStatusOk ) {
		return TsStatusErrorInternalServerError;
	}
	struct addrinfo * address_list;
	if( getaddrinfo( host, port, &hints, &address_list ) != 0 ) {
		return TsStatusErrorNotFound;
	}

	// find active listener
	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( controller );
	TsStatus_t status = TsStatusOk;
	struct addrinfo * current;
	for( current = address_list; current != NULL; current = current->ai_next ) {

		sock->_fd = (int) socket( current->ai_family, current->ai_socktype, current->ai_protocol );
		if( sock->_fd < 0 ) {
			status = TsStatusErrorNotFound;
			continue;
		}
		if( connect( sock->_fd, current->ai_addr, current->ai_addrlen ) == 0 ) {

			if( fcntl( sock->_fd, F_SETFL, fcntl( sock->_fd, F_GETFL, 0 ) | O_NONBLOCK ) == -1 ) {
				status = TsStatusErrorInternalServerError;
				close( sock->_fd );
				continue;
			}
			status = TsStatusOk;
			break;
		}
		status = TsStatusErrorBadGateway;
		ts_disconnect( controller );
	}
	freeaddrinfo( address_list );
	return status;
}

static TsStatus_t ts_disconnect( TsControllerRef_t controller ) {

	ts_status_trace( "ts_controller_disconnect\n" );
	ts_platform_assert( controller != NULL );

	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( controller );
	close( sock->_fd );

	return TsStatusOk;
}

/**
 * Read from the socket controller (non-blocking). Note that we dont use select() in order to emulate
 * the other channels better, e.g., uart and usb.
 *
 * @note
 * TsStatusReadPending has a very specific meaning, only return when the read
 * has returned pending and there isn't data in the buffer, in all other cases
 * return a valid status with the contents of the current buffer.
 *
 * @param controller
 * [in] The socket state
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
 * TsStatusOk, *buffer_size > 0 - Return the given amount of read data
 * TsStatusOk, *buffer_size = 0 - Usually indicates and end-of-file condition
 * TsStatusOkPendingRead        - Indicates a blocking condition exists (and avoided),
 *                                note, *buffer_size is guaranteed to be zero when this condition occurs.
 *                                TODO - currently, we dont wait for the budget to be exhausted, this may change later
 * TsStatusErrorConnectionReset - Indicates that the controller was broken
 * TsStatusError*               - Indicates an error has occurred, see ts_status.h for more information.
 */
static TsStatus_t ts_read( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_controller_read\n" );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( controller );

	// initialize timestamp for read timer budgeting
	uint64_t timestamp = ts_platform_time();

	// limit read to 1MHz call bandwidth
	// note that this doesnt limit the number of recv calls made below,
	// just the number of reattempts by the caller,...
	if( timestamp - sock->_last_read_timestamp == 0 ) {
		*buffer_size = 0;
		return TsStatusReadPending;
	}
	sock->_last_read_timestamp = timestamp;

	// perform read
	int flags = 0x00;
	bool reading = true;
	ssize_t index = 0;
	TsStatus_t status = TsStatusOk;
	do {

		// read from the socket
		ssize_t size = recv( sock->_fd, (void *) ( buffer + index ), ( *buffer_size ) - index, flags );
		if( size < 0 ) {

			// recv has indicated either non-block status
			// or an actual controller issue,...
			size = 0;
			reading = false;

			// establish the exit scenario for the caller
			if( errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR ) {
				if( index > 0 ) {
					status = TsStatusOk;
				} else {
					// TODO - allow the timer budget to be exhausted before returning?
					status = TsStatusReadPending;
				}
			} else if( errno == EPIPE || errno == ECONNRESET ) {
				status = TsStatusErrorConnectionReset;
			} else if( errno != 0 ) {
				ts_status_debug( "ts_controller_read: ignoring error, %d\n", errno);
				status = TsStatusErrorInternalServerError;
			}

		} else if( size == 0 ) {

			// first normal exit condition, non-block io read returns zero bytes
			reading = false;
			status = TsStatusOk;

		} else if( ts_platform_time() - timestamp > budget ) {

			// there is more to read than expected on this attempt,
			// give back control to caller
			ts_status_debug( "ts_controller_read: timer budget exceeded\n", errno);
			reading = false;

			if( index > 0 ) {
				status = TsStatusOk;
			} else {
				status = TsStatusReadPending;
			}
		}

		index = index + size;

		if( index >= *buffer_size ) {
			// second normal exit condition, the buffer is full
			reading = false;
			status = TsStatusOk;
		}

	} while( reading );

	// update read buffer size and return
	*buffer_size = (size_t) index;
	return status;
}

static TsStatus_t ts_write( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_controller_write\n" );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerSocketRef_t sock = (TsControllerSocketRef_t) ( controller );

	// initialize timestamp for write timer budgeting
	uint64_t timestamp = ts_platform_time();

	// perform write
	int flags = 0x00;
	bool writing = true;
	ssize_t index = 0;
	TsStatus_t status = TsStatusOk;
	do {

		// write to the socket
		ssize_t size = send( sock->_fd, buffer + index, *buffer_size - index, flags );
		if( size < 0 ) {

			// send has indicated either non-block status
			// or an actual controller issue,...
			size = 0;
			writing = false;

			// establish exit scenarip for the caller
			if( errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR ) {
				if( index > 0 ) {
					status = TsStatusOk;
				} else {
					status = TsStatusWritePending;
				}
			} else if( errno == EPIPE || errno == ECONNRESET ) {
				status = TsStatusErrorConnectionReset;
			} else if( errno != 0 ) {
				ts_status_debug( "ts_controller_write: ignoring error, %d\n", errno);
				status = TsStatusErrorInternalServerError;
			}

		} else if( size == 0 ) {

			// unexpected non-blocking call returns zero bytes
			ts_status_info( "ts_controller_write: unexpected empty write occured\n" );
			writing = false;
			if( index > 0 ) {
				status = TsStatusOk;
			} else {
				status = TsStatusWritePending;
			}

		} else if( ts_platform_time() - timestamp > budget ) {

			// there is more to write, but dont give control back to the caller
			ts_status_debug( "ts_controller_write: ignoring timer budget exceeded\n", errno);
		}

		index = index + size;

		if( index >= *buffer_size ) {

			// second normal exit condition, the buffer is full
			writing = false;
			status = TsStatusOk;
		}

	} while( writing );

	// update write buffer size and return
	*buffer_size = (size_t) index;
	return status;
}
#endif // __unix__
