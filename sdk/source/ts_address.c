// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>

#include "ts_status.h"
#include "ts_address.h"

/**
 * Copy the host and port section of address (delimited by a colon) to the given host and
 * port array.
 * @param address
 * The string (i.e., zero terminated character array) containing the host and port
 * as, [HOST] ':' [PORT], e.g., 'verizon.com:8080'
 * @param host
 * A copy host section of the address.
 * @param port
 * A copy of the port section of the address.
 * @return
 * TsStatusError if the given format does not match the expected format,
 * TsStatusOk otherwise.
 */
TsStatus_t ts_address_parse(TsAddress_t address, char * host, char * port) {
	int host_length = 0;
	size_t address_length = strlen(address), port_length = 0;
	char * port_start = NULL;
	for( int i = 0; (i < address_length) && (i < TS_ADDRESS_MAX_HOST_SIZE + TS_ADDRESS_MAX_PORT_SIZE + 1); i++) {
		if( address[i] == ':' ) {
			host_length = i;
			port_start = (char*)(&address[i + 1]);
			port_length = (size_t)(address_length - i - 1);
			break;
		}
	}
	if( ( port_start != NULL ) && ( host_length > 0 ) && ( port_length > 0 ) ) {
		memset(host, 0x00, TS_ADDRESS_MAX_HOST_SIZE);
		memset(port, 0x00, TS_ADDRESS_MAX_PORT_SIZE);
		// TODO - not zero terminated when length equals size, need to check,...
		memcpy(host, address, host_length);
		memcpy(port, port_start, port_length);
		return TsStatusOk;
	}
	return TsStatusErrorBadRequest;
}
