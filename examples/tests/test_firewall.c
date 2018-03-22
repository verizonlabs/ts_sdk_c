// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_firewall.h"
#include "ts_platform.h"

int main() {

	TsStatus_t status;
	ts_status_set_level( TsStatusLevelTrace );

	// create new firewall
	TsFirewallRef_t firewall;
	ts_status_debug( "test_firewall: create firewall, %s\n", ts_status_string( ts_firewall_create( &firewall ) ) );

	// test simple configuration setting
	char * xmessage =
		"{\"transactionid\":\"00000000-0000-0000-0000-000000000001\","
		"\"kind\":\"ts.event.firewall\","
		"\"action\":\"set\","
		"\"fields\":{"
		"\"configuration\":{"
			"\"enabled\":true,"
			"\"default_domains\":[\"google.com\",\"verizon.com\",\"amazon.com\"],"
			"\"default_rules\":[{\"sense\":\"outbound\",\"action\":\"drop\",\"destination\":{\"address\":\"35.194.94.155\"}},"
				"{\"sense\":\"outbound\",\"action\":\"drop\",\"destination\":{\"address\":\"35.194.94.156\"}}]"
		"}}}";
	char * ymessage =
		"{\"transactionid\":\"00000000-0000-0000-0000-000000000002\","
			"\"kind\":\"ts.event.firewall\","
			"\"action\":\"get\","
			"\"fields\":{"
			"\"configuration\":null,"
			"\"rules\":null"
			"}}";
/*
	// test decoding to message from json
	TsMessageRef_t from_json;
	ts_status_debug( "test_firewall: create message from_json, %s\n", ts_status_string( ts_message_create( &from_json ) ) );
	ts_status_debug( "test_firewall: decode json, %s\n", ts_status_string( ts_message_decode( from_json, TsEncoderJson, (uint8_t*)xmessage, strlen(xmessage) ) ) );
	ts_status_debug( "test_firewall: encode debug, %s\n", ts_status_string( ts_message_encode( from_json, TsEncoderDebug, NULL, 0 ) ) );

	// test encoding to ts-cbor from message
	uint8_t buffer[ 2048 ];
	size_t buffer_size = sizeof( buffer );
	ts_status_debug( "test_firewall: encode ts-cbor, %s\n", ts_status_string( ts_message_encode( from_json, TsEncoderTsCbor, buffer, &buffer_size ) ) );
	for( int i = 0; i < buffer_size; i++ ) {
		ts_platform_printf( "%02x ", buffer[i] );
	}
	ts_platform_printf( "\n" );
	ts_status_debug( "test_firewall: destroy from_json, %s\n", ts_status_string( ts_message_destroy( from_json ) ) );

	// test decoding from ts-cbor to message
	TsMessageRef_t from_cbor;
	ts_status_debug( "test_firewall: create new message from_cbor, %s\n", ts_status_string( ts_message_create( &from_cbor ) ) );
	ts_status_debug( "test_firewall: decode ts-cbor, %s\n", ts_status_string( ts_message_decode( from_cbor, TsEncoderTsCbor, buffer, buffer_size ) ) );
	ts_status_debug( "test_firewall: encode debug, %s\n", ts_status_string( ts_message_encode( from_cbor, TsEncoderDebug, NULL, 0 ) ) );
	ts_status_debug( "test_firewall: destroy from_cbor, %s\n", ts_status_string( ts_message_destroy( from_cbor ) ) );
*/
	// test firewall handler, set
	TsMessageRef_t message;
	ts_status_debug( "test_firewall: create firewall message, %s\n", ts_status_string( ts_message_create( &message ) ) );
	ts_status_debug( "test_firewall: decode json, %s\n", ts_status_string( ts_message_decode( message, TsEncoderJson, (uint8_t*)xmessage, strlen(xmessage) ) ) );
	ts_status_debug( "test_firewall: handle set firewall message, %s\n", ts_status_string( ts_firewall_handle( firewall, message ) ) );
	ts_status_debug( "test_firewall: destroy firewall message, %s\n", ts_status_string( ts_message_destroy( message ) ) );

	// test firewall handler, get
	ts_status_debug( "test_firewall: create firewall message, %s\n", ts_status_string( ts_message_create( &message ) ) );
	ts_status_debug( "test_firewall: decode json, %s\n", ts_status_string( ts_message_decode( message, TsEncoderJson, (uint8_t*)ymessage, strlen(ymessage) ) ) );
	ts_status_debug( "test_firewall: handle get firewall message, %s\n", ts_status_string( ts_firewall_handle( firewall, message ) ) );
	ts_status_debug( "test_firewall: result, %s\n", ts_status_string( ts_message_encode( message, TsEncoderDebug, NULL, 0 ) ) );
	ts_status_debug( "test_firewall: destroy firewall message, %s\n", ts_status_string( ts_message_destroy( message ) ) );


	// clean up
	ts_status_debug( "test_firewall: destroy firewall, %s\n", ts_status_string( ts_firewall_destroy( firewall ) ) );
}
