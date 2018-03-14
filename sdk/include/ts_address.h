/**
 * @file
 * ts_address.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * Basic host and port string utilities
 */
#ifndef TS_ADDRESS_H
#define TS_ADDRESS_H

#include "ts_status.h"

/**
 * Maximum size of any host-name character array
 */
#define TS_ADDRESS_MAX_HOST_SIZE 256

/**
 * Maximum size of the port-value character array
 */
#define TS_ADDRESS_MAX_PORT_SIZE 6

/**
 * A well-known network address
 * e.g., "mqtt.thingspace.verizon.com:1883"
 */
typedef const char *TsAddress_t;

/**
 * Path Node(s)
 * e.g., "location" and "longitude" from, "/location/longitude"
 */
typedef char *TsPathNode_t;
typedef TsPathNode_t *TsPathNodes_t;

/**
 * Path
 * e.g., "/location/logitude"
 */
typedef char *TsPath_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse a string in the format "host:port", and fill the host and port out parameters to the pointer to each.
 *
 * @param destination
 * The network address in the form "host:port".
 *
 * @param host
 * [out] pointer to the given host.
 *
 * @param port
 * [out] pointer to the given port.
 *
 * @return
 * TsStatusOk        - Host and port were found successfully
 * TsStatusError...  - Indicates an error has occurred, see ts_status.h for more information.
 */
TsStatus_t ts_address_parse(TsAddress_t destination, char * host, char * port);

#ifdef __cplusplus
}
#endif

#endif // TS_ADDRESS_H
