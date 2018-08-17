/**
 * @file
 * ts_message.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A directed acyclic graph, i.e., a tree data-type.
 *
 * @details
 * The "message" is used to hold data as a tree structure in an encoding-agnostic way. It
 * is used as the intermediate structure between encoded data, i.e., JSON, CBOR and a special
 * compressed CBOR form using predefined key and value tokens (used by the TS-CBOR protocol).
 *
 * @code
 *
 * 	TsMessageRef_t sensors, location;
 * 	ts_message_create( &sensors );
 *
 * 	// create a name-value pair at the root, "temperature"
 * 	ts_message_set_float( sensors, "temperature", 50.2 );
 *
 * 	// create a branch, "location"
 * 	ts_message_create_message( sensors, "location", &location );
 * 	ts_message_set_float( location, "longitude", -71.0 );
 * 	ts_message_set_float( location, "latitude", 42.3 );
 *
 * 	// encode to JSON
 * 	uint8_t buffer[ 2048 ];
 * 	size_t buffer_size( sizeof( buffer ) );
 * 	ts_message_encode( sensors, TsEncoderJson, buffer, buffer_size );
 *
 * 	// clean-up
 * 	ts_message_destroy( sensors );
 *
 * @endcode
 */
#ifndef TS_MESSAGE_H
#define TS_MESSAGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "cJSON.h"
#include "cbor.h"

#include "ts_status.h"
#include "ts_address.h"

// static memory model, e.g., for debug (warning - affects bss directly) 
// #define TS_MESSAGE_STATIC_MEMORY 

// maximum number of roots 
// this is just for guidance - at runtime, the application could allocate 
// all of the nodes for just one message (however, note TS_MESSAGE_MAX_DEPTH). 
#define TS_MESSAGE_MAX_ROOTS        3

// maximum number of branches allowed per node 
// for TsTypeMessage, limits the number of attributes per JSON/CBOR object 
// for TsTypeArray, limits the maximum size of the array. 
#define TS_MESSAGE_MAX_BRANCHES     20

// total number of nodes available for messages 
#define TS_MESSAGE_MAX_NODES        (TS_MESSAGE_MAX_BRANCHES * TS_MESSAGE_MAX_ROOTS)

// length of a UUID plus dashes
#define TS_MESSAGE_UUID_SIZE        36

// maximum size of a string attribute

#define TS_MESSAGE_MAX_STRING_SIZE  4096

//i.e., lenght of cert size is 3000 ( > 2048)
#define TS_MESSAGE_MAX_CERT_SIZE  3000

// maximum size of a key (i.e., field name) 
#define TS_MESSAGE_MAX_KEY_SIZE     24

// supported encoders 
typedef enum {
	TsEncoderDebug,
	TsEncoderJson,
	TsEncoderCbor,
	TsEncoderTsCbor,
} TsEncoder_t;

// supported encoded field types
typedef enum {
	TsTypeInteger,  // int* 
	TsTypeFloat,    // float* 
	TsTypeBoolean,  // stdbool, bool* 
	TsTypeString,   // zero terminated byte array (i.e., char *) 
	TsTypeCert,	// zero terminated byte array max limit is 3K (i.e., char *) 
	TsTypeMessage,  // TsMessage_t*[N], where N is the number of fields 
	TsTypeArray,    // TsMessage_t*[N], where N is the number of elements 
	TsTypeNull      // no value 
} TsType_t;

/**
 * The message object reference
 */
typedef struct TsMessage *TsMessageRef_t;

/**
 * The message object
 */
typedef TsStatus_t (*TsMessageHandler_t)( void * source, int source_action, TsMessageRef_t );

/**
 * A message value
 */
typedef void *TsValue_t;

// message string (zero terminated)
typedef char *TsString_t;

// field value 
// note, union size will take the largest attribute 
typedef union TsField *TsFieldRef_t;
typedef union {
	int _xinteger;
	float _xfloat;
	bool _xboolean;
	TsString_t _xstring;
	// TODO - switch to linked list 
	TsMessageRef_t _xfields[TS_MESSAGE_MAX_BRANCHES];
} TsField_t;

// a single message node binding 
// (which, during runtime, could be either a root or a branch node)
// TODO - add verb? e.g., post, get, etc.
typedef struct TsMessage {
	int references;
	char name[TS_MESSAGE_MAX_KEY_SIZE];
	TsType_t type;
	TsField_t value;
} TsMessage_t;

#ifdef __cplusplus
extern "C" {
#endif

// create and destroy 
TsStatus_t ts_message_report();


/**
 * Allocate and initialize a new message object.
 *
 * @param message
 * [on/out] The pointer to a pre-existing TsMessageRef_t, which will be initialized with the message state.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_message_create(TsMessageRef_t *message);

TsStatus_t ts_message_create_copy(TsMessageRef_t message, TsMessageRef_t *value);
TsStatus_t ts_message_create_array(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);
TsStatus_t ts_message_create_message(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);

/**
 * Deallocate the given message object.
 *
 * @param message
 * [in] The message state.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_message_destroy(TsMessageRef_t message);

// set and get operations 
TsStatus_t ts_message_set(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value);
TsStatus_t ts_message_set_null(TsMessageRef_t message, TsPathNode_t field);
TsStatus_t ts_message_set_int(TsMessageRef_t message, TsPathNode_t field, int value);
TsStatus_t ts_message_set_float(TsMessageRef_t message, TsPathNode_t field, float value);
TsStatus_t ts_message_set_string(TsMessageRef_t message, TsPathNode_t field, char *value);
TsStatus_t ts_message_set_cert( TsMessageRef_t message, TsPathNode_t field, char * value );
TsStatus_t ts_message_set_bool(TsMessageRef_t message, TsPathNode_t field, bool value);
TsStatus_t ts_message_set_array(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value);
TsStatus_t ts_message_set_message(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value);

TsStatus_t ts_message_has(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);

TsStatus_t ts_message_get(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);
TsStatus_t ts_message_get_int(TsMessageRef_t message, TsPathNode_t field, int *value);
TsStatus_t ts_message_get_float(TsMessageRef_t message, TsPathNode_t field, float *value);
TsStatus_t ts_message_get_string(TsMessageRef_t message, TsPathNode_t field, char **value);
TsStatus_t ts_message_get_bool(TsMessageRef_t message, TsPathNode_t field, bool *value);
TsStatus_t ts_message_get_array(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);
TsStatus_t ts_message_get_message(TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t *value);

// array operations 
TsStatus_t ts_message_get_size(TsMessageRef_t array, size_t *size);

TsStatus_t ts_message_set_at(TsMessageRef_t array, size_t index, TsMessageRef_t item);
TsStatus_t ts_message_set_int_at(TsMessageRef_t array, size_t index, int value);
TsStatus_t ts_message_set_float_at(TsMessageRef_t array, size_t index, float value);
TsStatus_t ts_message_set_string_at(TsMessageRef_t array, size_t index, char *value);
TsStatus_t ts_message_set_bool_at(TsMessageRef_t array, size_t index, bool value);
TsStatus_t ts_message_set_array_at(TsMessageRef_t array, size_t index, TsMessageRef_t item);
TsStatus_t ts_message_set_message_at(TsMessageRef_t array, size_t index, TsMessageRef_t item);

TsStatus_t ts_message_get_at(TsMessageRef_t array, size_t index, TsMessageRef_t *item);

// encoding and decoding 
TsStatus_t ts_message_encode(TsMessageRef_t message, TsEncoder_t encoder, uint8_t *buffer, size_t *buffer_size);
TsStatus_t ts_message_decode(TsMessageRef_t message, TsEncoder_t encoder, uint8_t *buffer, size_t buffer_size);
TsStatus_t ts_message_decode_json(TsMessageRef_t message, cJSON *value);
TsStatus_t ts_message_decode_cbor(TsMessageRef_t message, CborValue *value);

// debugging
TsStatus_t ts_message_dump(TsMessageRef_t message);

#ifdef __cplusplus
}
#endif

#endif // TS_MESSAGE_H 
