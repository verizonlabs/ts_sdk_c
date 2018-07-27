// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_security.h"
#include "ts_controller.h"

static TsStatus_t ts_initialize();

static TsStatus_t ts_create(TsSecurityRef_t *);
static TsStatus_t ts_destroy(TsSecurityRef_t);
static TsStatus_t ts_tick(TsSecurityRef_t, uint32_t);

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t, const char *);
static TsStatus_t ts_set_server_cert(TsSecurityRef_t, const uint8_t *, size_t);
static TsStatus_t ts_set_client_cert(TsSecurityRef_t, const uint8_t *, size_t);
static TsStatus_t ts_set_client_key(TsSecurityRef_t, const uint8_t *, size_t);

static TsStatus_t ts_get_spec_mtu( TsSecurityRef_t, uint32_t* );
static TsStatus_t ts_get_spec_id( TsSecurityRef_t, const uint8_t *, size_t );
static TsStatus_t ts_get_spec_budget( TsSecurityRef_t, uint32_t* );

static TsStatus_t ts_connect(TsSecurityRef_t, TsAddress_t);
static TsStatus_t ts_disconnect(TsSecurityRef_t);
static TsStatus_t ts_read(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);
static TsStatus_t ts_write(TsSecurityRef_t, const uint8_t *, size_t *, uint32_t);

TsSecurityVtable_t ts_security_none = {
	.initialize = ts_initialize,

	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.set_server_cert_hostname = ts_set_server_cert_hostname,
	.set_server_cert = ts_set_server_cert,
	.set_client_cert = ts_set_client_cert,
	.set_client_key = ts_set_client_key,

	.get_spec_mtu = ts_get_spec_mtu,
	.get_spec_id = ts_get_spec_id,
	.get_spec_budget = ts_get_spec_budget,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

static TsStatus_t ts_initialize() {
	return TsStatusOk;
}

static TsStatus_t ts_create(TsSecurityRef_t * security) {
	ts_status_trace("ts_security_create: none\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	// first, allocate and initialize controller memory
	TsControllerRef_t controller;
	TsStatus_t status = ts_controller_create(&controller);
	if( status != TsStatusOk ) {
		return status;
	}

	// the, allocate and initialize security
	*security = (TsSecurityRef_t) (ts_platform_malloc(sizeof(TsSecurity_t)));
	(*security)->_controller = controller;

	return TsStatusOk;
}

static TsStatus_t ts_destroy(TsSecurityRef_t security) {
	ts_status_trace("ts_security_destroy\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	// first, free controller memory
	TsStatus_t status = ts_controller_destroy(security->_controller);
	if( status != TsStatusOk ) {
		ts_status_debug("ts_security_destroy: ignoring failure, '%s'\n", ts_status_string(status));
	}

	// release the rest
	ts_platform_free(security, sizeof(TsSecurity_t));
	return TsStatusOk;
}

static TsStatus_t ts_tick(TsSecurityRef_t security, uint32_t budget) {
	ts_status_trace("ts_security_tick\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return ts_controller_tick(security->_controller, budget);
}

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t security, const char * hostname) {
	ts_status_trace("ts_set_server_cert_hostname\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return TsStatusOk;
}

static TsStatus_t ts_set_server_cert(TsSecurityRef_t security, const uint8_t * cacert, size_t cacert_size) {
	ts_status_trace("ts_security_set_server_cert\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return TsStatusOk;
}

static TsStatus_t ts_set_client_cert(TsSecurityRef_t security, const uint8_t * clcert, size_t clcert_size) {
	ts_status_trace("ts_security_set_client_cert\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return TsStatusOk;
}

static TsStatus_t ts_set_client_key(TsSecurityRef_t security, const uint8_t * clkey, size_t key_size) {
	ts_status_trace("ts_security_set_client_key\n");
	return TsStatusOk;
}

static TsStatus_t ts_get_spec_mtu( TsSecurityRef_t security, uint32_t* mtu ) {

	ts_status_trace( "ts_security_get_spec_mtu\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( security != NULL );
	ts_platform_assert( security->_controller != NULL );
	ts_platform_assert( security->_controller->_driver != NULL );
	ts_platform_assert( mtu != NULL );

	// TODO - This should call the next link in pipeline
	*mtu = security->_controller->_driver->_spec_mtu;

	return TsStatusOk;
}

static TsStatus_t ts_get_spec_id( TsSecurityRef_t security, const uint8_t * id, size_t id_size ) {

	ts_status_trace( "ts_security_get_spec_id\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( security != NULL );
	ts_platform_assert( security->_controller != NULL );
	ts_platform_assert( security->_controller->_driver != NULL );
	ts_platform_assert( id != NULL );
	ts_platform_assert( id_size <= TS_DRIVER_MAX_ID_SIZE );

	// TODO - This should call the next link in pipeline
	snprintf( (char *)id, id_size, "%s", (const char *)(security->_controller->_driver->_spec_id) );

	return TsStatusOk;
}

static TsStatus_t ts_get_spec_budget( TsSecurityRef_t security, uint32_t* budget ) {

	ts_status_trace( "ts_security_get_spec_budget\n" );
	ts_platform_assert( ts_security != NULL );
	ts_platform_assert( security != NULL );
	ts_platform_assert( security->_controller != NULL );
	ts_platform_assert( security->_controller->_driver != NULL );
	ts_platform_assert( budget != NULL );

	// TODO - This should call the next link in pipeline
	*budget = security->_controller->_driver->_spec_budget;

	return TsStatusOk;
}

static TsStatus_t ts_connect(TsSecurityRef_t security, TsAddress_t address) {
	ts_status_trace("ts_security_connect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return ts_controller_connect(security->_controller, address);
}

static TsStatus_t ts_disconnect(TsSecurityRef_t security) {
	ts_status_trace("ts_security_disconnect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return ts_controller_disconnect(security->_controller);
}

static TsStatus_t ts_read(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_security_read\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return ts_controller_read(security->_controller, buffer, buffer_size, budget);
}

static TsStatus_t ts_write(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {
	ts_status_trace("ts_security_write\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	return ts_controller_write(security->_controller, buffer, buffer_size, budget);
}
