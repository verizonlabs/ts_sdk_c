// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_CONNECTION_H
#define TS_CONNECTION_H

#include <stdint.h>
#include <stdio.h>

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_address.h"
#include "ts_security.h"
#include "ts_controller.h"

/**
 * A connection initializes, encapsulates and forwards calls through connection
 * the pipeline, i.e., this --> ts_security --> ts_controller --> ts_channel
 */
typedef struct TsConnection * TsConnectionRef_t;
typedef struct TsConnection {

	TsSecurityRef_t _security;
	TsProfileRef_t _profile;

} TsConnection_t;

#ifdef __cplusplus
extern "C" {
#endif

TsStatus_t ts_connection_create( TsConnectionRef_t * );
TsStatus_t ts_connection_destroy( TsConnectionRef_t );
TsStatus_t ts_connection_tick( TsConnectionRef_t, uint32_t );

TsStatus_t ts_connection_get_security( TsConnectionRef_t, TsSecurityRef_t *);
TsStatus_t ts_connection_get_profile( TsConnectionRef_t, TsProfileRef_t *);

TsStatus_t ts_connection_get_spec_mtu( TsConnectionRef_t, uint32_t* );
// TODO - should the signature be size_t* (in/out parameter)
TsStatus_t ts_connection_get_spec_id( TsConnectionRef_t, const uint8_t *, size_t );
TsStatus_t ts_connection_get_spec_budget( TsConnectionRef_t, uint32_t* );

// TODO - TsStatus_t ts_connection_set_spec_id( TsConnectionRef_t, const uint8_t *, size_t );

TsStatus_t ts_connection_set_server_cert_hostname( TsConnectionRef_t, const char * );
TsStatus_t ts_connection_set_server_cert( TsConnectionRef_t, const uint8_t *, size_t );
TsStatus_t ts_connection_set_client_cert( TsConnectionRef_t, const uint8_t *, size_t );
TsStatus_t ts_connection_set_client_key( TsConnectionRef_t, const uint8_t *, size_t );

TsStatus_t ts_connection_connect( TsConnectionRef_t, TsAddress_t );
TsStatus_t ts_connection_disconnect( TsConnectionRef_t );
TsStatus_t ts_connection_read( TsConnectionRef_t, const uint8_t *, size_t *, uint32_t );
TsStatus_t ts_connection_write( TsConnectionRef_t, const uint8_t *, size_t *, uint32_t );

#ifdef __cplusplus
}
#endif

#endif // TS_CONNECTION_H
