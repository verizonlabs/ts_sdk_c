/**
 * @file
 * ts_profile.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * Used to instrument the SDK, esp. for the sake of diagnostics.
 *
 * @details
 * The profile utility class provides runtime counter, latency (TBD) and
 * frequency (TBD) data types used to measure profiling information, e.g.,
 * diagnostics.
 */
#ifndef TS_PROFILE_H
#define TS_PROFILE_H

#include "ts_status.h"
#include "ts_message.h"

/**
 * The profile object reference
 */
typedef struct TsMessage * TsProfileRef_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate and initialize a new profile object.
 *
 * @param profile
 * [on/out] The pointer to a pre-existing TsProfileRef_t, which will be initialized with the profile state.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_profile_create( TsProfileRef_t * );

/**
 * Deallocate the given profile object.
 *
 * @param profile
 * [in] The profile state.
 *
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_profile_destroy( TsProfileRef_t );

TsStatus_t ts_profile_inc_int(TsProfileRef_t, TsPathNode_t);
TsStatus_t ts_profile_dec_int(TsProfileRef_t, TsPathNode_t);
TsStatus_t ts_profile_set_int(TsProfileRef_t, TsPathNode_t, int);
TsStatus_t ts_profile_get_int(TsProfileRef_t, TsPathNode_t, int*);

#ifdef __cplusplus
}
#endif

#endif // TS_PROFILE_H
