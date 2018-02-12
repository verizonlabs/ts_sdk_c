/* Copyright (c) 2017, 2018 Verizon. All rights reserved. */
#include <ctype.h>
#include <string.h>
#include "rbuf.h"

/* Refers to the first unread byte in the ring buffer. */
#define RBUF_BEGIN                     ((size_t)0xFFFFFFFF)

bool rbuf_init(rbuf *r, size_t len, volatile uint8_t buffer[])
{
	if (r == NULL || len == RBUF_BEGIN || buffer == NULL)
		return false;
	r->sz = len;
	r->data = buffer;
	rbuf_clear(r);
	return true;
}

bool rbuf_wbs(rbuf *r, size_t sz, const uint8_t data[])
{
	if (r == NULL || data == NULL || (r->widx - r->ridx) + sz > r->sz)
		return false;
	for (size_t i = 0; i < sz; i++) {
		r->data[r->widx % r->sz] = data[i];
		r->widx++;
	}
	return true;
}

bool rbuf_rbs(rbuf *r, size_t sz, uint8_t data[])
{
	if (r == NULL || data == NULL || r->widx - r->ridx < sz)
		return false;
	for (size_t i = 0; i < sz; i++) {
		data[i] = r->data[r->ridx % r->sz];
		r->ridx++;
	}
	return true;
}

size_t rbuf_unread(const rbuf * const r)
{
	if (r == NULL)
		return 0;
	return (size_t)(r->widx - r->ridx);
}

void rbuf_clear(rbuf *r)
{
	if (r == NULL)
		return;
	r->ridx = 0;
	r->widx = 0;
}

void rbuf_reset_fmt_desc(match_fmt_t *m)
{
	m->cidx = 0;
	m->sidx = RBUF_BEGIN;
	m->m_func = NULL;
}

/* Detect errors in the format string and select the correct matching function. */
static bool set_match_func(match_fmt_t *m)
{
	m->cidx++;
	switch (m->fmt[m->cidx]) {
	case 'u':
		m->m_func = isdigit;
		m->cidx++;			/* Skip over entire wildcard */
		break;
	case 's':
		m->m_func = isalnum;
		m->cidx++;			/* Skip over entire wildcard */
		break;
	case 'x':
		m->m_func = isxdigit;
		m->cidx++;			/* Skip over entire wildcard */
		break;
	case '%':
		m->m_func = NULL;
		/*
		 * Don't skip over suffix i.e. the last '%' of '%%'. It will be
		 * compared as a literal.
		 */
		break;
	default:
		return false;			/* Format string error */
	}
	return true;
}

match_res_t rbuf_matchf(const rbuf * const r, match_fmt_t *m, size_t *sz)
{
	if (r == NULL || m == NULL || sz == NULL)
		return MATCH_ERR;

	if (r->widx - r->ridx == 0 || m->fmt == NULL)
		return MATCH_FAIL;

	/*
	 * If set, matching future characters from the ring buffer belonging to
	 * the same class (%u, %s or %x) becomes optional. This is automatically
	 * set to true after a single character match for all classes.
	 */
	bool fut_match_opt = false;

	size_t i = m->sidx == RBUF_BEGIN ? r->ridx : m->sidx;

	while (i != r->widx && m->fmt[m->cidx] != '\0') {
		if (m->m_func == NULL && m->fmt[m->cidx] == '%')
			if (!set_match_func(m))
				return MATCH_ERR;

		size_t ridx = i % r->sz;		/* Adjust read index into buffer */
		if (m->m_func == NULL) {		/* Literal comparison */
			if (m->fmt[m->cidx++] != r->data[ridx])
				return MATCH_FAIL;
			i++;
		} else {				/* Wildcard comparison */
			if (m->m_func(r->data[ridx]) == 0) {
				if (!fut_match_opt)
					return MATCH_FAIL;
				m->m_func = NULL;
				fut_match_opt = false;
			} else {
				i++;
				fut_match_opt = true;
			}
		}
	}

	if (m->fmt[m->cidx] == '\0') {		/* End of match specifier */
		*sz = (size_t)(i - r->ridx);
		return MATCH_OK;
	}
	m->sidx = i;
	return MATCH_PARTIAL;
}
