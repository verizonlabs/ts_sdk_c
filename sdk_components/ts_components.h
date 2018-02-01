// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_COMPONENTS_H
#define TS_COMPONENTS_H

#include "ts_service.h"
#include "ts_transport.h"
#include "ts_controller.h"
#include "ts_security.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TsServiceVtable_t    ts_service_ts_cbor;
extern TsServiceVtable_t    ts_service_ts_json;

extern TsTransportVtable_t  ts_transport_mqtt;

extern TsControllerVtable_t ts_controller_monarch;
extern TsControllerVtable_t ts_controller_none;

extern TsSecurityVtable_t   ts_security_mbedtls;
extern TsSecurityVtable_t   ts_security_mocana;
extern TsSecurityVtable_t   ts_security_none;

#ifdef __cplusplus
}
#endif

#endif // TS_COMPONENTS_H
