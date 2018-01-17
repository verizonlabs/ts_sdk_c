// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdbool.h>
#include <stdint.h>

#include "ts_platform.h"
#include "ts_profile.h"
#include "ts_security.h"
#include "ts_controller.h"

#include "mbedtls/config.h"
#include "mbedtls/debug.h"
#include "mbedtls/net.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

/* TODO - setup config
#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time       time
#define mbedtls_time_t     time_t
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif
*/

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

static int mbedtls_tcp_send(void *, const unsigned char *, size_t);
static int mbedtls_tcp_recv(void *, unsigned char *, size_t);
static void mbedtls_debug(void *, int, const char *, int, const char *);

TsSecurityVtable_t ts_security_mbedtls = {
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

typedef struct TsSecurityEmbed *TsSecurityEmbedRef_t;
typedef struct TsSecurityEmbed {

	// inheritance by encapsulation; must be the first
	// attribute in order to treat this struct as a
	// TsSecurity struct
	TsSecurity_t                _security;
	
	// mbed attributes
	mbedtls_entropy_context     _entropy;
	mbedtls_ctr_drbg_context    _ctr_drbg;
	mbedtls_ssl_context         _ssl;
	mbedtls_ssl_config          _ssl_config;
	mbedtls_x509_crt            _cacert;
	mbedtls_x509_crt            _clcert;
	mbedtls_pk_context          _clkey;
	bool                        _clcert_is_set;
	bool                        _clkey_is_set;
	char *                      _cacert_hostname;

	// profile attributes
	TsProfileRef_t              _profile;

} TsSecurityEmbed_t;

static TsStatus_t ts_create(TsSecurityRef_t *security) {
	
	ts_status_trace("ts_security_create: mbed\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);

	// first, allocate and initialize controller memory
	TsControllerRef_t controller;
	TsStatus_t status = ts_controller_create(&controller);
	if( status != TsStatusOk ) {
		return status;
	}

	// the, allocate and initialize security
	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (ts_platform_malloc(sizeof(TsSecurityEmbed_t)));
	*security = (TsSecurityRef_t)mbed;

	mbed->_security._controller = controller;
	mbed->_security._profile = NULL;
	mbed->_clcert_is_set = false;
	mbed->_clkey_is_set = false;
	mbed->_cacert_hostname = SSL_HOST;

	mbedtls_ssl_init(&(mbed->_ssl));
	mbedtls_ssl_config_init(&(mbed->_ssl_config));
	mbedtls_x509_crt_init(&(mbed->_cacert));
	mbedtls_x509_crt_init(&(mbed->_clcert));
	mbedtls_pk_init(&(mbed->_clkey));
	mbedtls_ctr_drbg_init(&(mbed->_ctr_drbg));
	mbedtls_entropy_init(&(mbed->_entropy));

	// initialize entropy
	{
		int error = mbedtls_ctr_drbg_seed(&(mbed->_ctr_drbg), mbedtls_entropy_func, &(mbed->_entropy), NULL, 0);
		if (error != 0) {
			ts_platform_free(mbed, sizeof(TsSecurityEmbed_t));
			return TsStatusErrorPreconditionFailed;
		}
		mbedtls_ssl_conf_rng(&(mbed->_ssl_config), mbedtls_ctr_drbg_random, &(mbed->_ctr_drbg));
	}

	// initialize ssl-config
	{
		// TODO - allow DTLS on config
		int error = mbedtls_ssl_config_defaults(&(mbed->_ssl_config),
			MBEDTLS_SSL_IS_CLIENT,
			MBEDTLS_SSL_TRANSPORT_STREAM,
			MBEDTLS_SSL_PRESET_DEFAULT);
		if (error != 0) {
			ts_platform_free(mbed, sizeof(TsSecurityEmbed_t));
			return TsStatusErrorPreconditionFailed;
		}
	}

	// initialize ssl-config debugging
	mbedtls_ssl_conf_dbg(&(mbed->_ssl_config), mbedtls_debug, mbed);
	switch (ts_status_get_level()) {
	default:
		mbedtls_debug_set_threshold(1);
		break;

	case TsStatusTrace:
		mbedtls_debug_set_threshold(4);
		break;
	}

	// default to no verification (setting the cacert will enable it again)
	mbedtls_ssl_conf_authmode(&(mbed->_ssl_config), MBEDTLS_SSL_VERIFY_NONE);

	// return success
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

	// then, free this memory
	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	mbedtls_ssl_free(&(mbed->_ssl));
	mbedtls_ssl_config_free(&(mbed->_ssl_config));
	mbedtls_x509_crt_free(&(mbed->_cacert));
	mbedtls_x509_crt_free(&(mbed->_clcert));
	mbedtls_pk_free(&(mbed->_clkey));
	mbedtls_ctr_drbg_free(&(mbed->_ctr_drbg));
	mbedtls_entropy_free(&(mbed->_entropy));

	ts_platform_free(mbed, sizeof(TsSecurityEmbed_t));

	return TsStatusOk;
}

static TsStatus_t ts_tick(TsSecurityRef_t security, uint32_t budget) {

	ts_status_trace("ts_security_tick\n");
	ts_platform_assert(ts_controller != NULL);

	// do nothing

	return ts_controller_tick(security->_controller, budget);
}

static TsStatus_t ts_set_server_cert_hostname(TsSecurityRef_t security, const char * hostname ) {

	ts_status_trace("ts_set_server_cert_hostname\n");
	ts_platform_assert(security != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);

	mbed->_cacert_hostname = (char*)hostname;

	return TsStatusOk;
}

static TsStatus_t ts_set_server_cert(TsSecurityRef_t security, const uint8_t *cacert, size_t cacert_size) {

	ts_status_trace("ts_security_set_server_cert\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	int status = mbedtls_x509_crt_parse_der(&(mbed->_cacert), (const unsigned char *) cacert, cacert_size);
	if (status != 0) {
		return TsStatusErrorPreconditionFailed;
	}
	mbedtls_ssl_conf_ca_chain(&(mbed->_ssl_config), &(mbed->_cacert), NULL);
	mbedtls_ssl_conf_authmode(&(mbed->_ssl_config), MBEDTLS_SSL_VERIFY_REQUIRED);

	return TsStatusOk;
}

static TsStatus_t ts_set_client_cert(TsSecurityRef_t security, const uint8_t *clcert, size_t clcert_size) {
	
	ts_status_trace("ts_security_set_client_cert\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	if( mbedtls_x509_crt_parse_der(&(mbed->_clcert), (const unsigned char *) clcert, clcert_size) != 0) {
		return TsStatusErrorPreconditionFailed;
	}

	mbed->_clcert_is_set = true;
	if( mbed->_clkey_is_set ) {
		if( mbedtls_ssl_conf_own_cert(&(mbed->_ssl_config), &(mbed->_clcert), &(mbed->_clkey) ) != 0) {
			return TsStatusErrorPreconditionFailed;
		}
	}

	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_set_client_key(TsSecurityRef_t security, const uint8_t *clkey, size_t key_size) {
	
	ts_status_trace("ts_security_set_client_key\n");
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	if( mbedtls_pk_parse_key(&(mbed->_clkey), clkey, key_size, NULL, 0) != 0) {
		return TsStatusErrorPreconditionFailed;
	}

	mbed->_clkey_is_set = true;
	if( mbed->_clcert_is_set ) {
		if( mbedtls_ssl_conf_own_cert(&(mbed->_ssl_config), &(mbed->_clcert), &(mbed->_clkey) ) != 0) {
			return TsStatusErrorPreconditionFailed;
		}
	}

	return TsStatusErrorNotImplemented;
}

static TsStatus_t ts_connect(TsSecurityRef_t security, TsAddress_t address) {

	ts_status_trace("ts_security_connect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security->_controller != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	mbedtls_ssl_context *ssl = &(mbed->_ssl);
	mbedtls_ssl_config *ssl_config = &(mbed->_ssl_config);

	char host[TS_ADDRESS_MAX_HOST_SIZE], port[TS_ADDRESS_MAX_PORT_SIZE];
	if (ts_address_parse(address, host, port) != TsStatusOk) {
		return TsStatusErrorPreconditionFailed;
	}
	// TODO - allow differentiate from TCP and UDP
	// TODO - allow non-block setup
	TsStatus_t status = ts_controller_connect(security->_controller, address);
	if (status != TsStatusOk) {
		return status;
	}
	if (mbedtls_ssl_setup(ssl, ssl_config) != 0) {
		ts_controller_disconnect(security->_controller);
		return TsStatusErrorPreconditionFailed;
	}
	if (mbedtls_ssl_set_hostname(ssl, mbed->_cacert_hostname) != 0) {
		ts_controller_disconnect(security->_controller);
		return TsStatusErrorPreconditionFailed;
	}
	mbedtls_ssl_set_bio(ssl, security, mbedtls_tcp_send, mbedtls_tcp_recv, NULL);

	// TODO - should be handled in tick?
	status = TsStatusOk;
	uint64_t timestamp = ts_platform_time();
	bool handshaking = true;
	do  {

		int error = mbedtls_ssl_handshake(ssl);
		switch (error) {
		case MBEDTLS_ERR_SSL_WANT_READ:
		case MBEDTLS_ERR_SSL_WANT_WRITE:

			// check timeout
			if (ts_platform_time() - timestamp > SSL_HANDSHAKE_TIMEOUT) {
				status = TsStatusErrorExceedTimeBudget;
				ts_controller_disconnect(security->_controller);
				handshaking = false;
			}
			break;

		default:

			ts_status_debug("ts_security_connect: error, '%d'\n", error);
			status = TsStatusErrorPreconditionFailed;
			ts_controller_disconnect(security->_controller);
			// fallthrough

		case 0:

			handshaking = false;
			break;
		}
	} while( handshaking );

	/* TODO - verify server x.509?
	if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
	{
		char vrfy_buf[512];
		mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
		ts_status_debug( "failed cert verification, %s\n", vrfy_buf );
		status = TsStatusErrorPreconditionFailed;
		ts_controller_disconnect(security);
	}
	*/

	return status;
}

static TsStatus_t ts_disconnect(TsSecurityRef_t security) {

	ts_status_trace("ts_security_disconnect\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security->_controller != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	mbedtls_ssl_context *ssl = &(mbed->_ssl);
	mbedtls_ssl_session_reset(ssl);

	return ts_controller_disconnect(security->_controller);
}

static TsStatus_t ts_read(TsSecurityRef_t security, const uint8_t *buffer, size_t *buffer_size, uint32_t budget) {

	ts_status_trace("ts_security_read\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);
	ts_platform_assert(buffer != NULL);
	ts_platform_assert(buffer_size != NULL);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	mbedtls_ssl_context *ssl = &(mbed->_ssl);
	if( budget != SSL_READ_BUDGET ) {
		ts_status_trace( "ts_security_read: mbedtls timer budget cannot be dynamic, use SSL_READ_BUDGET instead.\n");
	}

	TsStatus_t status = TsStatusOk;
	bool reading = true;
	int index = 0;
	do {

		/**
		 * \brief          Read at most 'len' application data bytes
		 *
		 * \param ssl      SSL context
		 * \param buf      buffer that will hold the data
		 * \param len      maximum number of bytes to read
		 *
		 * \return         the number of bytes read, or
		 *                 0 for EOF, or
		 *                 MBEDTLS_ERR_SSL_WANT_READ or MBEDTLS_ERR_SSL_WANT_WRITE, or
		 *                 MBEDTLS_ERR_SSL_CLIENT_RECONNECT (see below), or
		 *                 another negative error code.
		 *
		 * \note           If this function returns something other than a positive
		 *                 value or MBEDTLS_ERR_SSL_WANT_READ/WRITE or
		 *                 MBEDTLS_ERR_SSL_CLIENT_RECONNECT, then the ssl context
		 *                 becomes unusable, and you should either free it or call
		 *                 \c mbedtls_ssl_session_reset() on it before re-using it for
		 *                 a new security; the current security must be closed.
		 *
		 * \note           When this function return MBEDTLS_ERR_SSL_CLIENT_RECONNECT
		 *                 (which can only happen server-side), it means that a client
		 *                 is initiating a new security using the same source port.
		 *                 You can either treat that as a security close and wait
		 *                 for the client to resend a ClientHello, or directly
		 *                 continue with \c mbedtls_ssl_handshake() with the same
		 *                 context (as it has beeen reset internally). Either way, you
		 *                 should make sure this is seen by the application as a new
		 *                 security: application state, if any, should be reset, and
		 *                 most importantly the identity of the client must be checked
		 *                 again. WARNING: not validating the identity of the client
		 *                 again, or not transmitting the new identity to the
		 *                 application layer, would allow authentication bypass!
		 */
		int size = mbedtls_ssl_read(ssl, (unsigned char *)(buffer + index), *buffer_size - index);
		switch (size) {
		default:

			if( size < 0 ) {
				ts_status_alarm( "ts_security_read: unexpected mbedtls error, %d\n", size );

				size = 0;
				reading = false;
				status = TsStatusErrorInternalServerError;
			}
			break;

		case MBEDTLS_ERR_SSL_CLIENT_RECONNECT:

			// TODO - handle reconnect
			ts_status_alarm( "ts_security_read: a client reconnect error occurred\n" );
			// fallthrough

		case MBEDTLS_ERR_SSL_WANT_READ:
		case MBEDTLS_ERR_SSL_WANT_WRITE:

			size = 0;
			reading = false;
			if( index > 0 ) {
				status = TsStatusOk;
			} else {
				status = TsStatusReadPending;
			}
			break;

		case 0:

			reading = false;
			break;
		}

		index = index + size;
		if (index >= *buffer_size) {
			reading = false;
		}
	} while( reading );

	*buffer_size = (size_t) index;
	return status;
}

static TsStatus_t ts_write(TsSecurityRef_t security, const uint8_t *buffer, size_t *buffer_size, uint32_t budget) {

	ts_status_trace("ts_security_write\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(security != NULL);
	ts_platform_assert(security->_controller != NULL);
	ts_platform_assert(buffer != NULL);
	ts_platform_assert(buffer_size > 0);

	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	mbedtls_ssl_context *ssl = &(mbed->_ssl);
	if( budget != SSL_WRITE_BUDGET ) {
		ts_status_trace( "ts_security_write: mbedtls timer budget cannot be dynamic, use SSL_WRITE_BUDGET instead.\n");
	}

	/**
	 * \brief          Try to write exactly 'len' application data bytes
	 *
	 * \warning        This function will do partial writes in some cases. If the
	 *                 return value is non-negative but less than length, the
	 *                 function must be called again with updated arguments:
	 *                 buf + ret, len - ret (if ret is the return value) until
	 *                 it returns a value equal to the last 'len' argument.
	 *
	 * \param ssl      SSL context
	 * \param buf      buffer holding the data
	 * \param len      how many bytes must be written
	 *
	 * \return         the number of bytes actually written (may be less than len),
	 *                 or MBEDTLS_ERR_SSL_WANT_WRITE or MBEDTLS_ERR_SSL_WANT_READ,
	 *                 or another negative error code.
	 *
	 * \note           If this function returns something other than a positive
	 *                 value or MBEDTLS_ERR_SSL_WANT_READ/WRITE, the ssl context
	 *                 becomes unusable, and you should either free it or call
	 *                 \c mbedtls_ssl_session_reset() on it before re-using it for
	 *                 a new security; the current security must be closed.
	 *
	 * \note           When this function returns MBEDTLS_ERR_SSL_WANT_WRITE/READ,
	 *                 it must be called later with the *same* arguments,
	 *                 until it returns a positive value.
	 *
	 * \note           If the requested length is greater than the maximum
	 *                 fragment length (either the built-in limit or the one set
	 *                 or negotiated with the peer), then:
	 *                 - with TLS, less bytes than requested are written.
	 *                 - with DTLS, MBEDTLS_ERR_SSL_BAD_INPUT_DATA is returned.
	 *                 \c mbedtls_ssl_get_max_frag_len() may be used to query the
	 *                 active maximum fragment length.
	 */
	TsStatus_t status = TsStatusOk;
	bool writing = true;
	int index = 0;
	do {

		int size = mbedtls_ssl_write(ssl, (buffer + index), *buffer_size - index);
		switch (size) {
		default:

			if( size < 0 ) {
				ts_status_alarm( "ts_security_write: unexpected mbedtls error, %d\n", size );

				size = 0;
				writing = false;
				status = TsStatusErrorInternalServerError;
			}
			break;

		case MBEDTLS_ERR_SSL_CLIENT_RECONNECT:

			// TODO - handle reconnect
			// fallthrough

		case MBEDTLS_ERR_SSL_WANT_READ:
		case MBEDTLS_ERR_SSL_WANT_WRITE:

			size = 0;
			break;

		case 0:

			writing = false;
			break;
		}

		index = index + size;
		if( index >= *buffer_size ) {
			writing = false;
		}
	} while( writing );

	*buffer_size = (size_t)index;
	return status;
}

/**
 * \brief          Set the debug callback
 *
 *                 The callback has the following argument:
 *                 void *           opaque context for the callback
 *                 int              debug level
 *                 const char *     file name
 *                 int              line number
 *                 const char *     message
 *
 * \param conf     SSL configuration
 * \param f_dbg    debug function
 * \param p_dbg    debug parameter
 */
static void mbedtls_debug(void *context, int level, const char *file, int line, const char *message) {
	ts_status_debug("%s:%04d: %s", file, line, message);
}

/**
 * \brief          Callback type: receive data from the network.
 *
 * \note           That callback may be either blocking or non-blocking.
 *
 * \param ctx      Context for the receive callback (typically a file
 *                 descriptor)
 * \param buf      Buffer to write the received data to
 * \param len      Length of the receive buffer
 *
 * \return         The callback must return the number of bytes received,
 *                 or a non-zero error code.
 *                 If performing non-blocking I/O, \c MBEDTLS_ERR_SSL_WANT_READ
 *                 must be returned when the operation would block.
 *
 * \note           The callback may receive fewer bytes than the length of the
 *                 buffer. It must always return the number of bytes actually
 *                 received and written to the buffer.
 */
static int mbedtls_tcp_recv(void *context, unsigned char *buffer, size_t buffer_size) {

	ts_status_trace( "mbedtls_tcp_recv\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(context != NULL);
	ts_platform_assert(buffer != NULL);

	int index = 0;
	bool reading = true;
	uint64_t timestamp = ts_platform_time();
	TsSecurityRef_t security = (TsSecurityRef_t)(context);
	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	do {

		size_t xbuffer_size = buffer_size - index;
		TsStatus_t status = ts_controller_read(security->_controller, (uint8_t *) (buffer + index), &xbuffer_size, SSL_READ_BUDGET);
		switch( status ) {
		default:
			ts_status_debug( "mbedtls_tcp_recv: %s\n", ts_status_string(status) );
			return MBEDTLS_ERR_NET_RECV_FAILED;

		case TsStatusErrorConnectionReset:
			ts_status_debug( "mbedtls_tcp_recv: %s\n", ts_status_string(status) );
			return MBEDTLS_ERR_NET_CONN_RESET;

		case TsStatusReadPending:
			if( index == 0 ) {
				return MBEDTLS_ERR_SSL_WANT_READ;
			} else {
				return index;
			}
			// fallthrough

		case TsStatusOk:
			break;
		}

		index = index + (int)xbuffer_size;
		if( index >= buffer_size || xbuffer_size == 0 ) {
			reading = false;
		} else if ( ts_platform_time() - timestamp > 60e6 ) {
			ts_status_alarm( "mbedtls_tcp_read: timeout occurred\n" );
			reading = false;
		}

	} while( reading );
	return index;
}

/**
 * \brief          Callback type: send data on the network.
 *
 * \note           That callback may be either blocking or non-blocking.
 *
 * \param ctx      Context for the send callback (typically a file descriptor)
 * \param buf      Buffer holding the data to send
 * \param len      Length of the data to send
 *
 * \return         The callback must return the number of bytes sent if any,
 *                 or a non-zero error code.
 *                 If performing non-blocking I/O, \c MBEDTLS_ERR_SSL_WANT_WRITE
 *                 must be returned when the operation would block.
 *
 * \note           The callback is allowed to send fewer bytes than requested.
 *                 It must always return the number of bytes actually sent.
 */
static int mbedtls_tcp_send(void *context, const unsigned char *buffer, size_t buffer_size) {

	ts_status_trace( "mbedtls_tcp_send\n");
	ts_platform_assert(ts_controller != NULL);
	ts_platform_assert(context != NULL);
	ts_platform_assert(buffer != NULL);

	int index = 0;
	bool writing = true;
	uint64_t timestamp = ts_platform_time();
	TsSecurityRef_t security = (TsSecurityRef_t)(context);
	TsSecurityEmbedRef_t mbed = (TsSecurityEmbedRef_t) (security);
	do {

		size_t xbuffer_size = buffer_size - index;
		TsStatus_t status = ts_controller_write(security->_controller, (uint8_t *) (buffer + index), &xbuffer_size, SSL_WRITE_BUDGET);
		switch( status ) {
		default:
			ts_status_debug( "mbedtls_tcp_send: %s\n", ts_status_string(status) );
			return MBEDTLS_ERR_NET_RECV_FAILED;

		case TsStatusErrorConnectionReset:
			ts_status_debug( "mbedtls_tcp_send: %s\n", ts_status_string(status) );
			return MBEDTLS_ERR_NET_CONN_RESET;

		case TsStatusWritePending:
			if( index == 0 ) {
				return MBEDTLS_ERR_SSL_WANT_WRITE;
			} else {
				return index;
			}

		case TsStatusOk:
			break;
		}

		index = index + (int)xbuffer_size;
		if ( index >= buffer_size ) {
			writing = false;
		} else if ( xbuffer_size == 0 ) {
			ts_status_alarm( "mbedtls_tcp_send: zero sized write\n" );
			writing = false;
		} else if ( ts_platform_time() - timestamp > 60e6 ) {
			ts_status_alarm( "mbedtls_tcp_send: timeout occurred\n" );
			writing = false;
		}

	} while( writing );
	return index;
}

/**
 * \brief           Entropy poll callback for a hardware source
 *
 * \warning         This is not provided by mbed TLS!
 *                  See \c MBEDTLS_ENTROPY_HARDWARE_ALT in config.h.
 *
 * \note            This must accept NULL as its first argument.
 */
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {

	ts_status_trace( "mbedtls_hardware_poll\n");
	size_t i, j;
	uint32_t number;
	((void) data);
	for (i = 0; i < len;) {
		ts_platform->random(&number);
		for (j = 0; j < sizeof(number) && i < len;) {
			*(output + i) = (unsigned char) (number & 0xff);
			number >>= 8;
			i++;
			j++;
		}
	}
	*olen = len;
	return 0;
}
