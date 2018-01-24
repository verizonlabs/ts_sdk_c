// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <string.h>
#include <stdio.h>

#include "cbor.h"
#include "cJSON.h"

#include "ts_status.h"
#include "ts_message.h"

/* static memory model, e.g., for debug (warning - affects bss directly) */
/* TS_MESSAGE_STATIC_MEMORY define. */
#ifdef TS_MESSAGE_STATIC_MEMORY
static TsMessage_t _ts_message_nodes[TS_MESSAGE_MAX_NODES];
static bool _ts_message_nodes_initialized = false;
static int _ts_message_counter = 0;
#endif

/* forward references */
#ifdef TS_MESSAGE_STATIC_MEMORY
static TsStatus_t _ts_message_initialize();
#endif
static TsStatus_t _ts_message_set( TsMessageRef_t, TsPathNode_t, TsType_t, TsValue_t );
static TsStatus_t _ts_message_get( TsMessageRef_t, TsPathNode_t, TsType_t, TsValue_t );
static TsStatus_t _ts_message_encode_debug( TsMessageRef_t, int );
static TsStatus_t _ts_message_encode_json( TsMessageRef_t, uint8_t *, size_t );
static TsStatus_t _ts_message_encode_cbor( TsMessageRef_t, CborEncoder *, uint8_t *, size_t );
static TsStatus_t _ts_message_encode_ts_cbor( TsMessageRef_t, CborEncoder *, bool, uint8_t *, size_t );
static TsStatus_t _ts_message_decode_cbor( TsMessageRef_t, int, CborValue * );
static TsStatus_t _ts_message_decode_ts_cbor( TsMessageRef_t, int, CborValue * );

TsStatus_t ts_message_report() {
#ifdef TS_MESSAGE_STATIC_MEMORY
	ts_status_debug("report: counter, %d\n", _ts_message_counter);
	for (int i = 0; i < TS_MESSAGE_MAX_NODES; i++) {
		if (_ts_message_nodes[i].references > 0) {
			ts_status_debug("report: referenced node %d: %s has %d references\n",
					   i,
					   _ts_message_nodes[i].name,
					   _ts_message_nodes[i].references);
		}
	}
#endif
	return TsStatusOk;
}

/* ts_message_create */
TsStatus_t ts_message_create( TsMessageRef_t * message ) {
#ifdef TS_MESSAGE_STATIC_MEMORY
	/* initialize static memory system */
	if (!_ts_message_nodes_initialized) {
		_ts_message_initialize();
	}

	/* search for next free */
	for (int i = 0; i < TS_MESSAGE_MAX_NODES; i++) {

		if (_ts_message_nodes[i].references == 0) {

			/* mark as assigned */
			_ts_message_nodes[i].references = 1;

			/* clear all, assume root (avoiding memset) */
			snprintf(_ts_message_nodes[i].name, TS_MESSAGE_MAX_KEY_SIZE, "$root");
			_ts_message_nodes[i].type = TsTypeMessage;
			for (int j = 0; j < TS_MESSAGE_MAX_BRANCHES; j++) {
				_ts_message_nodes[i].value._xfields[j] = NULL;
			}

			/* set the return value (root) */
			*message = &_ts_message_nodes[i];
			_ts_message_counter++;

			/* return ok */
			return TsStatusOk;
		}
	}

	/* if none found, then clear the return value */
	*message = NULL;

	/* and return an out-of-memory error */
	ts_status_debug("ts_message_create: out of memory");
	return TsStatusErrorOutOfMemory;

#else

	*message = (TsMessageRef_t) ( malloc( sizeof( TsMessage_t )));

	memset( *message, 0x00, sizeof( TsMessage_t ));
	( *message )->references = 1;
	( *message )->type = TsTypeMessage;
	snprintf(( *message )->name, TS_MESSAGE_MAX_KEY_SIZE, "$root" );

	return TsStatusOk;
#endif
}

/* ts_message_create_message */
TsStatus_t ts_message_create_copy( TsMessageRef_t message, TsMessageRef_t * value ) {
	/* TODO - check depth, check message null */
	/* allocate a single message node */
	TsStatus_t status = ts_message_create( value );
	if( status == TsStatusOk ) {

		/* set the field relative to the given message to the new message */
		snprintf(( *value )->name, TS_MESSAGE_MAX_KEY_SIZE, "%s", message->name );
		( *value )->type = message->type;
		switch( message->type ) {
		case TsTypeInteger:
			( *value )->value._xinteger = message->value._xinteger;
			break;

		case TsTypeFloat:
			( *value )->value._xfloat = message->value._xfloat;
			break;

		case TsTypeBoolean:
			( *value )->value._xboolean = message->value._xboolean;
			break;

		case TsTypeString:
			snprintf(( *value )->value._xstring, TS_MESSAGE_MAX_STRING_SIZE, "%s", message->value._xstring );
			break;

		case TsTypeMessage:
		case TsTypeArray:
			for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
				if( message->value._xfields[ i ] == NULL) {
					break;
				}
				TsMessageRef_t field;
				status = ts_message_create_copy( message->value._xfields[ i ], &field );
				if( status != TsStatusOk ) {
					ts_message_destroy( *value );
					return status;
				}
				( *value )->value._xfields[ i ] = field;
			}
			break;

		case TsTypeNull:
		default:
			/* do nothing */
			break;
		}
	}

	/* return result */
	return status;
}

/* ts_message_create_message */
TsStatus_t ts_message_create_message( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	/* allocate a single message node */
	TsStatus_t status = ts_message_create( value );
	if( status == TsStatusOk ) {

		/* set the field relative to the given message to the new message */
		snprintf(( *value )->name, TS_MESSAGE_MAX_KEY_SIZE, "%s", field );
		( *value )->type = TsTypeMessage;
		status = _ts_message_set( message, field, TsTypeMessage, *value );

		/* since 'set' does not use reference counting (it copies instead), we need
		 * to clean up the created message and return the copied one (via 'get') */
		ts_message_destroy( *value );
		if( status != TsStatusOk ) {
			*value = NULL;
			return status;
		}
		ts_message_get( message, field, value );
	}

	/* return result */
	return status;
}

/* ts_message_create_array */
/* TODO - precreate array item type and size */
TsStatus_t ts_message_create_array( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	/* allocate a single message node */
	TsStatus_t status = ts_message_create( value );
	if( status == TsStatusOk ) {

		/* set the field relative to the given message to the new message */
		snprintf(( *value )->name, TS_MESSAGE_MAX_KEY_SIZE, "%s", field );
		( *value )->type = TsTypeArray;
		status = _ts_message_set( message, field, TsTypeArray, *value );

		/* since 'set' does not use reference counting (it copies instead), we need
		 * to clean up the created message and return the copied one (via 'get') */
		ts_message_destroy( *value );
		if( status != TsStatusOk ) {
			*value = NULL;
			return status;
		}
		ts_message_get( message, field, value );
	}

	/* return result */
	return status;
}

/* ts_message_destroy */
TsStatus_t ts_message_destroy( TsMessageRef_t message ) {
	/* check preconditions */
	if( message == NULL || message->references <= 0 ) {
		return TsStatusErrorPreconditionFailed;
	}

	/* simply change its status */
	message->references--;

	/* and destroy along with children */
	if( message->references <= 0 ) {

		if( message->type == TsTypeArray || message->type == TsTypeMessage ) {
			for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
				if( message->value._xfields[ i ] != NULL) {
					ts_message_destroy( message->value._xfields[ i ] );
				}
			}
		}
#ifdef TS_MESSAGE_STATIC_MEMORY
		message->references = 0;
		_ts_message_counter--;
		if (_ts_message_counter <= 0) {
			ts_status_debug("ts_message_destroy: all messages that had been created are now destroyed\n");
		}
#else
		free( message );
#endif
	}

	/* return ok */
	return TsStatusOk;
}

/**
 * Set the given field with the *contents* of the given value, i.e., it does not create a
 * grandchild of the message with the value name under the field (e.g., message->field->value.field)
 * but only a child (e.g., message->field)
 * @param message
 * The message to set the field on,...
 * @param field
 * The field name.
 * @param value
 * The value to set the message field too, note that the ownership is not transfered to the message,
 * instead a copy of the value is made and the value remains independent (and requires a seperate 'destroy')
 * @return
 * The status of the call as defined by ts_common.h
 */
TsStatus_t ts_message_set( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value ) {
	/* hold the type of the value, since set will force it to be TsTypeMessage */
	TsType_t type = value->type;

	/* find the best field (e.g., by name) and set that field to this value */
	TsStatus_t status = _ts_message_set( message, field, TsTypeMessage, value );
	/* note that this message doesnt take ownership of the given value, so subsequent
	 * destroys most occur on both the given value and this message in order to
	 * clean up memory allocated */

	/* reset the "new" field (i.e., the given pointer with references bumped by one) */
	/* to the correct type. */
	TsMessageRef_t copy;
	ts_message_get( message, field, &copy );
	copy->type = type;

	return status;
}

/* ts_message_set_null */
TsStatus_t ts_message_set_null( TsMessageRef_t message, TsPathNode_t field ) {
	return _ts_message_set( message, field, TsTypeNull, NULL);
}

/* ts_message_set_int */
TsStatus_t ts_message_set_int( TsMessageRef_t message, TsPathNode_t field, int value ) {
	return _ts_message_set( message, field, TsTypeInteger, &value );
}

/* ts_message_set_float */
TsStatus_t ts_message_set_float( TsMessageRef_t message, TsPathNode_t field, float value ) {
	return _ts_message_set( message, field, TsTypeFloat, &value );
}

/* ts_message_set_string */
TsStatus_t ts_message_set_string( TsMessageRef_t message, TsPathNode_t field, char * value ) {
	return _ts_message_set( message, field, TsTypeString, value );
}

/* ts_message_set_bool */
TsStatus_t ts_message_set_bool( TsMessageRef_t message, TsPathNode_t field, bool value ) {
	return _ts_message_set( message, field, TsTypeBoolean, &value );
}

/* ts_message_set_array */
TsStatus_t ts_message_set_array( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value ) {
	return _ts_message_set( message, field, TsTypeArray, value );
}

/* ts_message_set_message */
TsStatus_t ts_message_set_message( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t value ) {
	return _ts_message_set( message, field, TsTypeMessage, value );
}

/* ts_message_has */
TsStatus_t ts_message_has( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	if( message == NULL || field == NULL || value == NULL) {
		return TsStatusErrorPreconditionFailed;
	}
	if( message->type != TsTypeMessage ) {
		return TsStatusErrorPreconditionFailed;
	}
	for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
		TsMessageRef_t object = message->value._xfields[ i ];
		if( object == NULL) {
			return TsStatusErrorNotFound;
		}
		if( strcmp( object->name, field ) == 0 ) {
			*value = object;
			return TsStatusOk;
		}
	}
	return TsStatusErrorNotFound;
}

/* ts_message_get */
TsStatus_t ts_message_get( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	return ts_message_has( message, field, value );
}

/* ts_message_get_int */
TsStatus_t ts_message_get_int( TsMessageRef_t message, TsPathNode_t field, int * value ) {
	return _ts_message_get( message, field, TsTypeInteger, value );
}

/* ts_message_get_float */
TsStatus_t ts_message_get_float( TsMessageRef_t message, TsPathNode_t field, float * value ) {
	return _ts_message_get( message, field, TsTypeFloat, value );
}

/* ts_message_get_string */
TsStatus_t ts_message_get_string( TsMessageRef_t message, TsPathNode_t field, char ** value ) {
	return _ts_message_get( message, field, TsTypeString, value );
}

/* ts_message_get_bool */
TsStatus_t ts_message_get_bool( TsMessageRef_t message, TsPathNode_t field, bool * value ) {
	return _ts_message_get( message, field, TsTypeBoolean, value );
}

/* ts_message_get_array */
TsStatus_t ts_message_get_array( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	return _ts_message_get( message, field, TsTypeArray, value );
}

/* ts_message_get_message */
TsStatus_t ts_message_get_message( TsMessageRef_t message, TsPathNode_t field, TsMessageRef_t * value ) {
	return _ts_message_get( message, field, TsTypeMessage, value );
}

/* ts_message_get_size */
TsStatus_t ts_message_get_size( TsMessageRef_t array, size_t * size ) {
	/* check preconditions */
	if( array == NULL || array->type != TsTypeArray ) {
		return TsStatusErrorPreconditionFailed;
	}

	/* return last available position */
	/* TODO - should manage a cached length attribute instead */
	*size = TS_MESSAGE_MAX_BRANCHES;
	for( size_t i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
		if( array->value._xfields[ i ] == NULL) {
			*size = i;
			break;
		}
	}
	return TsStatusOk;
}

/* ts_message_get_at */
TsStatus_t ts_message_get_at( TsMessageRef_t array, size_t index, TsMessageRef_t * item ) {
	/* check preconditions */
	if( array == NULL || array->type != TsTypeArray ) {
		return TsStatusErrorPreconditionFailed;
	}
	if( index >= TS_MESSAGE_MAX_BRANCHES || array->value._xfields[ index ] == NULL) {
		return TsStatusErrorIndexOutOfRange;
	}

	/* return indexed value */
	*item = array->value._xfields[ index ];
	return TsStatusOk;
}

/* ts_message_set_at */
TsStatus_t ts_message_set_at( TsMessageRef_t array, size_t index, TsMessageRef_t item ) {
	/* check preconditions */
	if( array == NULL || array->type != TsTypeArray ) {
		return TsStatusErrorPreconditionFailed;
	}
	size_t length;
	ts_message_get_size( array, &length );
	if( index >= TS_MESSAGE_MAX_BRANCHES || index > length ) {
		return TsStatusErrorIndexOutOfRange;
	}

	/* note, passing NULL in item is the same as resizing the array */
	if( item == NULL && index < length - 1 ) {
		/* the caller should set the contents to NULL, not the item itself */
		return TsStatusErrorBadRequest;
	}

	/* remove old,... */
	TsMessageRef_t current = array->value._xfields[ index ];
	if( current != NULL) {
		array->value._xfields[ index ] = NULL;
		ts_message_destroy( current );
	}

	/* ...and set new and return */
	ts_message_create_copy( item, &current );
	array->value._xfields[ index ] = current;
	return TsStatusOk;
}

TsStatus_t ts_message_set_int_at( TsMessageRef_t array, size_t index, int value ) {
	TsMessage_t item = { .type = TsTypeInteger, .value._xinteger = value };
	return ts_message_set_at( array, index, &item );
}

TsStatus_t ts_message_set_float_at( TsMessageRef_t array, size_t index, float value ) {
	TsMessage_t item = { .type = TsTypeFloat, .value._xfloat = value };
	return ts_message_set_at( array, index, &item );
}

TsStatus_t ts_message_set_string_at( TsMessageRef_t array, size_t index, char * value ) {
	TsMessage_t item = { .type = TsTypeString };
	snprintf( item.value._xstring, TS_MESSAGE_MAX_STRING_SIZE, "%s", value );
	return ts_message_set_at( array, index, &item );
}

TsStatus_t ts_message_set_bool_at( TsMessageRef_t array, size_t index, bool value ) {
	TsMessage_t item = { .type = TsTypeBoolean, .value._xboolean = value };
	return ts_message_set_at( array, index, &item );
}

TsStatus_t ts_message_set_array_at( TsMessageRef_t array, size_t index, TsMessageRef_t value ) {
	return ts_message_set_at( array, index, value );
}

TsStatus_t ts_message_set_message_at( TsMessageRef_t array, size_t index, TsMessageRef_t value ) {
	return ts_message_set_at( array, index, value );
}

/* ts_message_encode */
/* encode will attempt to fill the given buffer with the encoded data found in the given message. */
TsStatus_t ts_message_encode( TsMessageRef_t message, TsEncoder_t encoder, uint8_t * buffer, size_t * buffer_size ) {

	/* check preconditions */
	if( message == NULL) {
		return TsStatusErrorPreconditionFailed;
	}
	if( buffer == NULL) {
		return TsStatusErrorBadRequest;
	}

	/* perform encoding */
	switch( encoder ) {
	case TsEncoderDebug:

		return _ts_message_encode_debug( message, 0 );

	case TsEncoderTsCbor: {

		CborEncoder cbor;
		cbor_encoder_init( &cbor, buffer, *buffer_size, 0 );
		TsStatus_t status = _ts_message_encode_ts_cbor( message, &cbor, true, buffer, *buffer_size );
		*buffer_size = cbor_encoder_get_buffer_size( &cbor, buffer );
		return status;
	}
	case TsEncoderJson: {

		memset( buffer, 0x00, *buffer_size );
		TsStatus_t status = _ts_message_encode_json( message, buffer, *buffer_size );
		*buffer_size = strlen((char *) buffer );
		return status;
	}
	case TsEncoderCbor: {

		CborEncoder cbor;
		cbor_encoder_init( &cbor, buffer, *buffer_size, 0 );
		TsStatus_t status = _ts_message_encode_cbor( message, &cbor, buffer, *buffer_size );
		*buffer_size = cbor_encoder_get_buffer_size( &cbor, buffer );
		return status;
	}
	default:

		/* do nothing */
		break;
	}
	return TsStatusErrorNotImplemented;
}

/* ts_message_set */
TsStatus_t ts_message_decode( TsMessageRef_t message, TsEncoder_t encoder, uint8_t * buffer, size_t buffer_size ) {

	/* check preconditions */
	if( message == NULL) {
		return TsStatusErrorPreconditionFailed;
	}

	/* perform encoding */
	switch( encoder ) {

	case TsEncoderJson: {

		if( buffer == NULL) {
			return TsStatusErrorBadRequest;
		}

		cJSON * cjson = cJSON_Parse((const char *) buffer );
		if( cjson->type == cJSON_Object) {
			cjson = cjson->child;
		}
		TsStatus_t status = ts_message_decode_json( message, cjson );
		cJSON_Delete( cjson );

		return status;
	}
	case TsEncoderCbor:
		/* not implemented */
		/* fallthrough */

	case TsEncoderTsCbor: {

		if( buffer == NULL) {
			return TsStatusErrorBadRequest;
		}

		CborParser parser;
		CborValue cbor;
		CborError error = cbor_parser_init( buffer, buffer_size, 0, &parser, &cbor );
		TsStatus_t status = _ts_message_decode_ts_cbor( message, 0, &cbor );

		return status;
	}
	case TsEncoderDebug:
	default:
		/* do nothing */
		break;
	}
	return TsStatusErrorNotImplemented;
}

/* ts_message_decode_json */
TsStatus_t ts_message_decode_json( TsMessageRef_t message, cJSON * value ) {

	/* decode each node in the current value */
	TsStatus_t status = TsStatusOk;
	while( value != NULL) {

		/* decode current node */
		switch( value->type ) {

		case cJSON_NULL:
			status = ts_message_set_null( message, value->string );
			break;

		case cJSON_Number:
			if( value->valuedouble == (double) ( value->valueint )) {
				status = ts_message_set_int( message, value->string, value->valueint );
			} else {
				status = ts_message_set_float( message, value->string, (float) ( value->valuedouble ));
			}
			break;

		case cJSON_String:
			status = ts_message_set_string( message, value->string, value->valuestring );
			break;

		case cJSON_True:
			status = ts_message_set_bool( message, value->string, true );
			break;

		case cJSON_False:
			status = ts_message_set_bool( message, value->string, false );
			break;

		case cJSON_Object: {

			TsMessageRef_t content;
			status = ts_message_create_message( message, value->string, &content );
			if( status == TsStatusOk ) {
				status = ts_message_decode_json( content, value->child );
			}
			break;
		}
		case cJSON_Array:
			/* TODO */
			/* fallthrough */

		case cJSON_Invalid:
		case cJSON_Raw:
		default:
			return TsStatusErrorNotImplemented;
		}

		/* get next sibling */
		value = value->next;
	}
	return status;
}

/* ts_message_decode_cbor */
TsStatus_t _ts_message_decode_cbor( TsMessageRef_t message, int depth, CborValue * value ) {
	/* TODO - _ts_message_decode_cbor not implemented */
	return TsStatusErrorNotImplemented;
}

/* ts_message_decode_json */
TsStatus_t _ts_message_decode_ts_cbor( TsMessageRef_t message, int depth, CborValue * value ) {

	// decode each node in the current value
	TsStatus_t status = TsStatusOk;
	do {

		bool get_next_sibling = false;
		CborError error;

		// get key if in object
		size_t key_size;
		char * key = NULL;
		if( depth > 0 ) {

			// check that the format of the message is correct
			if( !cbor_value_is_text_string( value ) ) {
				ts_status_alarm( "ts_message_decode_ts_cbor: malformed TS-CBOR\n" );
				status = TsStatusErrorBadRequest;
				break;
			}

			// notice this will advance to the next sibling
			cbor_value_copy_text_string( value, key, &key_size, value );
		}

		// decode value node
		CborType type = cbor_value_get_type( value );
		switch( type ) {
		case CborNullType:

			status = ts_message_set_null( message, key );
			get_next_sibling = true;
			break;

		case CborIntegerType: {

			int64_t data = 0;
			cbor_value_get_int64( value, &data);
			ts_message_set_int( message, key, (int)data );
			get_next_sibling = true;
			break;
		}
		case CborTextStringType: {

			size_t data_size;
			char * data = NULL;
			error = cbor_value_copy_text_string( value, data, &data_size, value );
			if( error ) {
				status = TsStatusErrorInternalServerError;
				break;
			}
			ts_message_set_string( message, key, data );
			free(data);
			break;
		}
		case CborByteStringType: {

			size_t data_size;
			uint8_t * data = NULL;
			error = cbor_value_dup_byte_string( value, &data, &data_size, value );
			if( error ) {
				status = TsStatusErrorInternalServerError;
				break;
			}
			// TODO - move data to message as base64 or other (e.g., UUID)
			ts_status_debug ("ts_message_decode_ts_cbor: byte string decoding not implemented\n" );
			ts_message_set_string( message, key, "not-saved" );
			free( data );
			break;
		}
		case CborBooleanType: {

			bool data;
			cbor_value_get_boolean( value, &data );
			ts_message_set_bool( message, key, data );
			get_next_sibling = true;
		}
		case CborDoubleType: {

			double data;
			cbor_value_get_double( value, &data );
			// TODO - check for possible loss of data
			ts_message_set_float( message, key, (float)data );
			get_next_sibling = true;
			break;
		}
		case CborFloatType: {

			float data;
			cbor_value_get_float( value, & data );
			ts_message_set_float( message, key, data );
			get_next_sibling = true;
			break;
		}
		case CborHalfFloatType:

			// TODO - half-float not implemented
			ts_status_alarm( "ts_message_decode_ts_cbor: half-float decoding not implemented\n" );
			status = TsStatusErrorNotImplemented;
			break;

		case CborArrayType:

			// TODO - array not implemented
			ts_status_alarm( "ts_message_decode_ts_cbor: array decoding not implemented\n" );
			status = TsStatusErrorNotImplemented;
			break;

		case CborMapType: {

			// recursive type
			CborValue recursed;
			error = cbor_value_enter_container( value, &recursed );
			if( error ) {
				ts_status_alarm( "ts_message_decode_ts_cbor: failed to open container\n" );
				status = TsStatusErrorInternalServerError;
			}

			TsMessageRef_t content;
			ts_message_create_message( message, key, &content );
			status = _ts_message_decode_ts_cbor( content, depth + 1, &recursed );
			if( status != TsStatusOk ) {
				ts_status_debug( "ts_message_decode_ts_cbor: recursion failed, %s\n", ts_status_string(status) );
			}

			error = cbor_value_leave_container( value, &recursed );
			if( error ) {
				ts_status_alarm( "ts_message_decode_ts_cbor: failed to close container\n" );
				status = TsStatusErrorInternalServerError;
			}
			break;
		}

		default:

			ts_status_alarm( "ts_message_decode_ts_cbor: unknown type encountered during decode, %d\n", value->type );
			status = TsStatusErrorNotImplemented;
			break;
		}

		// clean-up key
		free( key );

		// get next key sibling
		if( get_next_sibling ) {

			error = cbor_value_advance_fixed( value );
			if( error ) {
				ts_status_alarm( "ts_message_decode_ts_cbor: failed to advance, %d\n", error );
				status = TsStatusErrorInternalServerError;
			}
		}

	} while( !cbor_value_at_end( value ) && status == TsStatusOk );

	return status;
}

/* //////////////////////////////////////////////////////////////////////////// */
/* P R I V A T E */

#ifdef TS_MESSAGE_STATIC_MEMORY
/* (private) _ts_message_initialize */
static TsStatus_t _ts_message_initialize()
{
	/* report some basic statistics */
	ts_status_debug("initializing messaging, message_t size (%lu) preallocated message nodes (%d)\n", sizeof(TsMessage_t),
			   TS_MESSAGE_MAX_NODES);

	/* initialize message management system */
	for (int i = 0; i < TS_MESSAGE_MAX_NODES; i++) {

		/* just mark everything free */
		_ts_message_nodes[i].references = 0;
	}
	_ts_message_nodes_initialized = true;

	/* return ok */
	return TsStatusOk;
}
#endif

/**
 * Set the current message node to the given type and value. The optional field may be used to set a node relative
 * to the one given, e.g., as in a JSON object field.
 * @param message
 * The object of the action, set.
 * @param field
 * The optional field name, e.g., a JSON object field. If this value is NULL, then the given message node type and
 * value is set with the ones given. Otherwise, if this value isn't NULL, then the given message node is treated as
 * an object, and the value as the fields, where one field is named, typed and valued with the values provided.
 * @param type
 * The type of the message, e.g., TsTypeInteger, TsTypeFloat, etc.
 * @param value
 * The value of the message, e.g., int, float, etc. Warning, if the given value is a message or array type, the
 * value is copied (not reference counted), the caller will need to insure the value is deleted w/o.r.t. the given
 * message's eventual 'destroy'.
 * @return
 * The status of the call as defined by ts_common.h
 */
static TsStatus_t _ts_message_set( TsMessageRef_t message, TsPathNode_t field, TsType_t type, TsValue_t value ) {
	/* check preconditions */
	if( message == NULL || ( type != TsTypeNull && value == NULL)) {
		return TsStatusErrorPreconditionFailed;
	}

	/* search for the relevant node */
	/* normally assume we're adding or modifying a field, */
	/* however we will check for primitives during the first iteration */
	for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {

		/* check for primitives */
		TsMessageRef_t branch = message;
		if( field != NULL) {

			/* the path node is either new or has been established previously */
			branch = message->value._xfields[ i ];
			if( branch == NULL || strcmp( field, branch->name ) == 0 ) {

				/* destroy the old message if overwriting a new value */
				if( branch != NULL) {
					message->value._xfields[ i ] = NULL;
					ts_message_destroy( branch );
				}

				/* establish branch */
				switch( type ) {

				case TsTypeInteger:
				case TsTypeFloat:
				case TsTypeBoolean:
				case TsTypeString:
				case TsTypeNull: {

					/* (re)create a new messsage */
					TsStatus_t status = ts_message_create( &branch );
					if( status != TsStatusOk ) {
						ts_status_debug( "_ts_message_set: failed to create new primitive(%d)\n", status );
						return status;
					}
					break;
				}
				case TsTypeMessage:
				case TsTypeArray: {

					/* copy given messsage */
					TsStatus_t status = ts_message_create_copy((TsMessageRef_t) value, &branch );
					if( status != TsStatusOk ) {
						ts_status_debug( "_ts_message_set: failed to copy message or array(%d)\n", status );
						return status;
					}
					break;
				}
				default:

					ts_status_debug( "_ts_message_set: unknown type\n" );
					return TsStatusErrorBadRequest;
				}
			} else {
				continue;
			}

			/* (re)set this field array to the updated branch */
			message->value._xfields[ i ] = branch;
		}

		/* (re)set the field name and type */
		snprintf( branch->name, TS_MESSAGE_MAX_KEY_SIZE, "%s", field );
		branch->type = type;

		/* (re)set the field value */
		switch( type ) {

		case TsTypeInteger:

			branch->value._xinteger = *((int *) ( value ));
			break;

		case TsTypeFloat:

			branch->value._xfloat = *((float *) ( value ));
			break;

		case TsTypeBoolean:

			branch->value._xboolean = *((bool *) ( value ));
			break;

		case TsTypeString:

			snprintf( branch->value._xstring, TS_MESSAGE_MAX_STRING_SIZE, "%s", (char *) value );
			if( strlen( branch->value._xstring ) < strlen((char *) value )) {
				ts_status_debug( "issue detected during set (%s), string truncated; the given string is too large\n",
					field );
			}
			break;

		case TsTypeMessage:
		case TsTypeArray:
		case TsTypeNull:

			/* do nothing */
			break;
		}

		return TsStatusOk;
	}

	/* there isn't a branch available */
	ts_status_debug( "failed to set (%s), there are no additional nodes available\n", field );
	return TsStatusErrorPayloadTooLarge;
}

/* _ts_message_get */
static TsStatus_t _ts_message_get( TsMessageRef_t message, TsPathNode_t field, TsType_t type, TsValue_t value ) {
	TsMessageRef_t object;
	if( ts_message_has( message, field, &object ) == TsStatusOk ) {

		/* automatic type promotion */
		switch( object->type ) {

		case TsTypeInteger: {
			switch( type ) {
			case TsTypeInteger:
				*((int *) ( value )) = object->value._xinteger;
				return TsStatusOk;
			case TsTypeFloat:
				*((float *) ( value )) = (float) ( object->value._xinteger );
				return TsStatusOk;
			default:
				return TsStatusErrorPreconditionFailed;
			}
		}

		case TsTypeFloat: {
			switch( type ) {
			case TsTypeInteger:
				*((int *) ( value )) = (int) ( object->value._xfloat );
				return TsStatusOk;
			case TsTypeFloat:
				*((float *) ( value )) = object->value._xfloat;
				return TsStatusOk;
			default:
				return TsStatusErrorPreconditionFailed;
			}
		}

		default:
			/* do nothing */
			break;
		}

		/* strict type checks, no promotion */
		if( type != object->type ) {
			return TsStatusErrorPreconditionFailed;
		}
		switch( object->type ) {

		case TsTypeBoolean:
			*((bool *) ( value )) = object->value._xboolean;
			return TsStatusOk;

		case TsTypeString:
			*((char **) ( value )) = object->value._xstring;
			return TsStatusOk;

		case TsTypeMessage:
		case TsTypeArray:
			*((TsMessageRef_t *) ( value )) = object;
			return TsStatusOk;

		default:
			/* do nothing */
			break;
		}

		/* type not matched (e.g., null) */
		return TsStatusErrorPreconditionFailed;
	}

	/* field not found */
	return TsStatusErrorNotFound;
}

/* _ts_message_encode_none */
/* simple debug based 'encoder', display the structure of the message as it stands */
static TsStatus_t _ts_message_encode_debug( TsMessageRef_t message, int depth ) {
	/* pretty print (indent) */
	for( int i = 0; i < depth; i++ ) {
		ts_status_debug( "  " );
	}
	if( strlen( message->name ) > 0 ) {
		ts_status_debug( "%s", message->name );
	}

	/* display type and value */
	switch( message->type ) {
	case TsTypeNull:
		ts_status_debug( ":NULL\n" );
		break;

	case TsTypeInteger:
		ts_status_debug( ":integer( %d )\n", message->value._xinteger );
		break;

	case TsTypeFloat:
		ts_status_debug( ":float( %f )\n", message->value._xfloat );
		break;

	case TsTypeBoolean:
		ts_status_debug( ":boolean( %u )\n", message->value._xboolean );
		break;

	case TsTypeString:
		ts_status_debug( ":string( %s )\n", message->value._xstring );
		break;

	case TsTypeArray: {
		ts_status_debug( ":array\n" );
		for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			TsMessageRef_t branch = message->value._xfields[ i ];
			if( branch == NULL) {
				break;
			}
			for( int j = 0; j < depth; j++ ) {
				ts_status_debug( "  " );
			}
			ts_status_debug( "[%d] = {\n", i );
			_ts_message_encode_debug( branch, depth + 1 );
			for( int j = 0; j < depth; j++ ) {
				ts_status_debug( "  " );
			}
			ts_status_debug( "}\n" );
		}
		break;
	}
	case TsTypeMessage: {
		ts_status_debug( ":message\n" );
		for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			TsMessageRef_t branch = message->value._xfields[ i ];
			if( branch == NULL) {
				break;
			}
			_ts_message_encode_debug( branch, depth + 1 );
		}
		break;
	}
	default:
		ts_status_debug( ":unknown\n" );
		break;
	}
	return TsStatusOk;
}

/* _ts_message_encode_json */
static TsStatus_t _ts_message_encode_json( TsMessageRef_t message, uint8_t * buffer, size_t buffer_size ) {
	/* re-point buffer append */
	/* TODO - check for negative sizes, etc. */
	char * xbuffer = (char *) buffer;
	size_t xbuffer_size = buffer_size - strlen( xbuffer );
	xbuffer = xbuffer + strlen( xbuffer );

	/* display type and value */
	switch( message->type ) {
	case TsTypeNull:
		snprintf( xbuffer, xbuffer_size, "null" );
		break;

	case TsTypeInteger:
		snprintf( xbuffer, xbuffer_size, "%d", message->value._xinteger );
		break;

	case TsTypeFloat:
		snprintf( xbuffer, xbuffer_size, "%f", message->value._xfloat );
		break;

	case TsTypeBoolean:
		snprintf( xbuffer, xbuffer_size, "%s", message->value._xboolean ? "true" : "false" );
		break;

	case TsTypeString:
		snprintf( xbuffer, xbuffer_size, "\"%s\"", message->value._xstring );
		break;

	case TsTypeArray: {
		snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "[" );
		for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			TsMessageRef_t branch = message->value._xfields[ i ];
			if( branch == NULL) {
				break;
			}
			if( i > 0 ) {
				/* TODO - check for negative sizes, etc. */
				snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "," );
			}
			switch( branch->type ) {
			case TsTypeArray:
				snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "[" );
				_ts_message_encode_json( branch, buffer, buffer_size );
				snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "]" );
				break;
			default:
				_ts_message_encode_json( branch, buffer, buffer_size );
				break;
			}
		}
		snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "]" );
		break;
	}
	case TsTypeMessage: {
		snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "{" );
		for( int i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			TsMessageRef_t branch = message->value._xfields[ i ];
			if( branch == NULL) {
				break;
			} else {
				/* TODO - check for negative sizes, etc. */
				if( i > 0 ) {
					snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "," );
				}
				snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "\"%s\":", branch->name );
				_ts_message_encode_json( branch, buffer, buffer_size );
			}
		}
		snprintf( xbuffer + strlen( xbuffer ), xbuffer_size - strlen( xbuffer ), "}" );
		break;
	}
	default:
		return TsStatusErrorInternalServerError;
	}

	/* check if we've used up the buffer */
	/* note, snprintf is safe, it wont overwrite the given buffer size */
	/* note also, the buffer_size includes the terminating null */
	if( strlen((char *) buffer ) >= ( buffer_size - 1 )) {
		return TsStatusErrorOutOfMemory;
	}
	return TsStatusOk;
}

/* _ts_message_encode_cbor */
static TsStatus_t _ts_message_encode_cbor( TsMessageRef_t message, CborEncoder * encoder, uint8_t * buffer, size_t buffer_size ) {

	/* display type and value */
	switch( message->type ) {
	case TsTypeNull:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_null( encoder );
		break;

	case TsTypeInteger:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_int( encoder, message->value._xinteger );
		break;

	case TsTypeFloat:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_float( encoder, message->value._xfloat );
		break;

	case TsTypeBoolean:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_boolean( encoder, message->value._xboolean );
		break;

	case TsTypeString:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_text_stringz( encoder, message->value._xstring );
		break;

	case TsTypeArray: {
		/* TODO - add array */
		return TsStatusErrorNotImplemented;
	}
	case TsTypeMessage: {

		/* tag if not root */
		if( strcmp( message->name, "$root" ) != 0 ) {
			cbor_encode_text_stringz( encoder, message->name );
		}

		/* determine size of map */
		/* TODO - avoid search, hold current free index in root */
		size_t length = TS_MESSAGE_MAX_BRANCHES;
		for( size_t i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			if( message->value._xfields[ i ] == NULL) {
				length = i;
				break;
			}
		}

		/* create and fill map */
		CborEncoder map;
		cbor_encoder_create_map( encoder, &map, length );
		for( int i = 0; i < length; i++ ) {
			_ts_message_encode_cbor( message->value._xfields[ i ], &map, buffer, buffer_size );
		}
		cbor_encoder_close_container( encoder, &map );
		break;
	}
	default:
		return TsStatusErrorInternalServerError;
	}

	/* check if we've used up the buffer */
	/* TODO - need to double check to see if cbor-encode functions can overrun buffer (dont think they do tho) */
	if( cbor_encoder_get_buffer_size( encoder, buffer ) >= buffer_size ) {
		return TsStatusErrorOutOfMemory;
	}
	return TsStatusOk;
}

typedef enum {
	TsCborValueTypeDefault,
	TsCborValueTypeUUID,
	TsCborValueTypeKind,
	TsCborValueTypeAction,
} TsCborValueType_t;

typedef struct {
	char *              name;
	int                 value;
	TsCborValueType_t   type;
} TsCborKeyMapping_t;

static TsCborKeyMapping_t _ts_cbor_key_mapping[] = {
	{ "id",             1, TsCborValueTypeUUID },
	{ "transactionid",  2, TsCborValueTypeUUID },
	{ "kind",           3, TsCborValueTypeKind },
	{ "version",        4, TsCborValueTypeDefault },
	{ "action",         5, TsCborValueTypeAction },
	{ "fields",         6, TsCborValueTypeDefault },
	{ "status",         7, TsCborValueTypeDefault },
	{ "error",          8, TsCborValueTypeDefault },
	{ "createdon",      9, TsCborValueTypeDefault },
};

static TsStatus_t _ts_message_encode_ts_cbor_key( CborEncoder * encoder, bool is_root, char * name, TsCborValueType_t * type ) {

	if( is_root ) {

		bool found = false;
		for( int i = 0; i < sizeof(_ts_cbor_key_mapping); i++ ) {
			if( strcmp( name, _ts_cbor_key_mapping[i].name ) == 0 ) {
				found = true;
				cbor_encode_int( encoder, _ts_cbor_key_mapping[i].value );
				*type = _ts_cbor_key_mapping[i].type;
				break;
			}
		}
		if( !found ) {
			ts_status_alarm( "ts_message_encode_ts_cbor: no mapping found for root attribute, %s, ignoring,...\n", name );
			cbor_encode_text_stringz( encoder, name );
		}

	} else {
		cbor_encode_text_stringz( encoder, name );
	}
	return TsStatusOk;
}

static TsStatus_t _ts_message_encode_ts_cbor_value( CborEncoder * encoder, bool is_root, char * value, TsCborValueType_t type ) {

	if( is_root ) {

		switch( type ) {
		case TsCborValueTypeUUID: {

			int uuid_size = 16;
			uint8_t uuid[ 16 ];
			if( strlen(value) != TS_MESSAGE_UUID_SIZE ) {
				ts_status_alarm( "ts_message_encode_ts_cbor: no mapping found for given UUID, %s, ignoring,...\n", value );
				cbor_encode_text_stringz( encoder, value );
			} else {
				int value_index = 0;
				int uuid_index = 0;
				while( ( uuid_index < uuid_size ) && ( value_index < TS_MESSAGE_UUID_SIZE ) ) {

					if( ( value[value_index] >= '0' && value[value_index] <= '9' ) &&
						( value[value_index+1] >= '0' && value[value_index+1] <= '9' ) ) {

						uint8_t high = (uint8_t)(value[value_index] - '0');
						uint8_t low = (uint8_t)(value[value_index+1] - '0');
						uuid[uuid_index] = high * (uint8_t)16 + low;
						uuid_index = uuid_index + 1;

					} else if(
						( value[value_index] == '-' ) &&
						( value[value_index+1] >= '0' && value[value_index+1] <= '9' ) ) {

						value_index = value_index + 1;

					} else {

						ts_status_alarm( "ts_message_encode_ts_cbor: no mapping found for malformed UUID, %s, ignoring,...\n", value );
						cbor_encode_text_stringz( encoder, value );
						return TsStatusOk;
					}
				};
				cbor_encode_byte_string( encoder, uuid, (size_t)uuid_size );
			}
			break;
		}

		case TsCborValueTypeKind:
			if( strcmp( value, "ts.element" ) == 0 ) {
				cbor_encode_int( encoder, 1 );
			} else if( strcmp( value, "ts.event" ) == 0 ) {
				cbor_encode_int( encoder, 2 );
			} else if( strcmp( value, "ts.event.diagnostic" ) == 0 ) {
				cbor_encode_int( encoder, 3 );
			} else {
				ts_status_alarm( "ts_message_encode_ts_cbor: no mapping found for malformed Kind, %s, ignoring,...\n", value );
				cbor_encode_text_stringz( encoder, value );
			}
			break;
		case TsCborValueTypeAction:
			if( strcmp( value, "activate" ) == 0 ) {
				cbor_encode_int( encoder, 1 );
			} else if( strcmp( value, "suspend" ) == 0 ) {
				cbor_encode_int( encoder, 2 );
			} else if( strcmp( value, "resume" ) == 0 ) {
				cbor_encode_int( encoder, 3 );
			} else if( strcmp( value, "deactivate" ) == 0 ) {
				cbor_encode_int( encoder, 4 );
			} else if( strcmp( value, "get" ) == 0 ) {
				cbor_encode_int( encoder, 5 );
			} else if( strcmp( value, "set" ) == 0 ) {
				cbor_encode_int( encoder, 6 );
			} else if( strcmp( value, "update" ) == 0 ) {
				cbor_encode_int( encoder, 7 );
			} else {
				ts_status_alarm( "ts_message_encode_ts_cbor: no mapping found for malformed Action, %s, ignoring,...\n", value );
				cbor_encode_text_stringz( encoder, value );
			}
			break;

		default:
		case TsCborValueTypeDefault:
			cbor_encode_text_stringz( encoder, value );
			break;
		}
	} else {
		cbor_encode_text_stringz( encoder, value );
	}
	return TsStatusOk;
}

static TsStatus_t _ts_message_encode_ts_cbor( TsMessageRef_t message, CborEncoder * encoder, bool is_root, uint8_t * buffer, size_t buffer_size ) {

	TsCborValueType_t type;
	switch( message->type ) {
	case TsTypeNull:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_null( encoder );
		break;

	case TsTypeInteger:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_int( encoder, message->value._xinteger );
		break;

	case TsTypeFloat:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_float( encoder, message->value._xfloat );
		break;

	case TsTypeBoolean:
		cbor_encode_text_stringz( encoder, message->name );
		cbor_encode_boolean( encoder, message->value._xboolean );
		break;

	case TsTypeString:
		_ts_message_encode_ts_cbor_key( encoder, is_root, message->name, &type );
		_ts_message_encode_ts_cbor_value( encoder, is_root, message->value._xstring, type );
		break;

	case TsTypeArray: {
		/* TODO - add array */
		return TsStatusErrorNotImplemented;
	}
	case TsTypeMessage: {

		/* tag if not root */
		if( strcmp( message->name, "$root" ) != 0 ) {
			_ts_message_encode_ts_cbor_key( encoder, is_root, message->name, &type );
		}

		/* determine size of map */
		/* TODO - avoid search, hold current free index in root */
		size_t length = TS_MESSAGE_MAX_BRANCHES;
		for( size_t i = 0; i < TS_MESSAGE_MAX_BRANCHES; i++ ) {
			if( message->value._xfields[ i ] == NULL) {
				length = i;
				break;
			}
		}

		/* create and fill map */
		CborEncoder map;
		cbor_encoder_create_map( encoder, &map, length );
		for( int i = 0; i < length; i++ ) {
			_ts_message_encode_ts_cbor( message->value._xfields[ i ], &map, true, buffer, buffer_size );
		}
		cbor_encoder_close_container( encoder, &map );
		break;
	}
	default:
		return TsStatusErrorInternalServerError;
	}

	/* check if we've used up the buffer */
	/* TODO - need to double check to see if cbor-encode functions can overrun buffer (dont think they do tho) */
	if( cbor_encoder_get_buffer_size( encoder, buffer ) >= buffer_size ) {
		return TsStatusErrorOutOfMemory;
	}
	return TsStatusOk;
}
