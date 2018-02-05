// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#if defined(__unix__) || defined(__unix) || ( defined(__APPLE__) && defined(__MACH__))
#if defined(__APPLE__) && defined(__MACH__)
#include <sys/ioctl.h>
#include <IOKit/serial/ioss.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "ts_platform.h"
#include "ts_driver.h"

static TsStatus_t ts_create( TsDriverRef_t * );
static TsStatus_t ts_destroy( TsDriverRef_t );
static TsStatus_t ts_tick( TsDriverRef_t, uint32_t );

static TsStatus_t ts_connect( TsDriverRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsDriverRef_t );
static TsStatus_t ts_read( TsDriverRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_reader(TsDriverRef_t, void*, TsDriverReader_t);
static TsStatus_t ts_write( TsDriverRef_t, const uint8_t *, size_t *, uint32_t );

TsDriverVtable_t ts_driver_unix_serial = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.reader = ts_reader,
	.write = ts_write,
};

typedef struct TsDriverSerial * TsDriverSerialRef_t;
typedef struct TsDriverSerial {

	// inheritance by encapsulation; must be the first
	// attribute in order to treat this struct as a
	// TsDriver struct
	TsDriver_t _driver;

	int _fd;
	struct termios _oldtty;
	struct termios _newtty;
	uint64_t _last_read_timestamp;

} TsDriverSerial_t;

static TsStatus_t ts_create( TsDriverRef_t * driver ) {

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( ts_platform_malloc( sizeof( TsDriverSerial_t )));
	serial->_driver._address = "";
	serial->_driver._profile = NULL;
	serial->_driver._reader = NULL;
	serial->_driver._reader_state = NULL;
	serial->_driver._spec_budget = 60 * TS_TIME_SEC_TO_USEC;
	serial->_driver._spec_mcu = 2048;
	// TODO - should provide mac address here? probably not
	// TODO - currently using my own mac-id - need to change this asap.
	snprintf( (char *)(serial->_driver._spec_id), TS_DRIVER_MAX_ID_SIZE, "%s", "B827EBA15910" );
	serial->_fd = -1;
	serial->_last_read_timestamp = 0;

	*driver = (TsDriverRef_t) serial;
	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsDriverRef_t driver ) {

	ts_status_trace( "ts_driver_destroy\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );
	ts_platform->free( serial, sizeof( TsDriverSerial_t ));

	return TsStatusOk;
}

static TsStatus_t ts_tick( TsDriverRef_t driver, uint32_t budget ) {

	ts_status_trace( "ts_driver_tick\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );
	if( serial->_driver._reader != NULL ) {

		uint8_t * buffer = (uint8_t*)ts_platform_malloc( serial->_driver._spec_mcu );
		size_t buffer_size = serial->_driver._spec_mcu;
		TsStatus_t status = ts_driver_read( driver, buffer, &buffer_size, budget );
		switch( status ) {
		case TsStatusOkReadPending:
			// do nothing
			break;

		case TsStatusOk:
			// callback
			if( buffer_size > 0 ) {
				serial->_driver._reader( driver, serial->_driver._reader_state, buffer, buffer_size );
			}
			break;

		default:
			ts_status_alarm( "ts_driver_tick: reader failed, %s\n", ts_status_string( status ));
			// do nothing, i.e., return ok
			break;
		}
		ts_platform_free( buffer, buffer_size );
	}
	return TsStatusOk;
}

static TsStatus_t ts_connect( TsDriverRef_t driver, TsAddress_t address )  {

	ts_status_trace( "ts_driver_connect\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );

	serial->_fd = open( address, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if (serial->_fd < 0) {
		ts_status_alarm("ts_driver_connect: error opening %s: %s (%d)\n", address, strerror(errno), errno);
		return TsStatusErrorBadRequest;
	}
	// Now that the device is open, clear the O_NONBLOCK flag so
	// subsequent I/O will block.
	if ( fcntl(serial->_fd , F_SETFL, 0) < 0 ) {
		ts_status_alarm("ts_driver_connect: error clearing non-block: %s\n", strerror(errno));
		return TsStatusErrorInternalServerError;
	}
	if (tcgetattr(serial->_fd, &(serial->_oldtty)) < 0) {
		ts_status_alarm("ts_driver_connect: error from tcgetattr: %s\n", strerror(errno));
		return TsStatusErrorInternalServerError;
	}
	struct termios tty = serial->_oldtty;
#if defined(__APPLE__) && defined(__MACH__)
	// The IOSSIOSPEED ioctl can be used to set arbitrary baud rates
	// other than those specified by POSIX. The driver for the underlying serial hardware
	// ultimately determines which baud rates can be used. This ioctl sets both the input
	// and output speed.
	cfsetspeed( &tty, B230400 );
	speed_t speed = 921600;
	if (ioctl(serial->_fd, IOSSIOSPEED, &speed) == -1) {
		ts_status_alarm("ts_driver_connect: error calling ioctl, %s (%d)\n", strerror(errno), errno);
		return TsStatusErrorInternalServerError;
	}
#else
	int speed = B921600;
	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);
	ts_status_debug("ts_driver_connect: input baud rate changed to %d\n", (int) cfgetispeed(&tty));
	ts_status_debug("ts_driver_connect: output baud rate changed to %d\n", (int) cfgetospeed(&tty));
#endif

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 1;

	serial->_newtty = tty;

	return TsStatusOk;
}

static TsStatus_t ts_disconnect( TsDriverRef_t driver ) {

	ts_status_trace( "ts_driver_connect\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );
	close( serial->_fd );

	return TsStatusOk;
}

static TsStatus_t ts_read( TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_driver_read\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );

	// initialize timestamp for read timer budgeting
	uint64_t timestamp = ts_platform_time();

	// limit read to 1MHz call bandwidth
	// note that this doesnt limit the number of recv calls made below,
	// just the number of reattempts by the caller,...
	if( timestamp - serial->_last_read_timestamp == 0 ) {
		*buffer_size = 0;
		return TsStatusOkReadPending;
	}
	serial->_last_read_timestamp = timestamp;

	// perform read
	int flags = 0x00;
	bool reading = true;
	ssize_t index = 0;
	TsStatus_t status = TsStatusOk;
	do {

		// read from the socket
		if (tcsetattr(serial->_fd, TCSANOW, &(serial->_newtty)) != 0) {
			ts_status_alarm("ts_driver_read: error from tcsetattr: %s (%d)\n", strerror(errno), errno );
		}
		ssize_t size = read( serial->_fd, (void*)(buffer + index), (*buffer_size) - index );
		if (tcsetattr(serial->_fd, TCSANOW, &(serial->_oldtty)) != 0) {
			ts_status_alarm("ts_driver_read: error from tcsetattr: %s (%d)\n", strerror(errno), errno );
		}
		if( size < 0 ) {

			// recv has indicated either non-block status
			// or an actual driver issue,...
			size = 0;
			reading = false;

			ts_status_debug( "ts_driver_read: ignoring error, %s (%d)\n", strerror(errno), errno );
			status = TsStatusErrorInternalServerError;

		} else if( ts_platform_time() - timestamp > budget ) {

			// give back control to caller
			reading = false;

			if( index > 0 ) {
				status = TsStatusOk;
			} else {
				status = TsStatusOkReadPending;
			}

		} else if( size > 0 ) {

			// this reset makes the budget into a 'time per character'
			// and so, the budget will always be exceeded before returning
			// i.e., there may be a better way to do this,...
			timestamp = ts_platform_time();
		}

		index = index + size;

		if( index >= *buffer_size ) {
			reading = false;
			status = TsStatusOk;
		}

	} while( reading );

	// update read buffer size and return
	*buffer_size = index;
	return status;
}

static TsStatus_t ts_reader(TsDriverRef_t driver, void* state, TsDriverReader_t reader ) {

	ts_status_trace( "ts_driver_reader\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );
	serial->_driver._reader = reader;
	serial->_driver._reader_state = state;

	return TsStatusOk;
}

static TsStatus_t ts_write( TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_driver_write\n" );
	ts_platform_assert( driver != NULL );

	TsDriverSerialRef_t serial = (TsDriverSerialRef_t) ( driver );

	// initialize timestamp for write timer budgeting
	uint64_t timestamp = ts_platform_time();

	// perform write
	int flags = 0x00;
	bool writing = true;
	ssize_t index = 0;
	TsStatus_t status = TsStatusOk;
	do {

		// write to the socket
		if (tcsetattr(serial->_fd, TCSANOW, &(serial->_newtty)) != 0) {
			ts_status_alarm("ts_driver_write: error from tcsetattr: %s (%d)\n", strerror(errno), errno );
		}
		ssize_t size = write( serial->_fd, buffer + index, *buffer_size - index );
		//tcdrain( serial->_fd );
		if (tcsetattr(serial->_fd, TCSANOW, &(serial->_oldtty)) != 0) {
			ts_status_alarm("ts_driver_write: error from tcsetattr: %s (%d)\n", strerror(errno), errno );
		}
		if( size < 0 ) {

			// send has indicated either non-block status
			// or an actual driver issue,...
			size = 0;
			writing = false;

			ts_status_debug( "ts_driver_write: ignoring error, %s (%d)\n", strerror(errno), errno );
			status = TsStatusErrorInternalServerError;

		} else if( size == 0 ) {

			// unexpected non-blocking call returns zero bytes
			ts_status_info( "ts_driver_write: unexpected empty write occured\n" );
			writing = false;
			if( index > 0 ) {
				status = TsStatusOk;
			} else {
				status = TsStatusOkWritePending;
			}

		} else if( ts_platform_time() - timestamp > budget ) {

			// there is more to write, but dont give control back to the caller
			ts_status_debug( "ts_driver_write: ignoring timer budget exceeded\n");
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
