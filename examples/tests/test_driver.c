// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>
#include <termios.h>

#include "ts_platforms.h"
#include "ts_components.h"

static struct termios oldt;

static void restore_terminal_settings(void) {

	tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

static void disable_waiting_for_enter(void) {

	struct termios newt;

	/* Make terminal read 1 char at a time */
	tcgetattr(0, &oldt);  /* Save terminal settings */
	newt = oldt;  /* Init new settings */
	newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
	tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
	atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}

int main() {

	TsStatus_t status;
	ts_status_set_level( TsStatusLevelDebug );

	// force driver implementation to serial
	ts_driver = &(ts_driver_unix_serial);

	TsDriverRef_t driver;
	status = ts_driver_create( &driver );
	if( status != TsStatusOk ) {
		ts_status_debug( "** create failed, %s\n", ts_status_string(status) );
		return 0;
	}

	status = ts_driver_connect( driver, "tty.usbserial-DM00B1YLgit" );
	if( status != TsStatusOk ) {
		ts_status_debug( "** connect failed, %s\n", ts_status_string(status) );
		return 0;
	}

	int ch;
	int index = 0;
	uint32_t budget = 5 * TS_TIME_SEC_TO_USEC;
	uint8_t wbuffer[ 2048 ];
	uint8_t rbuffer[ 2048 ];
	size_t buffer_size;
	bool echoing = true;
	do {

		ch = getchar();
		while(( ch != EOF) && ( index < 2048 )) {

			wbuffer[ index ] = (uint8_t) ch;
			index = index + 1;
			ts_status_debug( "** write, %02x\n", ch );
			ch = getchar();
		}
		if( index > 0 ) {

			buffer_size = (size_t) index;
			status = ts_driver_write( driver, wbuffer, &buffer_size, budget );
			if( status > TsStatusOk ) {
				ts_status_debug( "** write failed, %s\n", ts_status_string( status ));
				echoing = false;
			}
			if( buffer_size != (size_t) index ) {
				ts_status_debug( "** write failed to complete\n" );
				echoing = false;
			}
			index = 0;
		}
		if( echoing ) {

			buffer_size = 2048;
			status = ts_driver_read( driver, rbuffer, &buffer_size, budget );
			if( status > TsStatusOk ) {
				ts_status_debug( "** read failed, %s\n", ts_status_string( status ));
				echoing = false;
			}
			ts_status_debug( "%.*s", buffer_size, rbuffer );
		}
	} while( echoing );

	ts_driver_disconnect( driver );
	ts_driver_destroy( driver );
	return 0;
}
