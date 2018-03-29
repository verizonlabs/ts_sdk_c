// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_controller.h"
#include "at_interpreter/at_intfc.h"
#include "cmd_urc_monarch.h"

#define mdbg( ... )                 ts_status_debug(__VA_ARGS__)

#define DEFAULT_ATTEMPTS            5
#define IDLE_TIME_MS                5
#define NET_REG_TIMEOUT_MS          20000
#define STARTUP_TIMEOUT_MS          5000

#define ts_platform_time_ms()       (ts_platform_time() / TS_TIME_MSEC_TO_USEC)
#define ts_platform_sleep_ms( t )   ts_platform_sleep((t) * TS_TIME_MSEC_TO_USEC)
#define MSEC2USEC( t )              ((t) * TS_TIME_MSEC_TO_USEC)

static TsStatus_t ts_create( TsControllerRef_t * );
static TsStatus_t ts_destroy( TsControllerRef_t );
static TsStatus_t ts_tick( TsControllerRef_t, uint32_t );

static TsStatus_t ts_connect( TsControllerRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsControllerRef_t );
static TsStatus_t ts_read( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_write( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );

static TsStatus_t ts_get_id( TsControllerRef_t, char * );
static TsStatus_t ts_get_rssi( TsControllerRef_t, char * );
static TsStatus_t ts_get_ipv4_addr( TsControllerRef_t, char * );
static TsStatus_t ts_get_iccid( TsControllerRef_t, char * );
static TsStatus_t ts_get_date_and_time( TsControllerRef_t, char * );
static TsStatus_t ts_get_imsi( TsControllerRef_t, char * );
static TsStatus_t ts_get_manufacturer( TsControllerRef_t, char * );
static TsStatus_t ts_get_module_name( TsControllerRef_t, char * );
static TsStatus_t ts_get_firmware_version( TsControllerRef_t, char * );

TsControllerVtable_t ts_controller_monarch = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,

	.get_id = ts_get_id,
	.get_rssi = ts_get_rssi,
	.get_ipv4_addr = ts_get_ipv4_addr,
	.get_iccid = ts_get_iccid,
	.get_date_and_time = ts_get_date_and_time,
	.get_imsi = ts_get_imsi,
	.get_manufacturer = ts_get_manufacturer,
	.get_module_name = ts_get_module_name,
	.get_firmware_version = ts_get_firmware_version
};

/**
 * \brief Type describing the status of network registration.
 */
typedef enum {
	MODEM_NO_NET = 0,    /**< No network was found. */
	MODEM_SEARCH_NET = 2,    /**< Looking for network. */
	MODEM_ATTACHED = 1    /**< Network found and registered. */
} net_reg_stat;

typedef struct {
	size_t sz;
	char * data;
} array_t;

#define SIZEOF_IMSI             TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_ICCID            TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_DATE_AND_TIME    TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_IPV4             TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_RSSI             TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_TEXT_INFO        TS_DRIVER_MAX_ID_SIZE
#define SIZEOF_IMEI             15

typedef struct TsControllerMonarch * TsControllerMonarchRef_t;
typedef struct TsControllerMonarch {

	TsController_t _controller;
	bool being_used;
	bool modem_started;
	net_reg_stat reg_to_net;
	bool tcp_connected;
	bool tcp_peer_close;
	size_t unread_tcp;

	/* Diagnostics information */
	char imei[SIZEOF_IMEI + 1];
	char rssi[SIZEOF_RSSI + 1];
	char ipv4_addr[SIZEOF_IPV4 + 1];
	char iccid[SIZEOF_ICCID + 1];
	char date_and_time[SIZEOF_DATE_AND_TIME + 1];
	char imsi[SIZEOF_IMSI + 1];
	char manufacturer[SIZEOF_TEXT_INFO + 1];
	char module_name[SIZEOF_TEXT_INFO + 1];
	char firmware_version[SIZEOF_TEXT_INFO + 1];

} TsControllerMonarch_t;

// XXX: For now, only one instance of the controller is possible.
// This needs to be configurable in a future release.
static TsControllerMonarch_t monarch;

static void sys_start( size_t sz, const char urc_text[] ) {
	monarch.modem_started = true;
}

static void parse_imei( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t imei_idx = 2;
	memcpy( pvt_data, resp + imei_idx, SIZEOF_IMEI );
}

static void parse_rssi( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t rssi_idx = 8;
	uint8_t i = rssi_idx;
	array_t * rssi = pvt_data;
	rssi->sz = 0;
	while( i < sz && resp[ i ] != ',' )
		rssi->data[ ( rssi->sz )++ ] = resp[ i++ ];
}

static void parse_ipv4_addr( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t ipv4_idx = 15;
	uint8_t i = ipv4_idx;
	array_t * ip = pvt_data;
	ip->sz = 0;
	while( i < sz && resp[ i ] != '"' )
		ip->data[ ( ip->sz )++ ] = resp[ i++ ];
}

static void parse_iccid( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t iccid_idx = 13;
	memcpy( pvt_data, resp + iccid_idx, SIZEOF_ICCID );
}

static void parse_date_and_time( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t date_and_time_idx = 10;
	memcpy( pvt_data, resp + date_and_time_idx, SIZEOF_DATE_AND_TIME );
}

static void parse_imsi( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t imsi_idx = 2;
	memcpy( pvt_data, resp + imsi_idx, SIZEOF_IMSI );
}

static void parse_module_name( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t mod_idx = 2;
	uint8_t i = mod_idx;
	array_t * mod = pvt_data;
	mod->sz = 0;
	while( i < sz && resp[ i ] != '\r' )
		mod->data[ ( mod->sz )++ ] = resp[ i++ ];
}

static void parse_manufacturer( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t man_idx = 2;
	uint8_t i = man_idx;
	array_t * man = pvt_data;
	man->sz = 0;
	while( i < sz && resp[ i ] != '\r' )
		man->data[ ( man->sz )++ ] = resp[ i++ ];
}

static void parse_firmware_version( size_t sz, const char resp[], void * pvt_data ) {
	const uint8_t fw_idx = 2;
	uint8_t i = fw_idx;
	array_t * fw = pvt_data;
	fw->sz = 0;
	while( i < sz && resp[ i ] != '\r' )
		fw->data[ ( fw->sz )++ ] = resp[ i++ ];
}

#define TCP_RECV_TIMEOUT_MS            5000
#define TCP_RECV_TRAIL_SZ            8
static void parse_tcp_data( size_t sz, const char resp[], void * pvt_data ) {
	array_t * bytes = pvt_data;
	size_t data_sz = bytes->sz;

	uint64_t start = ts_platform_time_ms();
	bool timeout = true;
	do {
		if( at_avail_bytes() >= data_sz + TCP_RECV_TRAIL_SZ ) {
			timeout = false;
			break;
		}
	} while(ts_platform_time_ms() - start <= TCP_RECV_TIMEOUT_MS );

	if( timeout ) {
		bytes->sz = 0;
		return;
	}

	if( at_read_bytes( data_sz, (uint8_t *) bytes->data ) != data_sz ) {
		bytes->sz = 0;
		return;
	}

	monarch.unread_tcp -= data_sz;

	/* Read the last 8 bytes: \r\n\r\nOK\r\n\. */
	at_discard_begin_bytes( TCP_RECV_TRAIL_SZ );
}

static void parse_cereg_urc( size_t sz, const char urc_text[] ) {
	const uint8_t stat_idx = 10;
	uint8_t net_stat = urc_text[ stat_idx ] - '0';
	if( net_stat > MODEM_SEARCH_NET )
		return;
	monarch.reg_to_net = net_stat;
}

static void tcp_conn_closed( size_t sz, const char urc_text[] ) {
	monarch.tcp_connected = false;
	monarch.tcp_peer_close = true;
	mdbg( "Peer closed connection\n" );
}

static void tcp_recv_bytes( size_t sz, const char urc_text[] ) {
	uint8_t num_idx = 15;
	size_t num_unread = 0;
	while( urc_text[ num_idx ] != '\r' ) {
		num_unread = num_unread*10 + urc_text[ num_idx ] - '0';
		num_idx++;
	}
	monarch.unread_tcp += num_unread;
}

static bool modem_get_imei( TsControllerMonarchRef_t modem ) {
	query_cmd_list[ QUERY_IMEI ].resp[ 0 ].pvt_data = modem->imei;
	if( at_wcmd( &query_cmd_list[ QUERY_IMEI ] ) != AT_WCMD_OK )
		return false;
	modem->imei[ SIZEOF_IMEI ] = '\0';
	return true;
}

static bool modem_get_rssi( TsControllerMonarchRef_t modem ) {
	array_t rssi_data = { 0, modem->rssi };
	query_cmd_list[ QUERY_RSSI ].resp[ 0 ].pvt_data = &rssi_data;
	if( at_wcmd( &query_cmd_list[ QUERY_RSSI ] ) != AT_WCMD_OK )
		return false;
	modem->rssi[ rssi_data.sz ] = '\0';
	return true;
}

static bool modem_get_ipv4_addr( TsControllerMonarchRef_t modem ) {
	array_t ip_data = { 0, modem->ipv4_addr };
	query_cmd_list[ QUERY_IPV4_ADDR ].resp[ 0 ].pvt_data = &ip_data;
	if( at_wcmd( &query_cmd_list[ QUERY_IPV4_ADDR ] ) != AT_WCMD_OK )
		return false;
	modem->ipv4_addr[ ip_data.sz ] = '\0';
	return true;
}

static bool modem_get_iccid( TsControllerMonarchRef_t modem ) {
	query_cmd_list[ QUERY_ICCID ].resp[ 0 ].pvt_data = modem->iccid;
	if( at_wcmd( &query_cmd_list[ QUERY_ICCID ] ) != AT_WCMD_OK )
		return false;
	modem->iccid[ SIZEOF_ICCID ] = '\0';
	return true;
}

static bool modem_get_date_and_time( TsControllerMonarchRef_t modem ) {
	query_cmd_list[ QUERY_DATE_AND_TIME ].resp[ 0 ].pvt_data = modem->date_and_time;
	if( at_wcmd( &query_cmd_list[ QUERY_DATE_AND_TIME ] ) != AT_WCMD_OK )
		return false;
	modem->date_and_time[ SIZEOF_DATE_AND_TIME ] = '\0';
	return true;
}

static bool modem_get_imsi( TsControllerMonarchRef_t modem ) {
	query_cmd_list[ QUERY_IMSI ].resp[ 0 ].pvt_data = modem->imsi;
	if( at_wcmd( &query_cmd_list[ QUERY_IMSI ] ) != AT_WCMD_OK )
		return false;
	modem->imsi[ SIZEOF_IMSI ] = '\0';
	return true;
}

static bool modem_get_manufacturer( TsControllerMonarchRef_t modem ) {
	array_t man_data = { 0, modem->manufacturer };
	query_cmd_list[ QUERY_MANUFACTURER ].resp[ 0 ].pvt_data = &man_data;
	if( at_wcmd( &query_cmd_list[ QUERY_MANUFACTURER ] ) != AT_WCMD_OK )
		return false;
	modem->manufacturer[ man_data.sz ] = '\0';
	return true;
}

static bool modem_get_module_name( TsControllerMonarchRef_t modem ) {
	array_t mod_data = { 0, modem->module_name };
	query_cmd_list[ QUERY_MODULE_NAME ].resp[ 0 ].pvt_data = &mod_data;
	if( at_wcmd( &query_cmd_list[ QUERY_MODULE_NAME ] ) != AT_WCMD_OK )
		return false;
	modem->module_name[ mod_data.sz ] = '\0';
	return true;
}

static bool modem_get_firmware_version( TsControllerMonarchRef_t modem ) {
	array_t fwver_data = { 0, modem->firmware_version };
	query_cmd_list[ QUERY_FIRMWARE_VERSION ].resp[ 0 ].pvt_data = &fwver_data;
	if( at_wcmd( &query_cmd_list[ QUERY_FIRMWARE_VERSION ] ) != AT_WCMD_OK )
		return false;
	modem->firmware_version[ fwver_data.sz ] = '\0';
	return true;
}

static bool wait_for_net_reg( TsControllerMonarchRef_t m, uint32_t timeout_ms ) {
	uint64_t start = ts_platform_time_ms();
	while( m->reg_to_net != MODEM_ATTACHED ) {
		if( ts_platform_time_ms() - start > timeout_ms ) {
			mdbg( "Timed out waiting for network attach\n" );
			return false;
		}
		at_intfc_service();
		ts_platform_sleep_ms( IDLE_TIME_MS );
	}
	return true;
}

static bool tcp_init( void ) {
	ts_platform_sleep_ms( 1000 );
	if( at_wcmd( &tcp_cmd_list[ PDP_ACT ] ) != AT_WCMD_OK )
		return false;
	if( at_wcmd( &tcp_cmd_list[ SOCK_CONF ] ) != AT_WCMD_OK )
		return false;
	if( at_wcmd( &tcp_cmd_list[ SOCK_CONF_EXT ] ) != AT_WCMD_OK )
		return false;
	return true;
}

static bool process_startup_urcs( TsControllerMonarchRef_t m ) {
	mdbg( "Waiting for modem start URC\n" );
	uint64_t start = ts_platform_time_ms();
	while( !m->modem_started ) {
		if( ts_platform_time_ms() - start > STARTUP_TIMEOUT_MS ) {
			mdbg( "Timed out waiting for modem to start\n" );
			return false;
		}
		at_intfc_service();
		ts_platform_sleep_ms( IDLE_TIME_MS );
	}
	return true;
}

static bool modem_hardware_reset( TsControllerMonarchRef_t m ) {
	m->modem_started = false;
	at_clear_rxbuf();
	ts_driver_reset( m->_controller._driver );
	return process_startup_urcs( m );
}

static bool initialize_diag_table( TsControllerMonarchRef_t modem ) {
	if( !modem_get_imei( modem ))
		return false;
	if( !modem_get_rssi( modem ))
		return false;
	if( !modem_get_ipv4_addr( modem ))
		return false;
	if( !modem_get_iccid( modem ))
		return false;
	if( !modem_get_date_and_time( modem ))
		return false;
	if( !modem_get_imsi( modem ))
		return false;
	if( !modem_get_manufacturer( modem ))
		return false;
	if( !modem_get_module_name( modem ))
		return false;
	if( !modem_get_firmware_version( modem ))
		return false;
	return true;
}

void debug_diag( TsControllerMonarchRef_t modem ) {
	mdbg( "IMEI: %s\n", modem->imei );
	mdbg( "RSSI: %s\n", modem->rssi );
	mdbg( "IPV4: %s\n", modem->ipv4_addr );
	mdbg( "ICCID: %s\n", modem->iccid );
	mdbg( "Time & Timezone: %s\n", modem->date_and_time );
	mdbg( "IMSI: %s\n", modem->imsi );
	mdbg( "Manufacturer: %s\n", modem->manufacturer );
	mdbg( "Module: %s\n", modem->module_name );
	mdbg( "Modem firmware version: %s\n", modem->firmware_version );
}

static TsStatus_t ts_create( TsControllerRef_t * controller ) {
	ts_status_trace( "ts_controller_create: monarch\n" );

	*controller = (TsControllerRef_t) &monarch;
	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) ( *controller );
	if( controller_monarch->being_used ) {
		*controller = NULL;
		return TsStatusErrorNoResourceAvailable;
	}

	controller_monarch->modem_started = false;
	controller_monarch->reg_to_net = MODEM_NO_NET;
	controller_monarch->tcp_connected = false;
	controller_monarch->tcp_peer_close = false;
	controller_monarch->unread_tcp = 0;

	TsDriverRef_t driver;
	if( ts_driver_create( &driver ) != TsStatusOk )
		return TsStatusErrorInternalServerError;
	( *controller )->_driver = driver;

	mdbg( "Begin initialization of modem\n" );
	if( !at_init( driver ))
		return TsStatusErrorInternalServerError;

	at_reg_urcs( NUM_URCS, urc_list );

	mdbg( "Reset the modem hardware\n" );
	modem_hardware_reset( controller_monarch );

	if( at_wcmd( &core_cmd_list[ EPS_URC_SET ] ) != AT_WCMD_OK )
		return TsStatusErrorInternalServerError;

	if( at_wcmd( &core_cmd_list[ EN_CELL_FUNC ] ) != AT_WCMD_OK )
		return TsStatusErrorInternalServerError;

	mdbg( "Checking for network registration\n" );
	if( !wait_for_net_reg( controller_monarch, NET_REG_TIMEOUT_MS ))
		return TsStatusErrorInternalServerError;

	if( at_wcmd( &core_cmd_list[ SIM_READY ] ) != AT_WCMD_OK )
		return TsStatusErrorInternalServerError;

	mdbg( "Initializing TCP layer\n" );
	if( !tcp_init()) {
		mdbg( "Failed to initialize the TCP layer\n" );
		return TsStatusErrorInternalServerError;
	}

	mdbg( "Retrieve modem diagnostics information\n" );
	if( !initialize_diag_table( controller_monarch )) {
		mdbg( "Failed to populate diagnostics information\n" );
		return TsStatusErrorInternalServerError;
	}

	debug_diag( controller_monarch );

	strncpy((char *) ( *controller )->_driver->_spec_id, controller_monarch->imei, SIZEOF_IMEI );

	mdbg( "Modem initialized\n" );
	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsControllerRef_t controller ) {
	ts_status_trace( "ts_controller_destroy\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) controller;
	controller_monarch->being_used = false;
	ts_driver_destroy( controller->_driver );
	return TsStatusOk;
}

static TsStatus_t ts_tick( TsControllerRef_t controller, uint32_t budget ) {
	ts_status_trace( "ts_controller_tick\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	at_intfc_service();

	return TsStatusOk;
}

#define MAX_TCP_HOST_PORT_NAME    (TS_ADDRESS_MAX_HOST_SIZE + TS_ADDRESS_MAX_PORT_SIZE + 30)
static TsStatus_t ts_connect( TsControllerRef_t controller, TsAddress_t address ) {
	ts_status_trace( "ts_controller_connect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) controller;
	char host[TS_ADDRESS_MAX_HOST_SIZE];
	char port[TS_ADDRESS_MAX_PORT_SIZE];
	if( ts_address_parse( address, host, port ) != TsStatusOk )
		return TsStatusErrorInternalServerError;

	if( controller_monarch->tcp_connected )
		return TsStatusErrorNoResourceAvailable;

	char cmd[MAX_TCP_HOST_PORT_NAME];
	at_cmd_desc * tcp_conn = &tcp_cmd_list[ SOCK_DIAL ];
	snprintf( cmd, sizeof( cmd ), tcp_conn->cmd_fmt, port, host );
	tcp_conn->cmd = cmd;
	if( at_wcmd( tcp_conn ) != AT_WCMD_OK )
		return TsStatusErrorInternalServerError;

	controller_monarch->tcp_connected = true;
	controller_monarch->tcp_peer_close = false;

	mdbg( "TCP connection established\n" );

	return TsStatusOk;
}

static TsStatus_t ts_disconnect( TsControllerRef_t controller ) {
	ts_status_trace( "ts_controller_disconnect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) controller;
	if( !controller_monarch->tcp_connected ) {
		mdbg( "TCP connection already closed\n" );
		return TsStatusErrorNoResourceAvailable;
	}

	mdbg( "Closing TCP connection\n" );
	if( at_wcmd( &tcp_cmd_list[ SOCK_CLOSE ] ) != AT_WCMD_OK ) {
		mdbg( "Failed to close TCP connection\n" );
		return TsStatusErrorInternalServerError;
	}
	controller_monarch->tcp_connected = false;
	controller_monarch->tcp_peer_close = false;
	controller_monarch->unread_tcp = 0;
	return TsStatusOk;
}

#define MAX_TCP_DATA_LEN            1500
#define MAX_TCP_RECV_CMD_LEN            32
static TsStatus_t ts_read( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_read\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) controller;

	if( controller_monarch->unread_tcp == 0 ) {
		if( controller_monarch->tcp_connected && controller_monarch->reg_to_net )
			return TsStatusOkReadPending;
		mdbg( "Attempting to read when not connected to TCP\n" );
		return TsStatusErrorConnectionReset;
	}

	if( *buffer_size > MAX_TCP_DATA_LEN )
		*buffer_size = MAX_TCP_DATA_LEN;

	if( *buffer_size > AT_BUF_SZ)
		*buffer_size = AT_BUF_SZ;

	if( *buffer_size > controller_monarch->unread_tcp )
		*buffer_size = controller_monarch->unread_tcp;

	char cmd[MAX_TCP_RECV_CMD_LEN];
	at_cmd_desc * tcp_recv = &tcp_cmd_list[ SOCK_RECV ];
	snprintf( cmd, sizeof( cmd ), tcp_recv->cmd_fmt, *buffer_size );
	tcp_recv->cmd = cmd;
	array_t bytes = { *buffer_size, (char *) buffer };
	tcp_recv->resp[ 0 ].pvt_data = &bytes;

	if( at_wcmd( tcp_recv ) != AT_WCMD_OK ) {
		mdbg( "TCP read error\n" );
		return TsStatusErrorInternalServerError;
	}

	if( bytes.sz == 0 ) {
		mdbg( "TCP read error\n" );
		return TsStatusErrorInternalServerError;
	}

	return TsStatusOk;
}

#define MAX_TCP_DATA_LEN            1500
#define MAX_TCP_SEND_CMD_LEN            32
static TsStatus_t ts_write( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_write\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t) controller;

	if( !controller_monarch->tcp_connected || !controller_monarch->reg_to_net ) {
		mdbg( "Attempting to send bytes when not connected to TCP\n" );
		if( !controller_monarch->tcp_peer_close )
			ts_platform_assert( false );
		else
			return TsStatusErrorConnectionReset;
	}

	if( *buffer_size > MAX_TCP_DATA_LEN )
		*buffer_size = MAX_TCP_DATA_LEN;

	char cmd[MAX_TCP_SEND_CMD_LEN];
	at_cmd_desc * tcp_write = &tcp_cmd_list[ SOCK_SEND_CMD ];
	snprintf( cmd, sizeof( cmd ), tcp_write->cmd_fmt, *buffer_size );
	tcp_write->cmd = cmd;
	if( at_wcmd( tcp_write ) != AT_WCMD_OK ) {
		mdbg( "TCP write failed\n" );
		return TsStatusErrorInternalServerError;
	}

	tcp_write = &tcp_cmd_list[ SOCK_SEND_DATA ];
	tcp_write->cmd = (const char *) buffer;
	tcp_write->cmd_len = *buffer_size;
	if( at_wcmd( tcp_write ) != AT_WCMD_OK ) {
		mdbg( "TCP write failed\n" );
		return TsStatusErrorInternalServerError;
	}

	return TsStatusOk;
}

static TsStatus_t ts_get_id( TsControllerRef_t controller, char * id ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( id, modem->imei, SIZEOF_IMEI );
	return TsStatusOk;
}

static TsStatus_t ts_get_rssi( TsControllerRef_t controller, char * rssi ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	if( !modem_get_rssi( modem ))
		return TsStatusErrorInternalServerError;
	strncpy( rssi, modem->rssi, SIZEOF_RSSI );
	return TsStatusOk;
}

static TsStatus_t ts_get_ipv4_addr( TsControllerRef_t controller, char * ipv4_addr ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	if( !modem_get_ipv4_addr( modem ))
		return TsStatusErrorInternalServerError;
	strncpy( ipv4_addr, modem->ipv4_addr, SIZEOF_IPV4 );
	return TsStatusOk;
}

static TsStatus_t ts_get_iccid( TsControllerRef_t controller, char * iccid ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( iccid, modem->iccid, SIZEOF_ICCID );
	return TsStatusOk;
}

static TsStatus_t ts_get_date_and_time( TsControllerRef_t controller, char * date_and_time ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	if( !modem_get_date_and_time( modem ))
		return TsStatusErrorInternalServerError;
	strncpy( date_and_time, modem->date_and_time, SIZEOF_DATE_AND_TIME );
	return TsStatusOk;
}

static TsStatus_t ts_get_imsi( TsControllerRef_t controller, char * imsi ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( imsi, modem->imsi, SIZEOF_IMSI );
	return TsStatusOk;
}

static TsStatus_t ts_get_manufacturer( TsControllerRef_t controller, char * manufacturer ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( manufacturer, modem->manufacturer, SIZEOF_TEXT_INFO );
	return TsStatusOk;
}

static TsStatus_t ts_get_module_name( TsControllerRef_t controller, char * module_name ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( module_name, modem->module_name, SIZEOF_TEXT_INFO );
	return TsStatusOk;
}

static TsStatus_t ts_get_firmware_version( TsControllerRef_t controller, char * firmware_version ) {
	TsControllerMonarchRef_t modem = (TsControllerMonarchRef_t) controller;
	strncpy( firmware_version, modem->firmware_version, SIZEOF_TEXT_INFO );
	return TsStatusOk;
}
