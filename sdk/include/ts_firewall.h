// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_FIREWALL_H
#define TS_FIREWALL_H

#include "ts_status.h"
#include "ts_message.h"

typedef struct TsFirewall * TsFirewallRef_t;
typedef struct TsFirewall {

} TsFirewall_t;


typedef struct TsFirewallVtable {

	TsStatus_t (*create)(TsFirewallRef_t *);
	TsStatus_t (*destroy)(TsFirewallRef_t);
	TsStatus_t (*tick)(TsFirewallRef_t, uint32_t);

	TsStatus_t (*set)(TsFirewallRef_t, TsMessageRef_t );
	TsStatus_t (*get)(TsFirewallRef_t, TsMessageRef_t );
	TsStatus_t (*remove)(TsFirewallRef_t, TsMessageRef_t );

} TsFirewallVtable_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const TsFirewallVtable_t *ts_firewall;

#define ts_firewall_create      ts_firewall->create
#define ts_firewall_destroy     ts_firewall->destroy
#define ts_firewall_tick        ts_firewall->tick
#define ts_firewall_diagnostics ts_firewall->diagnostics

#define ts_firewall_set         ts_firewall->set
#define ts_firewall_get         ts_firewall->get
#define ts_firewall_remove      ts_firewall->remove

#ifdef __cplusplus
}
#endif

#endif // TS_FIREWALL_H
