/* Copyright (c) 2017, 2018 Verizon. All rights reserved. */
#include <string.h>
#include <ctype.h>

#include "at_intfc.h"
#include "ts_platform.h"
#include "ts_status.h"

#define atdbg(...)		ts_status_debug(__VA_ARGS__)

/*
 * Enable raw write debug. This should usually be turned off since every AT
 * command uses this function and it can cause to output redundant debug
 * messages.
 */
/*#define DEBUG_AT_WRITE*/

/* Minimum wait time after processing a response from the modem (3GPP standard). */
#define MAX_RESP_INTERVAL_MS	100

/* Maximum number of bytes to dump from the ring buffer in case of an error. */
#define MAX_DUMP_SZ		30

/*
 * Maximum Scratch buffer size. Use this space to temporarily read bytes from
 * the ring buffer and present them in callbacks. All URC / URC header /
 * response matches are assumed to fit within this space.
 */
#define MAX_SCRATCH_SPACE_SZ	128

/*
 * Idle time in milliseconds while waiting for an operation to complete. On
 * systems which are running on an OS, this idle time can be allotted to other
 * tasks.
 */
#define IDLE_TIME_MS		5

static TsDriverRef_t driver;

struct {
	size_t sz;
	at_urc_desc *urcs;
} urc_list;

static bool echo_en;		/* Set when echo is enabled on the modem. */

static rbuf r;			/* AT ring buffer. */
static uint8_t atb[AT_BUF_SZ];	/* Underlying buffer to use. */
static bool at_dbg_en;		/* Set to true to enable debug output. */

void at_toggle_cmd_echo(bool on)
{
	at_dbg_en = on;
}

#define ts_platform_time_ms()	(ts_platform_time() / TS_TIME_MSEC_TO_USEC)
#define ts_platform_sleep_ms(t)	ts_platform_sleep((t) * TS_TIME_MSEC_TO_USEC)
#define MSEC2USEC(t)		((t) * TS_TIME_MSEC_TO_USEC)

/*
 * Read sz bytes from the ring buffer and dump it into buf. If sz bytes are not
 * available, dump all bytes. This should not be called from the interrupt
 * context.
 */
static void dump_buf(size_t sz, uint8_t buf[])
{
	size_t unread = rbuf_unread(&r);
	unread = unread > sz ? sz : unread;
	if (unread == 0)
		return;
	atdbg("Dumping first %u bytes\n", unread);
	rbuf_rbs(&r, unread, buf);
	atdbg("{%02x", buf[0]);
	for (size_t i = 1; i < unread; i++)
		atdbg(", %02x", buf[i]);
	atdbg("}\n");
}

/*
 * Attempt to detect and process a URC. The URC callback is invoked if a complete
 * match is found. Return 'true' on a complete or partial match. If no URC was
 * detected, return 'false'.
 */
static match_res_t process_urcs(void)
{
	bool partial_match = false;
	for (size_t i = 0; i < urc_list.sz; i++) {
		size_t msz = 0;
		match_res_t res = rbuf_matchf(&r, &urc_list.urcs[i].urc, &msz);
		if (res == MATCH_OK) {
			char scratch_space[MAX_SCRATCH_SPACE_SZ];
			rbuf_rbs(&r, msz, (uint8_t *)scratch_space);
			urc_callback cb = urc_list.urcs[i].urc_cb;
			if (cb != NULL)
				cb(msz, scratch_space);
			rbuf_reset_fmt_desc(&urc_list.urcs[i].urc);
			return res;
		}
		if (res == MATCH_PARTIAL)
			partial_match = true;
		else
			rbuf_reset_fmt_desc(&urc_list.urcs[i].urc);
	}
	return partial_match ? MATCH_PARTIAL : MATCH_FAIL;
}

/* Return true if at least one URC was detected. */
static inline bool attempt_proc_urc(void)
{
	bool urc_detected = false;
	if (rbuf_unread(&r) == 0)
		return false;
	while (1) {
		match_res_t res = process_urcs();
		if (res == MATCH_OK)
			urc_detected = true;
		else if (res == MATCH_FAIL)
			break;
		ts_platform_sleep_ms(IDLE_TIME_MS);
	}
	return urc_detected;
}

static TsStatus_t rx_cb(TsDriverRef_t driver, void *reader_state, const uint8_t *data, size_t sz)
{
	rbuf_wbs(&r, sz, data);
	return TsStatusOk;
}

bool at_init(TsDriverRef_t d)
{
	if (d == NULL)
		return false;
	driver = d;
	at_dbg_en = true;

	atdbg("%s:%d Initializing modem receive buffer\n", __func__, __LINE__);
	if (!rbuf_init(&r, sizeof(atb), atb)) {
		atdbg("%s:%d Initialization failed\n", __func__, __LINE__);
		return false;
	}

	atdbg("%s:%d Setting modem communication port callback\n", __func__, __LINE__);
	if (ts_driver_reader(driver, NULL, rx_cb)) {
		atdbg("%s:%d Initialization failed\n", __func__, __LINE__);
		return false;
	}

	return true;
}

void at_reg_urcs(size_t sz, at_urc_desc urcs[])
{
	if (urcs == NULL || sz == 0)
		return;
	atdbg("%s:%d Registering %u new URC(s)\n", __func__, __LINE__, sz);
	urc_list.sz = sz;
	urc_list.urcs = urcs;
	for (size_t i = 0; i < sz; i++) {
		atdbg("%s\n", urcs[i].urc.fmt);
		rbuf_reset_fmt_desc(&urcs[i].urc);
	}
}

static inline void load_resp_err_fmts(uint8_t resp_idx, const at_cmd_desc *at_cmd,
		match_fmt_t *resp, match_fmt_t *err_resp)
{
	rbuf_reset_fmt_desc(resp);
	rbuf_reset_fmt_desc(err_resp);
	resp->fmt = at_cmd->resp[resp_idx].exp_resp;
	err_resp->fmt = at_cmd->err;
}

static void print_at_wcmd_result(const char * f, at_wcmd_res res)
{
	switch (res) {
	case AT_WCMD_OK:
		/* No message on correct behavior */
		break;
	case AT_WCMD_INV:
		atdbg("%s: Invalid params\n", f);
		break;
	case AT_WCMD_TX_ERR:
		atdbg("%s: Transmit error\n", f);
		break;
	case AT_WCMD_UNEXP:
		atdbg("%s: Unexpected response\n", f);
		break;
	case AT_WCMD_ERR:
		atdbg("%s: Error response\n", f);
		break;
	case AT_WCMD_TO:
		atdbg("%s: Timed out\n", f);
		break;
	default:
		atdbg("%s: Undefined response!\n", f);
		break;
	}
}

/*
 * Read the response of size sz bytes from the ring buffer. Call the corresponding
 * response handler - its index is given by 'idx'.
 */
static void invoke_resp_cb(uint8_t idx, size_t sz, const at_cmd_desc *cmd)
{
	char scratch_space[MAX_SCRATCH_SPACE_SZ];
	resp_callback cb = cmd->resp[idx].resp_cb;
	rbuf_rbs(&r, sz, (uint8_t *)scratch_space);
	if (cb != NULL)
		cb(sz, scratch_space, cmd->resp[idx].pvt_data);
}

/* Attempt to eat the echoed command. If successful, return true, else false. */
static bool eat_echo(const at_cmd_desc *cmd, uint32_t timeout)
{
	match_fmt_t cmd_match;
	rbuf_reset_fmt_desc(&cmd_match);
	cmd_match.fmt = cmd->cmd;
	uint32_t start = ts_platform_time_ms();
	size_t sz = 0;
	while (1) {
		if (ts_platform_time_ms() - start > timeout)
			return false;
		match_res_t mres = rbuf_matchf(&r, &cmd_match, &sz);
		if (mres == MATCH_OK) {
			uint8_t buf[sz];
			rbuf_rbs(&r, sz, buf);
			return true;
		} else {
			ts_platform_sleep_ms(IDLE_TIME_MS);
		}
	}
	return false;
}

/*
 * Attempt matching the response from the modem with the stored expected response
 * or the error string of the command. If there's a match, return MATCH_OK and
 * fill in the index from the list of format descriptors that matched. Also
 * fill in the size of the match. If neither match, return MATCH_FAIL.
 */
static match_res_t match_rsp_err(match_fmt_t m[2], size_t *midx, size_t *msz)
{
	for (size_t i = 0; i < 2; i++) {
		match_res_t res = rbuf_matchf(&r, &m[i], msz);
		if (res == MATCH_OK) {
			*midx = i;
			rbuf_reset_fmt_desc(&m[i]);
			return res;
		}
		if (res != MATCH_PARTIAL)
			rbuf_reset_fmt_desc(&m[i]);
	}
	return MATCH_FAIL;
}

#define RSP		0
#define ERR		1
#define ECHO_TMO_MS	250
at_wcmd_res at_wcmd(const at_cmd_desc *at_cmd)
{
	attempt_proc_urc();

	at_wcmd_res res = AT_WCMD_OK;
	if (at_cmd == NULL || at_cmd->cmd == NULL || strlen(at_cmd->cmd) == 0) {
		atdbg("%s:%d Invalid parameters\n", __func__, __LINE__);
		return AT_WCMD_INV;
	}

	size_t cmd_len = at_cmd->cmd_len == 0 ? strlen(at_cmd->cmd)
		: at_cmd->cmd_len;
	if (at_dbg_en)
		atdbg("%s:%d Issuing: %s\n", __func__, __LINE__, at_cmd->cmd);
	if (!at_write(cmd_len, (const uint8_t *)at_cmd->cmd))
		return AT_WCMD_TX_ERR;

	/* If echo is enabled, command will repeat before response. */
	if (echo_en && !eat_echo(at_cmd, ECHO_TMO_MS)) {
		atdbg("%s:%d Expected echo of command\n", __func__, __LINE__);
		return AT_WCMD_TX_ERR;
	}

	match_fmt_t m_list[2];
	uint8_t i = 0;
	while (i < MAX_RESP) {
		if (at_cmd->resp[i].exp_resp == NULL) {
			i++;
			continue;
		}

		size_t m_idx = 0;
		size_t m_sz = 0;
		match_res_t m_res = MATCH_FAIL;
		load_resp_err_fmts(i, at_cmd, &m_list[RSP], &m_list[ERR]);

		uint32_t start = ts_platform_time_ms();
		while (1) {	/* Wait for response / error or timeout. */
			if (ts_platform_time_ms() - start > at_cmd->timeout) {
				res = AT_WCMD_TO;
				break;
			}
			m_res = match_rsp_err(m_list, &m_idx, &m_sz);
			if (m_res == MATCH_OK) {
				if (m_idx == RSP) {
					res = AT_WCMD_OK;
					invoke_resp_cb(i, m_sz, at_cmd);
					break;
				} else if (m_idx == ERR) {
					res = AT_WCMD_ERR;
					goto check_urc;
				}
			} else {
				ts_platform_sleep_ms(IDLE_TIME_MS);
			}
		}

		if (m_res == MATCH_FAIL) {
			/*
			 * On the Ublox modem, a URC cannot appear between the
			 * command and its response. It's not clear if this is
			 * possible on other modems. The following statement
			 * handles this case.
			 */
			if (attempt_proc_urc())
				continue;
			if (res == AT_WCMD_TO)
				goto exit_wcmd;
			res = AT_WCMD_UNEXP;
			uint8_t buf_dump[MAX_DUMP_SZ];
			dump_buf(sizeof(buf_dump), buf_dump);
			rbuf_clear(&r);
			goto exit_wcmd;
		}
		i++;
	}

check_urc:
	/* Recommended wait time after a response / URC is received. */
	ts_platform_sleep_ms(MAX_RESP_INTERVAL_MS);
	if (attempt_proc_urc())
		ts_platform_sleep_ms(MAX_RESP_INTERVAL_MS);
exit_wcmd:
	print_at_wcmd_result(__func__, res);
	return res;
}

bool at_write(size_t sz, const uint8_t data[])
{
	if (data == NULL)
		return false;

#ifdef DEBUG_AT_WRITE
	atdbg("%s:%d Writing %"PRIu16" bytes\n", __func__, __LINE__, sz);
	atdbg("{%02x", data[0]);
	for (size_t i = 1; i < sz; i++)
		atdbg(", %02x", data[i]);
	atdbg("}\n");
#endif

	if (ts_driver_write(driver, data, &sz, MSEC2USEC(TX_TIMEOUT_MS)) != TsStatusOk) {
		atdbg("%s:%d Timed out / error while writing write\n", __func__, __LINE__);
		return false;
	}
	return true;
}

size_t at_avail_bytes(void)
{
	return rbuf_unread(&r);
}

size_t at_read_bytes(size_t sz, uint8_t data[])
{
	size_t unread = rbuf_unread(&r);
	size_t actual_sz = sz > unread ? unread : sz;
	if (actual_sz == 0 || !rbuf_rbs(&r, actual_sz, data))
		return 0;
	else
		return actual_sz;
}

void at_clear_rxbuf(void)
{
	rbuf_clear(&r);
}

void at_set_echo(bool on)
{
	echo_en = on;
	atdbg("%s:%d Echo %s\n", __func__, __LINE__, on ? "on" : "off");
}

void at_intfc_service(void)
{
	attempt_proc_urc();
}

void at_discard_begin_bytes(size_t sz)
{
	size_t unread = rbuf_unread(&r);
	if (sz > unread)
		sz = unread;
	r.ridx += sz;
}
