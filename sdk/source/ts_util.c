// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

#include "ts_platform.h"
#include "ts_util.h"
#include "ts_status.h"
#include <string.h>

// Use the ts_random entry point to generate random hex bytes.
// Set - Set bits in the output as specified by set bits here
// Mask - Clear bits in the output as specified by cleared bits here
static void _fourrandomhexbytes( char * out, uint32_t set, uint32_t mask) {
	uint32_t tmp;
	ts_platform_random(&tmp);

	// Set/clear some bits as specified by set and mask

	tmp = set | (tmp & mask);

	snprintf(out, 9, "%08lx", tmp);
}

// Make a UUID. Out must have room for 36 characters + 1 null termination (UUID_SIZE).
void ts_uuid( char * out ) {
	char hex[9];

	memset(out, 0, UUID_SIZE);

	_fourrandomhexbytes(hex, 0x0, 0xffffffff);
	strncpy(out, hex, 9);
	strncat(out, "-", 1);

	// Set some bits in the second and third words as specified by the UUID spec
	_fourrandomhexbytes(hex, 0x00004000, 0xffff0fff);
	strncat(out, hex, 4);
	strncat(out, "-", 1);
	strncat(out, &hex[4], 4);
	strncat(out, "-", 1);

	_fourrandomhexbytes(hex, 0x80000000, 0x3fffffff);
	strncat(out, hex, 4);
	strncat(out, "-", 1);
	strncat(out, &hex[4], 4);

	_fourrandomhexbytes(hex, 0x0, 0xffffffff);
	strncat(out, hex, 8);
}

// Try to allocate a buffer with progressively smaller sizes until we succeed (or reach a limit).
uint8_t *ts_get_buffer(size_t* size, size_t minimum) {
	uint8_t *buffer = NULL;
	while (buffer == NULL && (*size) >= minimum) {
		buffer = (uint8_t*) ts_platform_malloc( *size );
		if (buffer == NULL) {
			ts_status_debug("buffer allocation failed at size = %d\n", *size);
			(*size) /= 2;
			if ((*size) >= minimum) {
				ts_status_debug("   retrying at size %d...\n", *size);
			}
		}
	}
	return buffer;
}
