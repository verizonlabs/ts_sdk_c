/**
 * @file
 * ts_suspend.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * An interface for suspending and resuming Operational Device Security features.
 *
 * @details
 * We have fields for suspending/resuming the firewall and the logging.
 *
 */

#ifndef TS_SUSPEND_H_
#define TS_SUSPEND_H_

#include "ts_status.h"
#include "ts_message.h"

/**
 * Handle a version query message.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_handle(TsMessageRef_t);

/**
 * Is the firewall suspended?
 * @return
 * True if the firewall is suspended, false otherwise.
 */
bool ts_firewall_suspended();

/**
 * Is logging suspended?
 * @return
 * True if logging is suspended, false otherwise.
 */
bool ts_logconfig_suspended();

#endif /* TS_SUSPEND_H_ */
