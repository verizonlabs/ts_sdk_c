// Copyright (C) 2017, Verizon, Inc. All rights reserved.
#include "ts_mutex.h"
#include "ts_firewall.h"

// optional components are expected to be set to NULL if they are not
// being used by the platform.

// optional mutex implementation for threaded applications
#if defined(TS_MUTEX_NONE)
const TsMutexVtable_t * ts_mutex = NULL;
#endif

// optional firewall implementation for networked implementations
#if defined(TS_FIREWALL_NONE)
const TsFirewallVtable_t * ts_firewall = NULL;
#endif
