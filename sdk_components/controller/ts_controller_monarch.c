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

TsControllerVtable_t ts_controller_monarch = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

static TsStatus_t ts_create( TsControllerRef_t * connection ) {
	ts_status_trace( "ts_controller_create: monarch\n" );

	// TODO - set spec data appropriately
	// connection->_spec_...
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_destroy( TsControllerRef_t connection ) {
	ts_status_trace( "ts_controller_destroy\n" );
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_tick( TsControllerRef_t connection, uint32_t budget ) {
	ts_status_trace( "ts_controller_tick\n" );
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_connect( TsControllerRef_t connection, TsAddress_t address ) {
	ts_status_trace( "ts_controller_connect\n" );
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_disconnect( TsControllerRef_t connection ) {
	ts_status_trace( "ts_controller_disconnect\n" );
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_read( TsControllerRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_read\n" );
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_write( TsControllerRef_t connection, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_write\n" );
	return TsStatusErrorNotImplemented;
}
