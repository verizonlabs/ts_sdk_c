// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_PROFILE_H
#define TS_PROFILE_H

#include "ts_status.h"
#include "ts_message.h"

typedef struct TsMessage * TsProfileRef_t;

#ifdef __cplusplus
extern "C" {
#endif

TsStatus_t ts_profile_create( TsProfileRef_t * );
TsStatus_t ts_profile_destroy( TsProfileRef_t );

TsStatus_t ts_profile_inc_int(TsProfileRef_t, TsPathNode_t);
TsStatus_t ts_profile_dec_int(TsProfileRef_t, TsPathNode_t);
TsStatus_t ts_profile_set_int(TsProfileRef_t, TsPathNode_t, int);
TsStatus_t ts_profile_get_int(TsProfileRef_t, TsPathNode_t, int*);

#ifdef __cplusplus
}
#endif

#endif // TS_PROFILE_H
