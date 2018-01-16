// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include "ts_platform.h"
#include "ts_status.h"
#include "ts_profile.h"

TsStatus_t ts_profile_create( TsProfileRef_t * profile ) {
	ts_status_trace("ts_profile_create");
	ts_platform_assert(profile != NULL);
	return ts_message_create( profile );
}

TsStatus_t ts_profile_destroy( TsProfileRef_t profile ) {
	ts_status_trace("ts_profile_destroy");
	ts_platform_assert(profile != NULL);
	return ts_message_destroy( profile );
}

TsStatus_t ts_profile_inc_int( TsProfileRef_t profile, TsPathNode_t field) {
	ts_status_trace("ts_profile_int_inc");
	ts_platform_assert(profile != NULL);

	int value = 0;
	ts_message_get_int(profile, field, &value );

	value = value + 1;
	return ts_message_set_int( profile, field, value );
}

TsStatus_t ts_profile_dec_int( TsProfileRef_t profile, TsPathNode_t field) {
	ts_status_trace("ts_profile_int_inc");
	ts_platform_assert(profile != NULL);

	int value = 0;
	ts_message_get_int(profile, field, &value );

	value = value - 1;
	return ts_message_set_int( profile, field, value );
}

TsStatus_t ts_profile_set_int( TsProfileRef_t profile, TsPathNode_t field, int value) {
	ts_status_trace("ts_profile_int_inc");
	ts_platform_assert(profile != NULL);
	return ts_message_set_int( profile, field, value );
}

TsStatus_t ts_profile_get_int( TsProfileRef_t profile, TsPathNode_t field, int* value) {
	ts_status_trace("ts_profile_int_inc");
	ts_platform_assert(profile != NULL);
	return ts_message_get_int( profile, field, value );
}

