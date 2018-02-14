/* Copyright (C) 2017, 2018 Verizon. All rights reserved. */
#ifndef _CMD_URC_SQMONARCH_H
#define _CMD_URC_SQMONARCH_H

#include "at_intfc.h"

#define MODEM_PDP_CTX			"3"
#define MODEM_SOCK_ID			"1"

enum modem_core_cmds {
	MODEM_RESET,
	SIM_READY,
	EN_CELL_FUNC,
	EPS_URC_SET,
	NUM_CORE_CMDS
};

enum modem_info_cmds {
	QUERY_IMEI,
	QUERY_RSSI,
	QUERY_IPV4_ADDR,
	QUERY_ICCID,
	QUERY_DATE_AND_TIME,
	QUERY_IMSI,
	QUERY_MODULE_NAME,
	QUERY_MANUFACTURER,
	QUERY_FIRMWARE_VERSION,
	NUM_MODEM_INFO_CMDS
};

enum modem_urcs {
	SYS_START_URC,
	EPS_STAT_URC,
	TCP_CLOSED,
	TCP_RECV_BYTES,
	NUM_URCS
};

enum tcp_cmds {
	PDP_ACT,
	SOCK_CONF,
	SOCK_CONF_EXT,
	SOCK_DIAL,
	SOCK_CLOSE,
	SOCK_SEND_CMD,
	SOCK_SEND_DATA,
	SOCK_RECV,
	NUM_TCP_CMDS
};

/* Response Callbacks */
static void parse_tcp_data(size_t sz, const char resp[], void *pvt_data);
static void parse_imei(size_t sz, const char resp[], void *pvt_data);
static void parse_rssi(size_t sz, const char resp[], void *pvt_data);
static void parse_ipv4_addr(size_t sz, const char resp[], void *pvt_data);
static void parse_iccid(size_t sz, const char resp[], void *pvt_data);
static void parse_date_and_time(size_t sz, const char resp[], void *pvt_data);
static void parse_imsi(size_t sz, const char resp[], void *pvt_data);
static void parse_module_name(size_t sz, const char resp[], void *pvt_data);
static void parse_manufacturer(size_t sz, const char resp[], void *pvt_data);
static void parse_firmware_version(size_t sz, const char resp[], void *pvt_data);

static const char err_str[] = "\r\nERROR\r\n";

static const at_cmd_desc core_cmd_list[NUM_CORE_CMDS] = {
	[MODEM_RESET] = {
		.cmd = "at^reset\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			},
			{
				.exp_resp = "\r\n+SHUTDOWN\r\n"
			}
		},
		.timeout = 5000
	},
	[SIM_READY] = {
		.cmd = "at+cpin?\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+CPIN: READY\r\n"
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 15000
	},
	[EN_CELL_FUNC] = {
		.cmd = "at+cfun=1\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[EPS_URC_SET] = {
		.cmd = "at+cereg=1\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	}
};

static at_cmd_desc query_cmd_list[NUM_MODEM_INFO_CMDS] = {
	[QUERY_IMEI] = {
		.cmd = "at+cgsn\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp  = "\r\n%u\r\n",
				.resp_cb = parse_imei
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 100
	},
	[QUERY_RSSI] = {
		.cmd = "at+csq\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+CSQ: %u,%u\r\n",
				.resp_cb = parse_rssi
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_IPV4_ADDR] = {
		.cmd = "at+cgpaddr="MODEM_PDP_CTX"\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+CGPADDR: "MODEM_PDP_CTX"," \
					     "\"%u.%u.%u.%u\"," \
					     "\"%u.%u.%u.%u." \
					     "%u.%u.%u.%u." \
					     "%u.%u.%u.%u." \
					     "%u.%u.%u.%u\"\r\n",
				.resp_cb = parse_ipv4_addr
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_ICCID] = {
		.cmd = "at+sqnccid\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+SQNCCID: \"%u\",\"\"\r\n",
				.resp_cb = parse_iccid
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_DATE_AND_TIME] = {
		.cmd = "at+cclk?\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+CCLK: \"%u/%u/%u,%u:%u:%u-%u\"\r\n",
				.resp_cb = parse_date_and_time
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_IMSI] = {
		.cmd = "at+cimi\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n%u\r\n",
				.resp_cb = parse_imsi
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_MODULE_NAME] = {
		.cmd = "at+cgmm\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n%s\r\n",
				.resp_cb = parse_module_name
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	},
	[QUERY_MANUFACTURER] = {
		.cmd = "at+cgmi\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n%s %s\r\n",
				.resp_cb = parse_manufacturer
			},
			{
				.exp_resp = "\r\nOK\r\n"
			},
		},
		.timeout = 500
	},
	[QUERY_FIRMWARE_VERSION] = {
		.cmd = "at+cgmr\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n%s.%u.%u.%u%s\r\n",
				.resp_cb = parse_firmware_version
			},
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 500
	}
};

static at_cmd_desc tcp_cmd_list[NUM_TCP_CMDS] = {
	[PDP_ACT] = {
		.cmd = "at+cgact=1,"MODEM_PDP_CTX"\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[SOCK_CONF] = {
		.cmd = "at+sqnscfg="MODEM_SOCK_ID","MODEM_PDP_CTX",0,600,600,50\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[SOCK_CONF_EXT] = {
		.cmd = "at+sqnscfgext="MODEM_SOCK_ID",1,0,0,0,0,0,0\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[SOCK_DIAL] = {
		.cmd_fmt = "at+sqnsd="MODEM_SOCK_ID",0,%s,\"%s\",0,0,1\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 7000
	},
	[SOCK_CLOSE] = {
		.cmd = "at+sqnsh="MODEM_SOCK_ID"\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[SOCK_SEND_CMD] = {
		.cmd_fmt = "at+sqnssendext="MODEM_SOCK_ID",%u\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n> "
			}
		},
		.timeout = 5000
	},
	[SOCK_SEND_DATA] = {
		.cmd = NULL,	/* Filled with data bytes before issuing */
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\nOK\r\n"
			}
		},
		.timeout = 5000
	},
	[SOCK_RECV] = {
		.cmd_fmt = "at+sqnsrecv="MODEM_SOCK_ID",%u\r",
		.err = err_str,
		.resp = {
			{
				.exp_resp = "\r\n+SQNSRECV: "MODEM_SOCK_ID",%u\r\n",
				.resp_cb = parse_tcp_data
			}
		},
		.timeout = 5000
	}
};

/* URC Callbacks */
static void sys_start(size_t sz, const char urc_text[]);
static void parse_cereg_urc(size_t sz, const char urc_text[]);
static void tcp_conn_closed(size_t sz, const char urc_text[]);
static void tcp_recv_bytes(size_t sz, const char urc_text[]);

static at_urc_desc urc_list[NUM_URCS] = {
	[SYS_START_URC] = {
		.urc = {
			.fmt = "\r\n+SYSSTART\r\n"
		},
		.urc_cb = sys_start
	},
	[EPS_STAT_URC] = {
		.urc = {
			.fmt = "\r\n+CEREG: %u\r\n"
		},
		.urc_cb = parse_cereg_urc
	},
	[TCP_CLOSED] = {
		.urc = {
			.fmt = "\r\n+SQNSH: "MODEM_SOCK_ID"\r\n"
		},
		.urc_cb = tcp_conn_closed
	},
	[TCP_RECV_BYTES] = {
		.urc = {
			.fmt = "\r\n+SQNSRING: "MODEM_SOCK_ID",%u\r\n"
		},
		.urc_cb = tcp_recv_bytes
	}
};
#endif
