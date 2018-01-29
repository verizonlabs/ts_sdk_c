// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_CONTROLLER_H
#define TS_CONTROLLER_H

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_driver.h"

typedef struct TsController *TsControllerRef_t;
typedef struct TsController {

	TsDriverRef_t   _driver;
	TsProfileRef_t	_profile;

} TsController_t;

typedef struct TsControllerVtable {

	TsStatus_t (*create)(TsControllerRef_t *);
	TsStatus_t (*destroy)(TsControllerRef_t);
	TsStatus_t (*tick)(TsControllerRef_t, uint32_t);

	TsStatus_t (*connect)(TsControllerRef_t, TsAddress_t);
	TsStatus_t (*disconnect)(TsControllerRef_t);
	TsStatus_t (*read)(TsControllerRef_t, const uint8_t *, size_t *, uint32_t);
	TsStatus_t (*write)(TsControllerRef_t, const uint8_t *, size_t *, uint32_t);

} TsControllerVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

// controller depends on ts_device
extern const TsControllerVtable_t *ts_controller;

#define ts_controller_create			ts_controller->create
#define ts_controller_destroy			ts_controller->destroy
#define ts_controller_tick				ts_controller->tick

#define ts_controller_connect			ts_controller->connect
#define ts_controller_disconnect		ts_controller->disconnect
#define ts_controller_read				ts_controller->read
#define ts_controller_write				ts_controller->write

#ifdef __cplusplus
}
#endif

#endif // TS_CONTROLLER_H
