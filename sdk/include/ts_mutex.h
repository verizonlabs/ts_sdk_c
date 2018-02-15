// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#ifndef TS_MUTEX_H
#define TS_MUTEX_H

#include "ts_status.h"

typedef struct TsMutex *TsMutexRef_t;
typedef struct TsMutex {} TsMutex_t;

typedef struct TsMutexVtable {
	TsStatus_t (*create)(TsMutexRef_t *);
	TsStatus_t (*destroy)(TsMutexRef_t);
	TsStatus_t (*lock)(TsMutexRef_t);
	TsStatus_t (*unlock)(TsMutexRef_t);
} TsMutexVtable_t;

extern const TsMutexVtable_t * ts_mutex;

#define ts_mutex_create         ts_mutex->create
#define ts_mutex_destroy		ts_mutex->destroy
#define ts_mutex_lock   		ts_mutex->lock
#define ts_mutex_unlock		    ts_mutex->unlock

#endif // TS_MUTEX_H
