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

	/* Diagnostics API */
	TsStatus_t (*get_id)(TsControllerRef_t, char *);
	TsStatus_t (*get_rssi)(TsControllerRef_t, char *);
	TsStatus_t (*get_ipv4_addr)(TsControllerRef_t, char *);
	TsStatus_t (*get_iccid)(TsControllerRef_t, char *);
	TsStatus_t (*get_date_and_time)(TsControllerRef_t, char *);
	TsStatus_t (*get_imsi)(TsControllerRef_t, char *);
	TsStatus_t (*get_manufacturer)(TsControllerRef_t, char *);
	TsStatus_t (*get_module_name)(TsControllerRef_t, char *);
	TsStatus_t (*get_firmware_version)(TsControllerRef_t, char *);

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

#define ts_controller_get_id			ts_controller->get_id
#define ts_controller_get_rssi			ts_controller->get_rssi
#define ts_controller_get_ipv4_addr		ts_controller->get_ipv4_addr
#define ts_controller_get_iccid			ts_controller->get_iccid
#define ts_controller_get_date_and_time		ts_controller->get_date_and_time
#define ts_controller_get_imsi			ts_controller->get_imsi
#define ts_controller_get_manufacturer		ts_controller->get_manufacturer
#define ts_controller_get_module_name		ts_controller->get_module_name
#define ts_controller_get_firmware_version	ts_controller->get_firmware_version

#ifdef __cplusplus
}
#endif

#endif // TS_CONTROLLER_H
