// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#include "ts_message.h"

int main() {

	TsStatus_t status;
	ts_status_set_level( TsStatusLevelDebug );

	ts_status_debug("** create example message\n");
	TsMessageRef_t message, sensor;
	ts_message_create(&message);
	ts_message_set_string( message, "id", "00000000-0000-0000-0000-000000000000" );
	ts_message_set_string( message, "kind", "ts.event" );
	ts_message_set_string( message, "action", "update" );
	ts_message_create_message( message, "fields", &sensor );
	ts_message_set_float( sensor, "temperature", 30.2 );
	ts_message_encode( message, TsEncoderDebug, NULL, 0 );

	// encode thingspace cbor
	ts_status_debug("** encode TS-CBOR\n");
	uint8_t buffer[ 2049 ]; // one extra byte for zero termination of string-centric encodings (e.g., json)
	size_t buffer_size = 2048;
	ts_message_encode( message, TsEncoderTsCbor, buffer, &buffer_size );
	ts_status_debug( "** TS-CBOR result (%d), '", buffer_size );
	for( int i = 0; i < buffer_size; i++ ) {
		ts_status_debug("%02x ", buffer[i]);
	}
	ts_status_debug( "'\n" );

	// dump cbor encoding
	ts_status_debug("** decode TS-CBOR\n");
	TsMessageRef_t test;
	ts_message_create( &test );
	ts_message_decode( test, TsEncoderTsCbor, buffer, buffer_size );
	ts_message_encode( test, TsEncoderDebug, NULL, 0 );
	ts_message_destroy( test );

	// encode plain-ole cbor
	ts_status_debug("** encode CBOR\n");
	buffer_size = 2048;
	ts_message_encode( message, TsEncoderCbor, buffer, &buffer_size );
	ts_status_debug( "** CBOR result (%d), '", buffer_size );
	for( int i = 0; i < buffer_size; i++ ) {
		ts_status_debug("%02x ", buffer[i]);
	}
	ts_status_debug( "'\n" );

	// dump cbor encoding
	ts_status_debug("** decode CBOR\n");
	ts_message_create( &test );
	ts_message_decode( test, TsEncoderCbor, buffer, buffer_size );
	ts_message_encode( test, TsEncoderDebug, NULL, 0 );
	ts_message_destroy( test );

	// encode json
	ts_status_debug("** encode JSON\n");
	buffer_size = 2048;
	ts_message_encode( message, TsEncoderJson, buffer, &buffer_size );
	buffer[buffer_size] = 0x00;
	ts_status_debug("** JSON result (%d), '%s'\n", strlen((char*)buffer), buffer);

	// dump json encoding
	ts_status_debug("** decode json\n");
	ts_message_create( &test );
	ts_message_decode( test, TsEncoderJson, buffer, buffer_size );
	ts_message_encode( test, TsEncoderDebug, NULL, 0 );

	// stop
	ts_message_destroy( message );
	ts_message_report();

	ts_status_debug("** done.\n");
}

