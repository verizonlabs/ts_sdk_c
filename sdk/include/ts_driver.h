// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_DRIVER_H
#define TS_DRIVER_H

#include "ts_status.h"
#include "ts_profile.h"

typedef struct TsDriver *TsDriverRef_t;
typedef TsStatus_t (*TsDriverRead_t)(TsDriverRef_t, void*, const uint8_t *, size_t);
typedef struct TsDriver {

	TsProfileRef_t _profile;

} TsDriver_t;

/**
 * The (optional) "channel" provides access to the network-layer via
 * a device driver, typically used to access the SMS or TCP/IP stack on
 * a cellular modem.
 */
typedef struct TsDriverVtable {

	TsStatus_t (*create)(TsDriverRef_t *);
	TsStatus_t (*destroy)(TsDriverRef_t);
	TsStatus_t (*tick)(TsDriverRef_t, uint32_t);

	TsStatus_t (*connect)(TsDriverRef_t, TsAddress_t);
	TsStatus_t (*disconnect)(TsDriverRef_t);
	TsStatus_t (*read)(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);
	/* opt. reader performs a callback to the given function when data received,
	 * passing the given void* in the same function (e.g., controller pointer) */
	TsStatus_t (*reader)(TsDriverRef_t, void*, TsDriverRead_t);
	TsStatus_t (*write)(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);

} TsDriverVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsDriverVtable_t * ts_driver;

#define ts_driver_create    ts_driver->create
#define ts_driver_destroy   ts_driver->destroy
#define ts_driver_tick		ts_driver->tick

#define ts_driver_connect	ts_driver->connect
#define ts_driver_disconnect ts_driver->disconnect
#define ts_driver_read		ts_driver->read
#define ts_driver_reader    ts_driver->reader
#define ts_diver_write		ts_driver->write

#ifdef __cplusplus
}
#endif

#endif // TS_DRIVER_H
