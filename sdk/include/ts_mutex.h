/**
 * @file
 * ts_mutex.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The mutex interface to abstract the OS-centric implementation of thread synchronization.
 *
 * @details
 * The mutex is rarely implemented, but is required if one or many components of the SDK
 * are threaded. This can happen if the developer customizes a particular component that
 * requires OS-agnostic thread synchronization.
 */
#ifndef TS_MUTEX_H
#define TS_MUTEX_H

#include "ts_status.h"

/**
 * The mutex object reference
 */
typedef struct TsMutex *TsMutexRef_t;

/**
 * The mutex object
 */
typedef struct TsMutex {} TsMutex_t;

/**
 * The mutex vector table (i.e., the mutex "class" definition), used to define the mutex SDK-aspect.
 * See examples/platforms for available mutex implementations, or use your own customized implementation.
 */
typedef struct TsMutexVtable {

	/**
	 * Allocate and initialize a new mutex object.
	 *
	 * @param mutex
	 * [on/out] The pointer to a pre-existing TsMutexRef_t, which will be initialized with the mutex state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*create)(TsMutexRef_t *);

	/**
	 * Deallocate the given mutex object.
	 *
	 * @param mutex
	 * [in] The mutex state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*destroy)(TsMutexRef_t);

	/**
	 * Lock the mutex, block the thread if the mutex has already been locked by another thread.
	 *
	 * @param mutex
	 * [in] The mutex state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*lock)(TsMutexRef_t);

	/**
	 * Unlock the mutex, signal blocked threads currently contending for the same lock.
	 *
	 * @param mutex
	 * [in] The mutex state.
	 *
	 * @return
	 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
	 * - TsStatusOk
	 * - TsStatusError[Code]
	 */
	TsStatus_t (*unlock)(TsMutexRef_t);
} TsMutexVtable_t;

extern const TsMutexVtable_t * ts_mutex;

#define ts_mutex_create         ts_mutex->create
#define ts_mutex_destroy		ts_mutex->destroy
#define ts_mutex_lock   		ts_mutex->lock
#define ts_mutex_unlock		    ts_mutex->unlock

#endif // TS_MUTEX_H
