// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_DRIVER_H
#define TS_DRIVER_H

#include "ts_status.h"
#include "ts_profile.h"

#define TS_DRIVER_MAX_ID_SIZE 36

typedef struct TsDriver *TsDriverRef_t;
typedef TsStatus_t (*TsDriverReader_t)(TsDriverRef_t, void*, const uint8_t *, size_t);
typedef struct TsDriver {

	TsProfileRef_t  _profile;
	TsAddress_t		_address;
	TsDriverReader_t _reader;
	void *          _reader_state;

	// advertised driver specifications
	uint32_t        _spec_budget;   // read/write timer budget in microseconds
	// TODO - these values may not belong here - need to resolve all spec parameters at connection level?
	uint32_t        _spec_mtu;      // maximum buffer size in bytes
	uint8_t         _spec_id[TS_DRIVER_MAX_ID_SIZE]; // zero terminated device id

} TsDriver_t;

/**
 * The (optional) "driver" provides access to the network-layer via
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
	TsStatus_t (*reader)(TsDriverRef_t, void*, TsDriverReader_t);
	TsStatus_t (*write)(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);
	void       (*reset)(TsDriverRef_t);

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
#define ts_driver_write		ts_driver->write
#define ts_driver_reset		ts_driver->reset

#ifdef __cplusplus
}
#endif

#endif // TS_DRIVER_H
