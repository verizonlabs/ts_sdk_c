// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_TRANSPORT_H
#define TS_TRANSPORT_H

#include "ts_status.h"
#include "ts_address.h"
#include "ts_message.h"
#include "ts_connection.h"

typedef struct TsTransport * TsTransportRef_t;
typedef TsStatus_t (*TsTransportHandler_t)( TsTransportRef_t, TsPath_t, const uint8_t *, size_t );
typedef struct TsTransport {
	TsConnectionRef_t _connection;
	TsTransportHandler_t _handler;
} TsTransport_t;

typedef struct TsTransportVtable {

	TsStatus_t (* create)( TsTransportRef_t * );
	TsStatus_t (* destroy)( TsTransportRef_t );
	TsStatus_t (* tick)( TsTransportRef_t, uint32_t );

	TsStatus_t (* get_connection)( TsTransportRef_t, TsConnectionRef_t* );

	TsStatus_t (* dial)( TsTransportRef_t, TsAddress_t );
	TsStatus_t (* hangup)( TsTransportRef_t );
	TsStatus_t (* listen)( TsTransportRef_t, TsAddress_t, TsPath_t, TsTransportHandler_t );
	TsStatus_t (* speak)( TsTransportRef_t, TsPath_t, const uint8_t *, size_t );

} TsTransportVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsTransportVtable_t * ts_transport;

#define ts_transport_create ts_transport->create
#define ts_transport_destroy ts_transport->destroy
#define ts_transport_tick   ts_transport->tick

#define ts_transport_get_connection ts_transport->get_connection

#define ts_transport_dial   ts_transport->dial
#define ts_transport_hangup ts_transport->hangup
#define ts_transport_listen ts_transport->listen
#define ts_transport_speak  ts_transport->speak

#ifdef __cplusplus
extern "C" {
#endif

#endif // TS_TRANSPORT_H
