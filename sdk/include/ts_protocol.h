// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PROTOCOL_H
#define TS_PROTOCOL_H

#include <stdint.h>

#include "ts_status.h"
#include "ts_address.h"
#include "ts_message.h"
#include "ts_service.h"
#include "ts_transport.h"

typedef struct TsProtocol * TsProtocolRef_t;
typedef struct TsProtocol {
	TsTransportRef_t _transport;
} TsProtocol_t;

typedef struct TsProtocolVtable * TsProtocolVtableRef_t;
typedef struct TsProtocolVtable {

	TsStatus_t (* create)( TsProtocolRef_t * );
	TsStatus_t (* destroy)( TsProtocolRef_t );

	TsStatus_t (* enqueue )( TsProtocolRef_t, TsMessageRef_t );
	TsStatus_t (* dequeue)( TsProtocolRef_t, /* TODO */ );

} TsProtocolVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsProtocolVtable_t * ts_protocol;

#define ts_protocol_create           ts_protocol->create
#define ts_protocol_destroy          ts_protocol->destroy
#define ts_protocol_tick             ts_protocol->tick

#define ts_protocol_enqueue          ts_protocol->enqueue
#define ts_protocol_dequeue          ts_protocol->dequeue

#ifdef __cplusplus
extern "C" {
#endif

#endif // TS_PROTOCOL_H
