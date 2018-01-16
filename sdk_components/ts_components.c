// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_components.h"

// application protocol binding
#if defined(TS_PROTOCOL_TSJSON)
const TsProtocolVtable_t *   ts_transport = &(ts_protocol_ts_json);
#elif defined(TS_PROTOCOL_TSCBOR)
const TsProtocolVtable_t *   ts_transport = &(ts_protocol_ts_cbor);
#elif defined(TS_PROTOCOL_NONE)
const TsProtocolVtable_t *   ts_transport = &(ts_protocol_none);
#elif defined(TS_PROTOCOL_CUSTOM)
// do nothing
#else
#warning "TS_PROTOCOL_<TYPE> not defined, options include TSJSON (deprecated), TSCBOR, NONE or CUSTOM"
#endif

// application protocol transport binding
#if defined(TS_TRANSPORT_MQTT)
const TsTransportVtable_t *   ts_transport = &(ts_transport_mqtt);
#elif defined(TS_TRANSPORT_CUSTOM)
#else
#warning "TS_TRANSPORT_<TYPE> not defined, options include MQTT or CUSTOM"
#endif

// connection security binding
#if defined(TS_SECURITY_MBED)
const TsSecurityVtable_t *    ts_security = &(ts_security_mbedtls);
#elif defined(TS_SECURITY_MOCANA)
const TsSecurityVtable_t *    ts_security = &(ts_security_mocana);
#elif defined(TS_SECURITY_NONE)
const TsSecurityVtable_t *    ts_security = &(ts_security_none);
#elif defined(TS_SECURITY_CUSTOM)
// do nothing
#else
#warning "TS_SECURITY_<TYPE> not defined, options include NONE, MBED, MOCANA or CUSTOM"
#endif

// connection controller binding
#if defined(TS_CONTROLLER_SOCKET)
const TsControllerVtable_t *  ts_controller = &(ts_controller_socket);
#elif define(TS_CONTROLLER_MONARCH)
const TsControllerVtable_t *  ts_controller = &(ts_controller_monarch);
#elif define(TS_CONTROLLER_CUSTOM)
// do nothing
#else
#warning "TS_CONTROLLER_<TYPE> not defined, options include MONARCH, SOCKET (UNIX only) or CUSTOM"
#endif
