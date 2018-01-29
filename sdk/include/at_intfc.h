/**
 * \file at_intfc.h
 * \copyright Copyright (C) 2017, 2018 Verizon. All rights reserved.
 * \brief AT command interpreter interface.
 *
 * \details This module is the AT command interpreter interface that allows to
 * send generic AT commands and receive responses. In addition, it detects URCs
 * and issues a callback on all matched detections. The interface assumes that
 * URCs are buffered by the modem while a command string is being sent to it
 * and also while a response is being sent by the modem.
 */
#ifndef _AT_INTFC_H
#define _AT_INTFC_H

#include <stdbool.h>
#include <stdint.h>

#include "ts_sdk.h"
#include "rbuf.h"

/**
 * \brief Enable debug output from the AT interface module.
 */
#define DEBUG_AT_LIB

/**
 * \brief Size of the buffer held by the AT command interpreter.
 */
#define AT_BUF_SZ		((size_t)2048)

/**
 * \brief Maximum number of response lines per AT command.
 */
#define MAX_RESP		2

/**
 * \brief Maximum time in milliseconds to complete transmission to modem.
 * \details This is the maximum amount of time the module will wait for a
 * successful transmission through the communication port (Eg. UART) to the
 * modem while sending a command in command mode / data in online mode. It does
 * not include the time taken by the modem to respond.
 */
#define TX_TIMEOUT_MS		5000

/**
 * \brief Callback to be invoked on receiving bytes on the input port.
 *
 * \details By default, any bytes received on the input port are routed to the
 * AT command interpreter. However, in some modes of the modem such as online
 * data mode, the bytes represent data and must be handled outside the AT layer.
 * This callback allows direct access to those bytes as they arrive from the
 * modem. To enable / disable it, use \ref at_set_data_fwd. Bytes are still
 * stored in the internal AT buffer and must be flushed explicitly using /ref
 * at_read_bytes.
 *
 * \param[out] sz Number of bytes received.
 * \param[out] data Pointer to the buffer containing the actual data bytes.
 *
 * \warning This callback will likely be invoked from the interrupt context.
 */
typedef void (*data_fwd_callback)(size_t sz, const uint8_t data[]);

/**
 * \brief Callback that is invoked on receiving a matched response.
 *
 * \details The response callback is called when the response to an AT command
 * matches the expected response \ref exp_resp. This callback is invoked from
 * outside the interrupt context.
 *
 * \param[out] sz Size of the response in bytes.
 * \param[out] resp Actual response from the modem.
 * \param[out] pvt_data Pointer to private data that may be used by the callback.
 *
 * \warning \ref at_wmcd should not be called from the response callback.
 */
typedef void (*resp_callback)(size_t sz, const char resp[], void *pvt_data);

/**
 * \brief Callback that is invoked on receiving a URC.
 * \details This callback is invoked from outside the interrupt context. URCs
 * are stored in the internal AT buffer in the order they are sent by the modem.
 * URC callbacks are invoked in the same order.
 *
 * \param[out] sz Number of bytes in the URC.
 * \param[out] urc_text Text of the URC.
 *
 * \note URCs are stored in the internal AT buffer in the order they are sent
 * by the modem. Therefore URC callbacks are also invoked in the same order. A
 * URC appearing first in a sequence must be processed before the other URCs that
 * follow.
 *
 * \warning \ref at_wmcd should not be called from the URC callback.
 */
typedef void (*urc_callback)(size_t sz, const char urc_text[]);

/**
 * \brief Defines a descriptor to store the expected response and associated
 * callback.
 * \details The response may contain one or more wildcards. '%u' represents
 * a sequence of digits, '%s' represents alphanumeric characters (0-9A-Za-z),
 * '%x' represents a sequence of hexadecimal digits (0-9A-Fa-f). On a match the
 * response callback is invoked (if it exists) with the private data. If a
 * response callback is not desired, pass in NULL.\n
 * For example, "\r\nCEREG: %u,%u\r\n", matches "\r\nCEREG: 0,1\r\n" and
 * "\r\nCEREG: 19,19\r\n" but not "\r\nCEREG: 0\r\n".
 */
typedef struct {
	const char *exp_resp;	/**< Expected response. */
	resp_callback resp_cb;	/**< Pointer to the response callback. */
	void *pvt_data;		/**< Pointer to private data of the callback. */
} at_resp_desc;

/**
 * \brief Defines a descriptor to store the expected format of the URC / its
 * header (for long URCs) along with the associated callback.
 *
 * \details The URC / header may contain one or more wildcards. '%u' represents
 * a sequence of digits, '%s' represents alphanumeric characters (0-9A-Za-z),
 * '%x' represents a sequence of hexadecimal digits (0-9A-Fa-f). On a match the
 * URC callback is invoked (if it exists). If a URC callback is not desired,
 * pass in NULL.\n
 * For example, "\r\n+CEREG: %u\r\n" will match "\r\n+CEREG: 1\r\n" and
 * "\r\n+CEREG: 19\r\n" but not "\r\n+CEREG: a\r\n".
 */
typedef struct {
	match_fmt_t urc;	/**< Expected URC match descriptor. */
	urc_callback urc_cb;	/**< Pointer to the URC callback. */
} at_urc_desc;

/**
 * \brief Defines a descriptor to store an AT command.
 * \details The actual command is stored in \ref cmd. However, the command may
 * be entered as a C format string which can then be used to populate the \ref
 * cmd field (say, using snprintf) before issuing it using \ref at_wcmd. The
 * error string accepts the same wildcards as the response string
 * (/ref at_resp_desc).
 */
typedef struct {
	const char *cmd_fmt;		/**< printf style format string describing the command */
	const char *cmd;		/**< AT command to be issued */
	const char *err;		/**< Expected error format string */
	at_resp_desc resp[MAX_RESP];	/**< Describes the response of the command */
	uint32_t timeout;		/**< Timeout (in milliseconds) for the command */
} at_cmd_desc;

/**
 * \brief Type defining the result of issuing the AT command.
 */
typedef enum {
	AT_WCMD_OK,		/**< AT command succeeded */
	AT_WCMD_INV,		/**< Invalid parameters */
	AT_WCMD_TX_ERR,		/**< Transmit error while issuing AT command */
	AT_WCMD_UNEXP,		/**< Unexpected response to AT command */
	AT_WCMD_ERR,		/**< Error response to AT command */
	AT_WCMD_TO		/**< Timed out waiting for response */
} at_wcmd_res;

/**
 * \brief Initialize the AT command interpreter interface.
 * \param[in] t Pointer to the tick management interface.
 * \param[in] m Pointer to the modem's hardware interface.
 * \param[in] a Pointer to the alternative callback to process data (See \ref
 * data_fwd_callback). If unused, pass in NULL.
 * \retval true AT interpreter was successfully initialized.
 * \retval false Initialization failed.
 * \note This routine must be called once before all other routines in this module.
 */
bool at_init(const tick_intfc_t *t, const modem_intfc_t *m, data_fwd_callback d);

/**
 * \brief Register URC descriptors with the AT interface.
 * \details If a new set of URCs are registered, the older set is automatically
 * unregistered.
 * \param[in] sz Number of elements in the URC descriptor array.
 * \param[in] urc Pointer to an array of URC descriptors.
 * \pre \ref at_init must be called before using this routine.
 */
void at_reg_urcs(size_t sz, at_urc_desc urcs[]);

/**
 * \brief Issue a command on the AT interface.
 * \param[in] at_cmd Pointer to the command descriptor of the AT command to be
 * issued.
 * \return A value of \ref at_wcmd_res type that describes the result of the
 * operation.
 * \pre \ref at_init must be called before using this routine.
 */
at_wcmd_res at_wcmd(const at_cmd_desc *at_cmd);

/**
 * \brief Write raw bytes to the AT interface.
 * \param[in] sz Number of bytes to write to the interface.
 * \param[in] data Pointer to the source data.
 * \retval true Write succeeded.
 * \retval false Write failed.
 * \pre \ref at_init must be called before using this routine.
 */
bool at_write(size_t sz, const uint8_t data[]);

/**
 * \brief Return the number of unread bytes in the internal AT buffer.
 * \return Number of bytes that are unread in the internal AT buffer.
 * \pre \ref at_init must be called before using this routine.
 */
size_t at_avail_bytes(void);

/**
 * \brief Read the next few unread bytes.
 * \details This routine is used to read the unread bytes into the user supplied
 * buffer from the beginning of the internal AT buffer.
 *
 * \param[in] sz Number of bytes to read.
 * \param[in] data Pointer to the buffer where the data will be read into.
 *
 * \return Number of bytes actually read into the buffer. This may be less than
 * the number of bytes requested to be read.
 *
 * \pre \ref at_init must be called before using this routine.
 */
size_t at_read_bytes(size_t sz, uint8_t data[]);

/**
 * \brief Clear the AT receive buffer.
 * \details Unprocessed URCs and command responses will be lost after a call to
 * this routine.
 * \pre \ref at_init must be called before using this routine.
 */
void at_clear_rxbuf(void);

/**
 * \brief Inform the AT interface the state of the modem's echo settings.
 * \details Knowing what state the echo is in helps the interface parse the
 * response correctly. If echo is on, the effective response received by the
 * modem is prepended by the command. When echo is off, only the response is
 * returned.
 * \param[in] on If set to true, it means that the modem has echo enabled.
 */
void at_set_echo(bool on);

/**
 * \brief Inform the AT interface whether bytes from the modem's output port
 * should also be routed to a callback for processing.
 * \param[in] on Set to true to turn on forwarding, false to stop.
 */
void at_set_data_fwd(bool on);

/**
 * \brief Service the AT interface.
 * \details This routine must be called periodically to ensure the AT interface
 * processes all inputs and reacts to all URCs.
 */
void at_intfc_service(void);

/**
 * \brief Flush recently written bytes.
 * \detail Removes bytes from the end of the internal AT buffer.
 * \param[in] sz Number of bytes to remove. If this is larger than the number of
 * bytes available, all bytes are removed.
 * \warning This routine can be safely called only from the data forward callback.
 */
void at_discard_end_bytes(size_t sz);

/**
 * \brief Flush the first few bytes.
 * \detail Removes bytes from the beginning of the internal AT buffer.
 * \param[in] sz Number of bytes to remove. If this is larger than the number of
 * bytes available, all bytes are removed.
 */
void at_discard_begin_bytes(size_t sz);

/**
 * \brief Helper function to toggle debug output.
 * \detail This routine can be used to prevent \ref at_wcmd from echoing the
 * command it is executing. It can be useful when the command involves writing
 * non-ASCII characters to the debug port.
 * \param[in] on Set to true to turn on echo, false to turn it off.
 */
void at_toggle_cmd_echo(bool on);

#endif
