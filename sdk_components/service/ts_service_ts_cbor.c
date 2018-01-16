// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_service.h"

static TsStatus_t ts_create( TsServiceRef_t * );
static TsStatus_t ts_destroy( TsServiceRef_t );
static TsStatus_t ts_tick( TsServiceRef_t, uint32_t );

static TsStatus_t ts_enqueue( TsServiceRef_t, TsMessageRef_t );
static TsStatus_t ts_dequeue( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

TsServiceVtable_t ts_service_ts_cbor = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,
	.enqueue = ts_enqueue,
	.dequeue = ts_dequeue,
};

static TsStatus_t ts_create( TsServiceRef_t * service ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_destroy( TsServiceRef_t service ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_tick( TsServiceRef_t service, uint32_t budget ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_enqueue( TsServiceRef_t service, TsMessageRef_t message ) {
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_dequeue( TsServiceRef_t service, TsServiceAction_t action, TsServiceHandler_t handler ) {
	return TsStatusErrorNotImplemented;
}
