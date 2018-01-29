/**
 * \file rbuf.h
 * \copyright Copyright (C) 2017 Verizon. All rights reserved.
 * \brief Single producer, single consumer ring buffer API.
 * \details Usually, the writer is an interrupt and the consumer is the
 * execution thread. In addition to reading from and writing to the ring buffer,
 * this module also provides a wildcard based search function.
 */

#ifndef _RBUF_H
#define _RBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \brief Type describing a single producer, single consumer ring buffer.
 */
typedef struct {
	volatile size_t ridx;	/**< Read index */
	volatile size_t widx;	/**< Write index */
	size_t sz;		/**< Maximum size of the ring buffer */
	volatile uint8_t *data;	/**< Pointer to the data buffer */
} rbuf;

/**
 * \brief Pointer to a function that matches against a character class.
 * \details This routine takes a character as an input and verifies if it belongs
 * to a given class (decimal digits, hexadecimal digits, or alphanumeric
 * characters).
 * \param[in] ch Character to be classified.
 * \returns 0 if \ref ch does not belong to the specified class. Non-zero value
 * otherwise.
 */
typedef int (*match_func_t)(int ch);

/**
 * \brief Defines a match format descriptor. The 'fmt' field holds a format
 * string. A format string accepts any byte along with special sequences that
 * act like wildcards:
 * 	'%u' -- Sequence of digits.
 * 	'%s' -- Sequence of alphanumeric characters.
 * 	'%x' -- Sequence of hexadecimal digits.
 * 	'%%' -- Literal %
 * All wildcards are greedy in nature, i.e. take up as many characters of that
 * class as possible. Furthermore, presence of a wildcard in the format string
 * implies that at least one character must match the wildcard.
 * \note Fields other than 'fmt' are private and shouldn't be modified directly.
 */
typedef struct {
	const char *fmt;	/**< Pointer to the NULL terminated format string. */
	size_t sidx;		/**< Privately used counter. */
	size_t cidx;		/**< Privately used counter. */
	match_func_t m_func;	/**< Privately used function pointer. */
} match_fmt_t;

/**
 * \brief Type describing the result of the match routine.
 */
typedef enum {
	MATCH_ERR,	/**< Error in the match format or input parameters. */
	MATCH_FAIL,	/**< Input pattern not found in the ring buffer. */
	MATCH_OK,	/**< Found the pattern in the ring buffer. */
	MATCH_PARTIAL	/**< Found a partial match to the pattern in the ring buffer. */
} match_res_t;

/**
 * \brief Initialize the ring buffer descriptor.
 * \param[in] r Pointer to the ring buffer descriptor to initialize.
 * \param[in] len Maximum size of the ring buffer in bytes.
 * \param[in] buffer Pointer to the underlying buffer to be used.
 * \retval true Ring buffer was initialized successfully.
 * \retval false Failed to initialize ring buffer.
 */
bool rbuf_init(rbuf *r, size_t len, volatile uint8_t buffer[]);

/**
 * \brief Write bytes to the ring buffer beginning at the current write index.
 * \param[in] r Pointer to the ring buffer to use.
 * \param[in] sz Size of the data to write into the ring buffer.
 * \param[in] data Pointer to source data buffer.
 * \retval true Data was successfully written into the ring buffer.
 * \retval false Write failed due to space availability.
 */
bool rbuf_wbs(rbuf *r, size_t sz, const uint8_t data[]);

/**
 * \brief Read bytes from the ring buffer beginning from the current read index.
 * \param[in] r Pointer to the ring buffer to use.
 * \param[in] sz Size of the data to be read from the ring buffer.
 * \param[out] data Pointer to the buffer to receive the byte.
 * \retval true Data was successfully read from the ring buffer.
 * \retval false Read failed due to inavailibility of data in the ring buffer.
 */
bool rbuf_rbs(rbuf *r, size_t sz, uint8_t data[]);

/**
 * \brief Retrieve the number of unread bytes.
 * \param[in] r Pointer to ring buffer to use.
 * \returns Number of bytes of unread bytes.
 */
size_t rbuf_unread(const rbuf * const r);

/**
 * \brief Clear the ring contents of the ring buffer.
 * \param[in] r Pointer to the ring buffer to use.
 */
void rbuf_clear(rbuf *r);

/**
 * \brief Attempt matching the contents of the ring buffer to the format string.
 * \details The search begins at the first unread byte of the ring buffer. On a
 * successful match, /ref sz will hold the size, in bytes, of the matched result.
 * In all other cases, this takes an undefined value.
 *
 * \param[in] r Pointer to the ring buffer to use.
 * \param[in] m Pointer to the match descriptor containing the format string.
 * \param[out] sz Pointer to the buffer that will hold the size of the match.
 *
 * \returns The result of the match. See \ref match_res_t.
 */
match_res_t rbuf_matchf(const rbuf * const r, match_fmt_t *m, size_t *sz);

/**
 * \brief Helper function to reset a format descriptor.
 * \details This routine only resets the private fields of the format descriptor.
 * \param[in] m Pointer to the format descriptor.
 */
void rbuf_reset_fmt_desc(match_fmt_t *m);
#endif
