// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#if defined( TS_SECURITY_MOCANA )
#include <math.h>
#include "common/moptions.h"
#include "common/mtypes.h"
#include "common/merrors.h"
#include "common/mtcp.h"
#include "crypto/hw_accel.h"
#include "crypto/cert_store.h"
#include "ssl/ssl.h"

#include "ts_platform.h"
#include "ts_security.h"

static TsStatus_t ts_create(TsSecurityRef_t *);
static TsStatus_t ts_destroy(TsSecurityRef_t);
static TsStatus_t ts_tick(TsSecurityRef_t, uint32_t);

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

#define TS_TCP_SOCKET_MAX 6
static int next_socket = 0;
TsControllerRef_t ts_security_mocana_controller[ TS_TCP_SOCKET_MAX ];

typedef struct TsSecurityMocana *TsSecurityMocanaRef_t;
typedef struct TsSecurityMocana {

	// inheritance by encapsulation; must be the first
	// attribute in order to treat this struct as a
	// TsSecurity struct
	TsSecurity_t                _security;

	// mocana attributes
	certStorePtr                _cert_store;
	const uint8_t *             _clcert;
	size_t                      _clcert_size;
	const uint8_t *             _clkey;
	size_t                      _clkey_size;
	const char *                _cacert_hostname;

	// connection instance (socket)
	TCP_SOCKET                  _socket;
	sbyte4                      _connection_instance;

} TsSecurityMocana_t;

static MSTATUS myCertStatusCallback(sbyte4 connectionInstance,
	struct certChain* pCertChain,
	MSTATUS validationstatus) {
	return 0;
}

static TsStatus_t ts_create(TsSecurityRef_t * security) {

	ts_status_trace("ts_security_create: mocana\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	// initialize NanoSSL for 0 server connections and 1 client connection
	MSTATUS mstatus = SSL_init( 0, 1 );
	if( mstatus != OK ) {
		return TsStatusErrorInternalServerError;
	}


	// first, allocate and initialize controller memory
	TsControllerRef_t controller;
	TsStatus_t status = ts_controller_create(&controller);
	if( status != TsStatusOk ) {
		return status;
	}

	// then, allocate and initialize security
	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (ts_platform_malloc(sizeof(TsSecurityMocanaRef_t)));
	*security = (TsSecurityRef_t)mocana;
	(*security)->_controller = controller;
	(*security)->_profile = NULL;

	// mocana specific attributes
	mocana->_cert_store = NULL;
	mocana->_clcert = NULL;
	mocana->_clkey = NULL;
	mocana->_cacert_hostname = NULL;
	mocana->_socket = 0;
	mocana->_connection_instance = 0;

	return TsStatusOk;
}

static TsStatus_t ts_destroy(TsSecurityRef_t security) {

	ts_status_trace("ts_security_destroy\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	// attempt to disconnect if not done already
	if( security->_controller != NULL ) {
		ts_controller_disconnect( security->_controller );
		ts_controller_destroy( security->_controller );
	}

	// free only the security memory (no controller, no profile)
	ts_platform_free(security, sizeof(TsSecurityMocana_t));

	return TsStatusOk;
}

static TsStatus_t ts_tick(TsSecurityRef_t security, uint32_t budget) {

	ts_status_trace("ts_security_tick\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	// delegate tick if the connection has established a controller
	if( security->_controller != NULL ) {
		return ts_controller_tick( security->_controller, budget );
	}

	return TsStatusOk;
}

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t security, const char * hostname) {

	ts_status_trace("ts_set_server_cert_hostname\n");
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);
	mocana->_cacert_hostname = (char*)hostname;

	return TsStatusOk;
}

static TsStatus_t ts_set_server_cert(TsSecurityRef_t security, const uint8_t * cacert, size_t cacert_size) {

	ts_status_trace("ts_security_set_server_cert\n");
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);
	if( mocana->_cert_store == NULL ) {
		MSTATUS mstatus = CERT_STORE_createStore( &(mocana->_cert_store) );
		if( mstatus < OK ) {
			return TsStatusErrorInternalServerError;
		}
	}
	MSTATUS mstatus = CERT_STORE_addTrustPoint( mocana->_cert_store, cacert, (ubyte4)cacert_size );
	if( mstatus != OK ) {
		return TsStatusErrorInternalServerError;
	}

	return TsStatusOk;
}

static TsStatus_t ts_set_client_cert(TsSecurityRef_t security, const uint8_t * clcert, size_t clcert_size) {

	ts_status_trace("ts_security_set_client_cert\n");
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);
	if( mocana->_cert_store == NULL ) {
		MSTATUS mstatus = CERT_STORE_createStore( &(mocana->_cert_store) );
		if( mstatus < OK ) {
			return TsStatusErrorInternalServerError;
		}
	}
	mocana->_clcert = clcert;
	mocana->_clcert_size = clcert_size;
	if( mocana->_clkey != NULL && mocana->_clkey_size != 0 ) {
		MSTATUS status = CERT_STORE_addIdentity( mocana->_cert_store, mocana->_clcert, (ubyte4)(mocana->_clcert_size), mocana->_clkey, (ubyte4)(mocana->_clkey_size) );
		if( status != OK ) {
			return TsStatusErrorInternalServerError;
		}
	}

	return TsStatusOk;
}

static TsStatus_t ts_set_client_key(TsSecurityRef_t security, const uint8_t * clkey, size_t clkey_size) {

	ts_status_trace("ts_security_set_client_key\n");
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);
	if( mocana->_cert_store == NULL ) {
		MSTATUS mstatus = CERT_STORE_createStore( &(mocana->_cert_store) );
		if( mstatus < OK ) {
			return TsStatusErrorInternalServerError;
		}
	}
	mocana->_clkey = clkey;
	mocana->_clkey_size = clkey_size;
	if( mocana->_clcert != NULL && mocana->_clcert_size != 0 ) {
		MSTATUS status = CERT_STORE_addIdentity( mocana->_cert_store, mocana->_clcert, (ubyte4)(mocana->_clcert_size), mocana->_clkey, (ubyte4)(mocana->_clkey_size) );
		if( status != OK ) {
			return TsStatusErrorInternalServerError;
		}
	}

	return TsStatusOk;
}

static TsStatus_t ts_connect(TsSecurityRef_t security, TsAddress_t address) {

	ts_status_trace("ts_security_connect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);

	char host[TS_ADDRESS_MAX_HOST_SIZE], xport[TS_ADDRESS_MAX_PORT_SIZE];
	if (ts_address_parse( address, host, xport ) != TsStatusOk) {
		return TsStatusErrorPreconditionFailed;
	}
	ubyte2 port = (ubyte2)atoi( xport );

	// connect using mocana's "plug-in", which will route back to ts_controller
	// our implementation expects the socket to be pre-initialized
	ts_security_mocana_controller[next_socket] = security->_controller;
	mocana->_socket = next_socket;
	MSTATUS mstatus = TCP_CONNECT( &(mocana->_socket), (sbyte*)host, port );

	// TODO - watch rollover
	next_socket = ( next_socket + 1 ) % TS_TCP_SOCKET_MAX;

	// custom check of the server certificate
	// OCSB (online-certificate protocol) related, SSL_sslSettings()->funcPtrCertStatusCallback = myCertStatusCallback;

	// connect using mocana's ssl-connection on top of our tcp connection
	mocana->_connection_instance = SSL_connect(
		mocana->_socket,        // should be the TCP_SOCKET
		0,                      // session-id length, zero for none.
		NULL,                   // session-id, none used.
		NULL,                   // master-secret, none used.
		(const sbyte *)(mocana->_cacert_hostname), // expected common name in cert
		mocana->_cert_store );
	if( mocana->_connection_instance < OK ) {
		TCP_CLOSE_SOCKET( mocana->_socket );
		return TsStatusErrorInternalServerError;
	}

	// turn-off server certificate validation
	if( OK > SSL_setCertAndStatusCallback( mocana->_connection_instance, myCertStatusCallback ) ) {
		ts_status_debug("ts_security_connect: SSL_setCertAndStatusCallback failed, ignoring,...\n " );
	}

	// perform handshake (note this will require tick() on embedded systems)
	mstatus = SSL_negotiateConnection( mocana->_connection_instance );
	if( mstatus < OK ) {
		ts_status_debug( "*** mstatus = %d\n", mstatus );
		TCP_CLOSE_SOCKET( mocana->_socket );
		return TsStatusErrorInternalServerError;
	}

	return TsStatusOk;
}

static TsStatus_t ts_disconnect(TsSecurityRef_t security) {

	ts_status_trace("ts_security_disconnect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);

	SSL_closeConnection( mocana->_connection_instance );
	TCP_CLOSE_SOCKET( mocana->_socket );

	return TsStatusOk;
}

static TsStatus_t ts_read(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {

	ts_status_trace("ts_security_read\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);

	size_t received = 0;
	MSTATUS result = SSL_recv( mocana->_connection_instance, buffer, (sbyte4)(*buffer_size), &received, budget );
	switch( result ) {
	case ERR_TCP_WOULDBLOCK:
		return TsStatusOkReadPending;
	default:
		if( result < OK ) {
			return TsStatusErrorInternalServerError;
		}
		*buffer_size = received;
	}
	return TsStatusOk;
}

static TsStatus_t ts_write(TsSecurityRef_t security, const uint8_t * buffer, size_t * buffer_size, uint32_t budget) {

	ts_status_trace("ts_security_write\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	TsSecurityMocanaRef_t mocana = (TsSecurityMocanaRef_t) (security);

	int index = 0;
	while( index < *buffer_size ) {
		// TODO - handle alternate result, e.g., connection-reset, pending-write, etc.
		index += SSL_send( mocana->_connection_instance, (sbyte*)(buffer+index), (sbyte4)((*buffer_size)-index) );
	}
	return TsStatusOk;
}


#endif // TS_SECURITY_MOCANA