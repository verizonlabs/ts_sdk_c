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

//#define TEST_SUSPEND

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

#ifdef TEST_SUSPEND
/**
 * Generate and handle a test message.
 * @param firewall true = suspend firewall, false = resume firewall
 * @param logconfig true = suspend logconfig, false = resume logconfig
 * @return
 */
TsStatus_t ts_suspend_test(bool firewall, bool logconfig);
#endif

#endif /* TS_SUSPEND_H_ */
