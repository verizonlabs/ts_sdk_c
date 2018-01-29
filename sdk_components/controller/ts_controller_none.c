// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_controller.h"

static TsStatus_t ts_create( TsControllerRef_t * );
static TsStatus_t ts_destroy( TsControllerRef_t );
static TsStatus_t ts_tick( TsControllerRef_t, uint32_t );

static TsStatus_t ts_connect( TsControllerRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsControllerRef_t );
static TsStatus_t ts_read( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_write( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );

TsControllerVtable_t ts_controller_none = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

static TsStatus_t ts_create( TsControllerRef_t * controller ) {

	ts_status_trace( "ts_controller_create: monarch\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsDriverRef_t driver;
	TsStatus_t status = ts_driver_create( &driver );
	if( status != TsStatusOk ) {
		return TsStatusErrorInternalServerError;
	}

	*controller = (TsControllerRef_t)ts_platform_malloc( sizeof( TsController_t ) );
	if( *controller == NULL ) {
		return TsStatusErrorInternalServerError;
	}
	(*controller)->_driver = driver;

	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsControllerRef_t controller ) {

	ts_status_trace( "ts_controller_destroy\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	ts_driver_destroy( controller->_driver );
	ts_platform_free( controller, sizeof( TsController_t ) );

	return TsStatusOk;
}

static TsStatus_t ts_tick( TsControllerRef_t controller, uint32_t budget ) {

	ts_status_trace( "ts_controller_tick\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return ts_driver_tick( controller->_driver, budget );
}

static TsStatus_t ts_connect( TsControllerRef_t controller, TsAddress_t address ) {

	ts_status_trace( "ts_controller_connect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return ts_driver_connect( controller->_driver, address );
}

static TsStatus_t ts_disconnect( TsControllerRef_t controller ) {

	ts_status_trace( "ts_controller_disconnect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return ts_driver_disconnect( controller->_driver );
}

static TsStatus_t ts_read( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_controller_read\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return ts_driver_read( controller->_driver, buffer, buffer_size, budget );
}

static TsStatus_t ts_write( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {

	ts_status_trace( "ts_controller_write\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return ts_driver_write( controller->_driver, buffer, buffer_size, budget );
}
