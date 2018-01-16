// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_security.h"

static TsStatus_t ts_create(TsSecurityRef_t *);
static TsStatus_t ts_destroy(TsSecurityRef_t);
static TsStatus_t ts_tick(TsSecurityRef_t, uint32_t);
static TsStatus_t ts_report(TsSecurityRef_t);

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t, const char *);
static TsStatus_t ts_set_server_cert(TsSecurityRef_t, const uint8_t *, size_t);
static TsStatus_t ts_set_client_cert(TsSecurityRef_t, const uint8_t *, size_t);
static TsStatus_t ts_set_client_key(TsSecurityRef_t, const uint8_t *, size_t);

static TsStatus_t ts_connect(TsSecurityRef_t, TsAddress_t);
static TsStatus_t ts_disconnect(TsSecurityRef_t);
static TsStatus_t ts_read(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);
static TsStatus_t ts_write(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);

TsSecurityVtable_t ts_security_mocana = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.set_server_cert_hostname = ts_set_server_cert_hostname,
	.set_server_cert = ts_set_server_cert,
	.set_client_cert = ts_set_client_cert,
	.set_client_key = ts_set_client_key,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

static TsStatus_t ts_create(TsSecurityRef_t * security) {
	ts_status_trace("ts_security_create: mocana\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_destroy(TsSecurityRef_t security) {
	ts_status_trace("ts_security_destroy\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_tick(TsSecurityRef_t security, uint32_t budget) {
	ts_status_trace("ts_security_tick\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t security, const char * hostname) {
	ts_status_trace("ts_set_server_cert_hostname\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_set_server_cert(TsSecurityRef_t security, const uint8_t * cacert, size_t cacert_size) {
	ts_status_trace("ts_security_set_server_cert\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_set_client_cert(TsSecurityRef_t security, const uint8_t * clcert, size_t clcert_size) {
	ts_status_trace("ts_security_set_client_cert\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_set_client_key(TsSecurityRef_t security, const uint8_t * clkey, size_t key_size) {
	ts_status_trace("ts_security_set_client_key\n");
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_connect(TsSecurityRef_t security, TsAddress_t address) {
	ts_status_trace("ts_security_connect\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_disconnect(TsSecurityRef_t security) {
	ts_status_trace("ts_security_disconnect\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_read(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_security_read\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_write(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_security_write\n");
	ts_platform_assert(ts_controller != NULL);
	return TsStatusErrorNotImplemented;
}
