 /*
 * ts_scep.h
 *
 *  Created on: Aug 28, 2018
 *      Author: LArry Ciummo
 */

#ifndef ts_scep_H
#define ts_scep_H
/**
 * @file
 * ts_scep.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The OS-agnostic abstraction of for access to the SCEP certificate enrollment protocol functions.
 * @details
 * The SDK uses SCEP protocol to maintain and updated X.509 certificates on the the device.
 *
 * @note
 * Components (e.g., ts_security, etc.) are simple vector-table (vtable) objects initialized according a
 * configuration chosen at compile time, or (in some rare use-case scenarios) runtime. They provide the
 * behavioral aspect of the particular component, allowing the developer to create and destroy objects of
 * that type, and act on those objects according to the design of the component (e.g., connect, disconect, etc.)
 */


#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include "ts_status.h"
#include "ts_cert.h"


 * The file object reference
 */
typedef struct TsScep *TsScepRef_t;
e;

/**
 * The file object
 */
typedef struct TsScep {


} TsScep_t;

/**
 * The file vector table (i.e., the file "class" definition), used to define the file SDK-aspect.
 * See examples/platforms for available platform implementations, or use your own customized implementation.
 */
typedef struct TsScepVtable {

	/**
	 * Initialize the storage device (flash) and the file system
	 */
	void		(*initialize)();


	/**
	 * Create a directory on the file system
	 */
	TsStatus_t 		(*enroll) (TsScepConfigRef_t config);



	/**
	 * Handle any assertion, i.e., this function doesnt perform the check, it simply performs the effect, e.g.,
	 * display the given message and halt, etc.
	 */
     void		(*assertion)(const char *msg, const char *file, int line);


} TsScepVtable_t;


extern const TsScepVtable_t *ts_scep;

#define noerror 			xxx->directory_delete

#define ts_scep_enroll			ts_scep->enroll

#ifdef NDEBUG
#define ts_scep_assert(EX) (void)0
#else
#define ts_scep_assert(EX)  (void)((EX) || (ts_scep->assertion(#EX,__FILE__,__LINE__),0))
#endif

#endif // ts_scep_H


