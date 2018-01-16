// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_SECURITY_H
#define TS_SECURITY_H

#include <stdint.h>
#include <stdio.h>

#include "ts_status.h"
#include "ts_profile.h"
#include "ts_address.h"
#include "ts_controller.h"

// the server identity (hostname) that must be present in the server
// certificate CN or SubjectAltName.
// TODO - move this out of a compiler define, double check actual setting
#ifndef SSL_HOST
#define SSL_HOST "simpm.thingspace.verizon.com"
#endif
#ifndef SSL_HANDSHAKE_TIMEOUT
#define SSL_HANDSHAKE_TIMEOUT 30e6
#endif
#ifndef SSL_READ_BUDGET
#define SSL_READ_BUDGET		50e3
#endif
#ifndef SSL_WRITE_BUDGET
#define SSL_WRITE_BUDGET	50e3
#endif

typedef struct TsSecurity *TsSecurityRef_t;
typedef struct TsSecurity {

	TsControllerRef_t   _controller;
	TsProfileRef_t      _profile;

} TsSecurity_t;

typedef struct TsSecurityVtable {

	TsStatus_t (*create)(TsSecurityRef_t *);
	TsStatus_t (*destroy)(TsSecurityRef_t);
	TsStatus_t (*tick)(TsSecurityRef_t, uint32_t);

	TsStatus_t (*set_server_cert_hostname)(TsSecurityRef_t, const char *);
	TsStatus_t (*set_server_cert)(TsSecurityRef_t, const uint8_t *, size_t);
	TsStatus_t (*set_client_cert)(TsSecurityRef_t, const uint8_t *, size_t);
	TsStatus_t (*set_client_key)(TsSecurityRef_t, const uint8_t *, size_t);

	TsStatus_t (*connect)(TsSecurityRef_t, TsAddress_t);
	TsStatus_t (*disconnect)(TsSecurityRef_t);
	TsStatus_t (*read)(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);
	TsStatus_t (*write)(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);

} TsSecurityVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

// security depends on ts_controller
extern const TsSecurityVtable_t * ts_security;

#define ts_security_create			ts_security->create
#define ts_security_destroy			ts_security->destroy
#define ts_security_tick			ts_security->tick

#define ts_security_set_server_cert_hostname ts_security->set_server_cert_hostname
#define ts_security_set_server_cert	ts_security->set_server_cert
#define ts_security_set_client_cert	ts_security->set_client_cert
#define ts_security_set_client_key	ts_security->set_client_key

#define ts_security_connect			ts_security->connect
#define ts_security_disconnect		ts_security->disconnect
#define ts_security_read			ts_security->read
#define ts_security_write			ts_security->write

#ifdef __cplusplus
}
#endif
#endif // TS_SECURITY_H
