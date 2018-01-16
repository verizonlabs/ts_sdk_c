// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_ADDRESS_H
#define TS_ADDRESS_H

#include "ts_status.h"

// maximum size of any host-name character array
#define TS_ADDRESS_MAX_HOST_SIZE 256

// maximum size of the port-value character array
#define TS_ADDRESS_MAX_PORT_SIZE 6

// well-known network interface
// e.g., "/dev/ttyAMA0" for USB, "eth0" for socket, etc.
typedef const char *TsInterface_t;

// well-known network address
// e.g., "mqtt.thingspace.verizon.com:1883"
typedef const char *TsAddress_t;

// path node
typedef char *TsPathNode_t;
typedef TsPathNode_t *TsPathNodes_t;

// path
typedef char *TsPath_t;

// verb
typedef char *TsVerb_t;

#ifdef __cplusplus
extern "C" {
#endif

TsStatus_t ts_address_parse(TsAddress_t destination, char * host, char * port);

#ifdef __cplusplus
}
#endif

#endif // TS_ADDRESS_H
