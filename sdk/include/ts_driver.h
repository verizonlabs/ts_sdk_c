// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_DRIVER_H
#define TS_DRIVER_H

#include "ts_status.h"
#include "ts_profile.h"

typedef struct TsDriver *TsDriverRef_t;
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
	TsStatus_t (*write)(TsDriverRef_t, const uint8_t *, size_t *, uint32_t);

} TsDriverVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsDriverVtable_t * ts_driver;

#define ts_diver_create		ts_diver->create
#define ts_diver_destroy    ts_diver->destroy
#define ts_diver_tick		ts_diver->tick

#define ts_diver_connect	ts_diver->connect
#define ts_diver_disconnect	ts_diver->disconnect
#define ts_diver_read		ts_diver->read
#define ts_diver_write		ts_diver->write

#ifdef __cplusplus
}
#endif

#endif // TS_DRIVER_H
