/**
 * @file
 * ts_version.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * An interface for getting version information.
 *
 * @details
 * We have three strings: the TS-SDK version, the ODS version, and the hardware version. The latter is manufacturer-dependent.
 *
 */

#ifndef TS_VERSION_H_
#define TS_VERSION_H_

#include "ts_status.h"
#include "ts_message.h"

#define TS_SDK_VERSION "2.0.0"
#define TS_ODS_VERSION "1.0.0"
#define TS_HARDWARE_VERSION "1.0"

/**
 * Handle a version query message.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_version_handle(TsMessageRef_t);

/**
 * Create an update message containing version info.
 * @param new
 * [out] Pointer to a TsMessageRef_t that will point to the new message.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_version_make_update(TsMessageRef_t *);
#endif /* TS_VERSION_H_ */
