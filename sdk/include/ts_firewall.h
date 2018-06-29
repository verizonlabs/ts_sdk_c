/**
 * @file
 * ts_firewall.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * A firewall connectivity service interface.
 *
 * @details
 * The "firewall" connectivity service interface is used by licenced ThingSpace devices to manage local
 * firewalls from the cloud. Implementation is specific to the OS and/or hardware platform.
 *
 * @note
 * A developer would not implement this interface, it is a Verizon specific service that
 * is only enabled from the network and billing systems.  Custom implementations may be
 * supported in the future.
 */
#ifndef TS_FIREWALL_H
#define TS_FIREWALL_H

#include "ts_status.h"
#include "ts_message.h"
#include "ts_log.h"

/**
 * The firewall object reference
 */
typedef struct TsFirewall * TsFirewallRef_t;

/**
 * The firewall object.
 */

typedef struct TsFirewall {
	bool _enabled;
	bool _suspended;
	TsMessageRef_t _default_rules;
	TsMessageRef_t _default_domains;
	TsMessageRef_t _rules;
	TsMessageRef_t _domains;
	uint64_t _statistics_reporting_interval;
	uint64_t _last_report_time;
} TsFirewall_t;

/**
 * Contains any state information needed for processing firewall packet events.
 */
typedef struct TsCallbackContext {
	bool alerts_enabled;
	bool alert_in_progress;
	TsMessageRef_t alert_to_send;
	int alert_threshold_inbound;
	int alert_threshold_outbound;
	int inbound_rejections;
	int outbound_rejections;
	TsStatus_t (*alert_callback) (TsMessageRef_t, char *);
	TsLogConfigRef_t log;
} TsCallbackContext_t;

/**
 * The firewall vector table (i.e., the firewall "class" definition), used to define the firewall SDK-aspect.
 * See examples/platforms for available firewalls, or use your own customized implementation.
 */
typedef struct TsFirewallVtable {

	/**
	 * Allocate and initialize a new firewall object.
	 *
	 * @param firewall
	 * [on/out] The pointer to a pre-existing TsFirewallRef_t, which will be initialized with the firewall state.
	 *
	 * @param alertCallback
	 * [in] Pointer to a function that will send an alert message with a designated kind.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*create)(TsFirewallRef_t *, TsStatus_t (*alertCallback)(TsMessageRef_t, char *));

	/**
	 * Deallocate the given firewall object.
	 *
	 * @param firewall
	 * [in] The firewall state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*destroy)(TsFirewallRef_t);

	/**
	 * Provide the given firewall processing time according to the given budget "recommendation".
	 * This function is typically called from ts_service.
	 *
	 * @param firewall
	 * [in] The firewall state.
	 *
	 * @param budget
	 * [in] The recommended time in microseconds budgeted for the function
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*tick)(TsFirewallRef_t, uint32_t);

	/**
	 * Process the given firewall message.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*handle)(TsFirewallRef_t, TsMessageRef_t );

	/**
	 * Register a logging instance for the firewall to use when logging events.
	 *
	 * @param log_config
	 * The TsLogConfigRef_t pointing to the logging configuration to use.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*set_log)(TsLogConfigRef_t);

	/**
	* Get firewall Stats
	* @return
	* The collection of statistics for the firewall
	*/
	TsMessageRef_t (*stats)();

	/**
	 * Suspend/resume the firewall.
	 *
	 * @param firewall
	 * [in] TsFirewallRef_t referring to the firewall to suspend/resume.
	 * @param suspend
	 * [in] True if the firewall is being suspended, false if it is being resumed.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*set_suspended)( TsFirewallRef_t, bool );

	/**
	 * Query whether the firewall is suspended.
	 *
	 * @param firewall
	 * [in] TsFirewallRef_t referring to the firewall state.
	 *
	 * @return
	 * True if the firewall is suspended, false if not.
	 */
	bool (*suspended)( TsFirewallRef_t );

} TsFirewallVtable_t;


/**
 * Set the firewall that we'll be suspending/resuming.
 * @param firewall
 * [in] The firewall that we'll be suspending/resuming.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_suspend_set_firewall(TsFirewallRef_t);


#ifdef __cplusplus
extern "C" {
#endif

extern const TsFirewallVtable_t *ts_firewall;

#define ts_firewall_create      	ts_firewall->create
#define ts_firewall_destroy     	ts_firewall->destroy
#define ts_firewall_tick        	ts_firewall->tick
#define ts_firewall_handle      	ts_firewall->handle
#define ts_firewall_set_log			ts_firewall->set_log
#define ts_firewall_stats			ts_firewall->stats
#define ts_firewall_set_suspended	ts_firewall->set_suspended
#define ts_firewall_suspended		ts_firewall->suspended

#ifdef __cplusplus
}
#endif

#endif // TS_FIREWALL_H
