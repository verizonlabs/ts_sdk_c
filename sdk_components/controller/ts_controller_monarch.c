// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_controller.h"
#include "at_intfc.h"
#include "cmd_urc_monarch.h"

#define mdbg(...)			ts_status_debug(__VA_ARGS__)

#define DEFAULT_ATTEMPTS		5
#define IDLE_TIME_MS			5
#define NET_REG_TIMEOUT_MS		20000
#define STARTUP_TIMEOUT_MS		5000

#define ts_platform_time_ms()	(ts_platform_time() / TS_TIME_MSEC_TO_USEC)
#define ts_platform_sleep_ms(t)	ts_platform_sleep((t) * TS_TIME_MSEC_TO_USEC)
#define MSEC2USEC(t)		((t) * TS_TIME_MSEC_TO_USEC)

/**
 * \brief Size of IMEI string.
 */
#define SIZEOF_IMEI			15

/**
 * \brief Size of IMSI string.
 */
#define SIZEOF_IMSI			15

/**
 * \brief Size of ICCID string.
 */
#define SIZEOF_ICCID			20

/**
 * \brief Size of TTZ string.
 */
#define SIZEOF_TTZ			20

static TsStatus_t ts_create( TsControllerRef_t * );
static TsStatus_t ts_destroy( TsControllerRef_t );
static TsStatus_t ts_tick( TsControllerRef_t, uint32_t );

static TsStatus_t ts_connect( TsControllerRef_t, TsAddress_t );
static TsStatus_t ts_disconnect( TsControllerRef_t );
static TsStatus_t ts_read( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );
static TsStatus_t ts_write( TsControllerRef_t, const uint8_t *, size_t *, uint32_t );

TsControllerVtable_t ts_controller_monarch = {
	.create = ts_create,
	.destroy = ts_destroy,
	.tick = ts_tick,

	.connect = ts_connect,
	.disconnect = ts_disconnect,
	.read = ts_read,
	.write = ts_write,
};

/**
 * \brief Type describing the status of network registration.
 */
typedef enum {
	MODEM_NO_NET = 0,	/**< No network was found. */
	MODEM_SEARCH_NET = 2,	/**< Looking for network. */
	MODEM_ATTACHED = 1	/**< Network found and registered. */
} net_reg_stat;

typedef struct {
	size_t sz;
	char *data;
} array_t;

typedef struct TsControllerMonarch * TsControllerMonarchRef_t;
typedef struct TsControllerMonarch {
	TsController_t _controller;
	bool being_used;
	bool modem_started;
	net_reg_stat reg_to_net;
	bool tcp_connected;
	bool tcp_peer_close;
	size_t unread_tcp;
} TsControllerMonarch_t;

// XXX: For now, only one instance of the controller is possible.
// This needs to be configurable in a future release.
static TsControllerMonarch_t monarch;

static void sys_start(size_t sz, const char urc_text[])
{
	monarch.modem_started = true;
}

static void parse_imei(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t imei_idx = 2;
	memcpy(pvt_data, resp + imei_idx, SIZEOF_IMEI);
}

static void parse_sig_str(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t rssi_idx = 8;
	uint8_t i = rssi_idx;
	uint8_t *rssi = pvt_data;
	*rssi = 0;
	while (i < sz && resp[i] != ',')
		*rssi = *rssi * 10 + resp[i++] - '0';
}

static void parse_ip_addr(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t ipv4_idx = 15;
	uint8_t i = ipv4_idx;
	array_t *ip = pvt_data;
	ip->sz = 0;
	while (i < sz && resp[i] != '"')
		ip->data[(ip->sz)++] = resp[i++];
}

static void parse_iccid(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t iccid_idx = 13;
	memcpy(pvt_data, resp + iccid_idx, SIZEOF_ICCID);
}

static void parse_ttz(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t ttz_idx = 10;
	memcpy(pvt_data, resp + ttz_idx, SIZEOF_TTZ);
}

static void parse_imsi(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t imsi_idx = 2;
	memcpy(pvt_data, resp + imsi_idx, SIZEOF_IMSI);
}

static void parse_mod_info(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t mod_idx = 2;
	uint8_t i = mod_idx;
	array_t *mod = pvt_data;
	mod->sz = 0;
	while (i < sz && resp[i] != '\r')
		mod->data[(mod->sz)++] = resp[i++];
}

static void parse_man_info(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t man_idx = 2;
	uint8_t i = man_idx;
	array_t *man = pvt_data;
	man->sz = 0;
	while (i < sz && resp[i] != '\r')
		man->data[(man->sz)++] = resp[i++];
}

static void parse_fw_ver(size_t sz, const char resp[], void *pvt_data)
{
	const uint8_t fw_idx = 2;
	uint8_t i = fw_idx;
	array_t *fw = pvt_data;
	fw->sz = 0;
	while (i < sz && resp[i] != '\r')
		fw->data[(fw->sz)++] = resp[i++];
}

#define TCP_RECV_TIMEOUT_MS			5000
#define TCP_RECV_TRAIL_SZ			8
static void parse_tcp_data(size_t sz, const char resp[], void *pvt_data)
{
	array_t *bytes = pvt_data;
	size_t data_sz = bytes->sz;

	uint32_t start = ts_platform_time_ms();
	bool timeout = true;
	do {
		if (at_avail_bytes() >= data_sz + TCP_RECV_TRAIL_SZ) {
			timeout = false;
			break;
		}
	} while (ts_platform_time_ms() - start <= TCP_RECV_TIMEOUT_MS);

	if (timeout) {
		bytes->sz = 0;
		return;
	}

	if (at_read_bytes(data_sz, (uint8_t *)bytes->data) != data_sz) {
		bytes->sz = 0;
		return;
	}

	monarch.unread_tcp -= data_sz;

	/* Read the last 8 bytes: \r\n\r\nOK\r\n\. */
	at_discard_begin_bytes(TCP_RECV_TRAIL_SZ);
}

static void parse_cereg_urc(size_t sz, const char urc_text[])
{
	const uint8_t stat_idx = 10;
	uint8_t net_stat = urc_text[stat_idx] - '0';
	if (net_stat > MODEM_SEARCH_NET)
		return;
	monarch.reg_to_net = net_stat;
}

static void tcp_conn_closed(size_t sz, const char urc_text[])
{
	monarch.tcp_connected = false;
	monarch.tcp_peer_close = true;
}

static void tcp_recv_bytes(size_t sz, const char urc_text[])
{
	uint8_t num_idx = 15;
	size_t num_unread = 0;
	while (urc_text[num_idx] != '\r') {
		num_unread = num_unread * 10 + urc_text[num_idx] - '0';
		num_idx++;
	}
	monarch.unread_tcp += num_unread;
}

static bool wait_for_net_reg(TsControllerMonarchRef_t m, uint32_t timeout_ms)
{
	uint32_t start = ts_platform_time_ms();
	while (m->reg_to_net != MODEM_ATTACHED) {
		if (ts_platform_time_ms() - start > timeout_ms) {
			mdbg("%s:%d Timed out waiting for network attach\n",
					__func__, __LINE__);
			return false;
		}
		at_intfc_service();
		ts_platform_sleep_ms(IDLE_TIME_MS);
	}
	return true;
}

static bool tcp_init(void)
{
	ts_platform_sleep_ms(1000);
	if (at_wcmd(&tcp_cmd_list[PDP_ACT]) != AT_WCMD_OK)
		return false;
	if (at_wcmd(&tcp_cmd_list[SOCK_CONF]) != AT_WCMD_OK)
		return false;
	if (at_wcmd(&tcp_cmd_list[SOCK_CONF_EXT]) != AT_WCMD_OK)
		return false;
	return true;
}

static bool process_startup_urcs(TsControllerMonarchRef_t m)
{
	mdbg("Waiting for modem start URC\n");
	uint32_t start = ts_platform_time_ms();
	while (!m->modem_started) {
		if (ts_platform_time_ms() - start > STARTUP_TIMEOUT_MS) {
			mdbg("%s:%d Timed out waiting for modem to start\n",
					__func__, __LINE__);
			return false;
		}
		at_intfc_service();
		ts_platform_sleep_ms(IDLE_TIME_MS);
	}
	return true;
}

static bool modem_hw_reset(TsControllerMonarchRef_t m)
{
	m->modem_started = false;
	at_clear_rxbuf();
	ts_driver_reset(m->_controller._driver);
	return process_startup_urcs(m);
}

static TsStatus_t ts_create( TsControllerRef_t * controller ) {
	ts_status_trace( "ts_controller_create: monarch\n" );

	*controller = (TsControllerRef_t)&monarch;
	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t)(*controller);
	if (controller_monarch->being_used) {
		*controller = NULL;
		return TsStatusErrorNoResourceAvailable;
	}

	controller_monarch->modem_started = false;
	controller_monarch->reg_to_net = MODEM_NO_NET;
	controller_monarch->tcp_connected = false;
	controller_monarch->tcp_peer_close = false;
	controller_monarch->unread_tcp = 0;

	TsDriverRef_t driver;
	if (ts_driver_create(&driver) != TsStatusOk)
		return TsStatusErrorInternalServerError;
	(*controller)->_driver = driver;

	mdbg("Begin initialization of modem\n");
	if (!at_init(driver))
		return TsStatusErrorInternalServerError;

	at_reg_urcs(NUM_URCS, urc_list);

	mdbg("Reset the modem hardware\n");
	modem_hw_reset(controller_monarch);

	if (at_wcmd(&core_cmd_list[EPS_URC_SET]) != AT_WCMD_OK)
		return TsStatusErrorInternalServerError;

	if (at_wcmd(&core_cmd_list[EN_CELL_FUNC]) != AT_WCMD_OK)
		return TsStatusErrorInternalServerError;

	mdbg("Checking for network registration\n");
	if (!wait_for_net_reg(controller_monarch, NET_REG_TIMEOUT_MS))
		return false;

	if (at_wcmd(&core_cmd_list[SIM_READY]) != AT_WCMD_OK)
		return false;

	mdbg("Initializing TCP layer\n");
	if (!tcp_init()) {
		mdbg("Failed to initialize the TCP layer\n");
		return false;
	}

	mdbg("%s:%d Modem initialized\n", __func__, __LINE__);
	return TsStatusOk;
}

static TsStatus_t ts_destroy( TsControllerRef_t controller ) {
	ts_status_trace( "ts_controller_destroy\n" );
	ts_platform_assert( ts_platform != NULL );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsControllerMonarchRef_t m = (TsControllerMonarchRef_t)controller;
	m->being_used = false;
	ts_driver_destroy( controller->_driver );
	return TsStatusOk;
}

static TsStatus_t ts_tick( TsControllerRef_t controller, uint32_t budget ) {
	ts_status_trace( "ts_controller_tick\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	uint32_t start = ts_platform_time_ms();
	while (ts_platform_time_ms() - start <= MSEC2USEC(budget))
		at_intfc_service();

	return TsStatusOk;
}

static TsStatus_t ts_connect( TsControllerRef_t controller, TsAddress_t address ) {
	ts_status_trace( "ts_controller_connect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	return TsStatusOk;
}

static TsStatus_t ts_disconnect( TsControllerRef_t controller ) {
	ts_status_trace( "ts_controller_disconnect\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t)controller;
	if (!controller_monarch->tcp_connected) {
		mdbg("%s:%d TCP connection already closed\n", __func__, __LINE__);
		return TsStatusErrorNoResourceAvailable;
	}

	mdbg("%s:%d Closing TCP connection\n", __func__, __LINE__);
	if (at_wcmd(&tcp_cmd_list[SOCK_CLOSE]) != AT_WCMD_OK) {
		mdbg("%s:%d Failed to close TCP connection\n", __func__, __LINE__);
		return TsStatusErrorInternalServerError;
	}
	controller_monarch->tcp_connected = false;
	controller_monarch->tcp_peer_close = false;
	controller_monarch->unread_tcp = 0;
	return TsStatusOk;
}

#define MAX_TCP_RECV_CMD_LEN			32
static TsStatus_t ts_read( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_read\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t)controller;
	if (controller_monarch->unread_tcp == 0)
		return TsStatusOkReadPending;

	if (*buffer_size > AT_BUF_SZ)
		*buffer_size = AT_BUF_SZ;

	if (*buffer_size > controller_monarch->unread_tcp)
		*buffer_size = controller_monarch->unread_tcp;

	char cmd[MAX_TCP_RECV_CMD_LEN];
	at_cmd_desc *tcp_recv = &tcp_cmd_list[SOCK_RECV];
	snprintf(cmd, sizeof(cmd), tcp_recv->cmd_fmt, *buffer_size);
	tcp_recv->cmd = cmd;
	array_t bytes = {*buffer_size, (char *)buffer};
	tcp_recv->resp[0].pvt_data = &bytes;

	if (at_wcmd(tcp_recv) != AT_WCMD_OK) {
		mdbg("%s:%d TCP read error\n", __func__, __LINE__);
		return TsStatusErrorInternalServerError;
	}

	if (bytes.sz == 0) {
		mdbg("%s:%d TCP read error\n", __func__, __LINE__);
		return TsStatusErrorInternalServerError;
	}

	return TsStatusOk;
}

#define MAX_TCP_DATA_LEN			1500
#define MAX_TCP_SEND_CMD_LEN			32
static TsStatus_t ts_write( TsControllerRef_t controller, const uint8_t * buffer, size_t * buffer_size, uint32_t budget ) {
	ts_status_trace( "ts_controller_write\n" );
	ts_platform_assert( ts_driver != NULL );
	ts_platform_assert( controller != NULL );
	ts_platform_assert( buffer_size != NULL );
	ts_platform_assert( *buffer_size > 0 );

	TsControllerMonarchRef_t controller_monarch = (TsControllerMonarchRef_t)controller;

	if (!controller_monarch->tcp_connected) {
		mdbg("%s:%d Unable to send, TCP not connected\n", __func__, __LINE__);
		if (!controller_monarch->tcp_peer_close)
			ts_platform_assert(false);
		else
			return TsStatusErrorConnectionReset;
	}

	if (*buffer_size > MAX_TCP_DATA_LEN)
		*buffer_size = MAX_TCP_DATA_LEN;

	char cmd[MAX_TCP_SEND_CMD_LEN];
	at_cmd_desc *tcp_write = &tcp_cmd_list[SOCK_SEND_CMD];
	snprintf(cmd, sizeof(cmd), tcp_write->cmd_fmt, *buffer_size);
	tcp_write->cmd = cmd;
	if (at_wcmd(tcp_write) != AT_WCMD_OK) {
		mdbg("%s:%d TCP write failed\n", __func__, __LINE__);
		return TsStatusErrorInternalServerError;
	}

	tcp_write = &tcp_cmd_list[SOCK_SEND_DATA];
	tcp_write->cmd = (const char *)buffer;
	tcp_write->cmd_len = *buffer_size;
	//at_toggle_cmd_echo(false);
	if (at_wcmd(tcp_write) != AT_WCMD_OK) {
		mdbg("%s:%d TCP write failed\n", __func__, __LINE__);
		//at_toggle_cmd_echo(true);
		return TsStatusErrorInternalServerError;
	}
	//at_toggle_cmd_echo(true);

	if (!controller_monarch->tcp_connected || !controller_monarch->reg_to_net) {
		mdbg("%s:%d TCP dropped in middle of write\n", __func__, __LINE__);
		return TsStatusErrorConnectionReset;
	}

	return TsStatusOk;
}
