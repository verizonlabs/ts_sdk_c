// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_SERVICE_H
#define TS_SERVICE_H

#include "ts_message.h"
#include "ts_transport.h"

#define TS_SERVICE_MAX_HANDLERS 8
#define TS_SERVICE_MAX_PATH_SIZE 256

// TODO - fix to one enum, and one mapping
typedef enum {

	TsServiceActionGet = 0x0001,
	TsServiceActionSet = 0x0002,
	TsServiceActionActivate = 0x0010,
	TsServiceActionDeactivate = 0x0020,
	TsServiceActionSuspend = 0x0040,
	TsServiceActionResume = 0x0080,
	TsServiceActionMaskAll = 0xFFFF,
	TsServiceActionMaskChannel = 0x000F,
	TsServiceActionMaskProvision = 0x00F0,

} TsServiceAction_t;

typedef enum {

	TsServiceActionGetIndex = 0,
	TsServiceActionSetIndex = 1,
	TsServiceActionActivateIndex = 4,
	TsServiceActionDeactivateIndex = 5,
	TsServiceActionSuspendIndex = 6,
	TsServiceActionResumeIndex = 7,
} TsServiceActionIndex_t;

typedef struct TsService * TsServiceRef_t;

// TODO - should we follow transport pattern, allowing us to echo a void* (set when dequeue called)?
typedef TsStatus_t (* TsServiceHandler_t)( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );

// TODO - should we switch all specializations to use a void* for state?
typedef struct TsService {
	char                _subscription[TS_SERVICE_MAX_PATH_SIZE];
	TsServiceHandler_t  _handlers[TS_SERVICE_MAX_HANDLERS];
	TsTransportRef_t    _transport;
} TsService_t;

typedef struct TsServiceVtable {

	TsStatus_t (*create)( TsServiceRef_t * );
	TsStatus_t (*destroy)( TsServiceRef_t );
	TsStatus_t (*tick)( TsServiceRef_t, uint32_t );

	TsStatus_t (*enqueue)( TsServiceRef_t, TsMessageRef_t );
	TsStatus_t (*dequeue)( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

} TsServiceVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsServiceVtable_t * ts_service;

TsStatus_t ts_service_create( TsServiceRef_t * );
TsStatus_t ts_service_destroy( TsServiceRef_t );
TsStatus_t ts_service_tick( TsServiceRef_t, uint32_t );

TsStatus_t ts_service_set_server_cert_hostname( TsServiceRef_t, const char * );
TsStatus_t ts_service_set_server_cert( TsServiceRef_t, const uint8_t *, size_t );
TsStatus_t ts_service_set_client_cert( TsServiceRef_t, const uint8_t *, size_t );
TsStatus_t ts_service_set_client_key( TsServiceRef_t, const uint8_t *, size_t );

TsStatus_t ts_service_dial( TsServiceRef_t, TsAddress_t );
TsStatus_t ts_service_hangup( TsServiceRef_t );

// TODO - how should they enqueue with a createdOn date, etc.
TsStatus_t ts_service_enqueue( TsServiceRef_t, TsMessageRef_t );
TsStatus_t ts_service_dequeue( TsServiceRef_t, TsServiceAction_t, TsServiceHandler_t );

#ifdef __cplusplus
}
#endif

#endif // TS_SERVICE_H
