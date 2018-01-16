// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_protocol.h"

static TsStatus_t ts_create( TsProtocolRef_t * );
static TsStatus_t ts_destroy( TsProtocolRef_t );
static TsStatus_t ts_tick( TsProtocolRef_t, uint32_t );

static TsStatus_t ts_enqueue( TsProtocolRef_t, TsMessageRef_t );
static TsStatus_t ts_dequeue( TsProtocolRef_t, TsPath_t, TsProtocolHandler_t );

TsProtocolVtable_t ts_protocol_ts_json = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,
	.enqueue = ts_enqueue,
	.dequeue = ts_dequeue,
};

static TsStatus_t ts_create( TsProtocolRef_t * protocol ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_destroy( TsProtocolRef_t protocol ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_tick( TsProtocolRef_t protocol, uint32_t budget ) {
	return TsStatusErrorNotImplemented;
}

// TODO - remember to include the "lastUpdate" value in unsolicited events
static TsStatus_t ts_enqueue( TsProtocolRef_t protocol, TsMessageRef_t message ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_dequeue( TsProtocolRef_t protocol, TsPath_t path, TsProtocolHandler_t handler ) {
	return TsStatusErrorNotImplemented;
}
