// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_driver.h"

static TsStatus_t ts_create(TsDriverRef_t *);
static TsStatus_t ts_destroy(TsDriverRef_t);
static TsStatus_t ts_tick(TsDriverRef_t, uint32_t);

static TsStatus_t ts_connect(TsDriverRef_t, TsAddress_t);
static TsStatus_t ts_disconnect(TsDriverRef_t);
static TsStatus_t ts_read(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);
static TsStatus_t ts_write(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);

/**
 * typically a tty device
 */
TsDriverVtable_t ts_driver_device = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

static TsStatus_t ts_create(TsDriverRef_t * driver) {
	ts_status_trace("ts_driver_create\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_destroy(TsDriverRef_t driver) {
	ts_status_trace("ts_driver_destroy\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_tick(TsDriverRef_t driver, uint32_t budget) {
	ts_status_trace("ts_driver_tick\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_connect(TsDriverRef_t driver, TsAddress_t address) {
	ts_status_trace("ts_driver_connect\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_disconnect(TsDriverRef_t driver) {
	ts_status_trace("ts_driver_disconnect\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_read(TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_driver_read\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_write(TsDriverRef_t driver, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_driver_write\n");
	return TsStatusErrorNotImplemented;
}
