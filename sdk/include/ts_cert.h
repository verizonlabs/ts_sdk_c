/**
 * @file
 * ts_cert.h
 *
 * @copyright
 * Copyright (C) 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * An interface for getting cert information.
 *
 * @details
 * We have two strings: the TS-SDK cert, TS-SDK key.
 *
 */

#ifndef TS_CERT_H_
#define TS_CERT_H_

#include "ts_status.h"
#include "ts_message.h"

/**
 * The scep config reference
 */
typedef struct TsScepConfig * TsScepConfigRef_t;

/**
 * The scep config object.
 */

typedef struct TsScepConfig {
	bool _enabled;			// Specifies if certificate auto-renewal is enable/disable on the device
	bool _generateNewPrivateKey;
	int _certExpiresAfter;		// Specifies the datetime, in ISO 8601 format, when this certificates expires.
	int _certEnrollmentType;	// Specifies the protocol used to distribute the device certificate(SCEP/EST)
	int _numDaysBeforeAutoRenew;	// Specifies the number of days before expiration where the certificate should be renewed
	char *_encryptionAlgorithm;  	// Specifies the encryption algorithm
	char *_hashFunction; 		// Specifies the hash function to be used
	int _retries; 			// Specifies the number of times to retries a cert renewal.
	int _retryDelayInSeconds; 	// Specifies the delay, in units of seconds, between retries
	int _keySize; 			// Specifies the key size
	char *_keyUsage; 			// Arrays of strings that specifies the key usage values
	char *_keyAlgorithm; 		// Specifies the key algorithm
	char *_keyAlgorithmStrength; 	// Specifies the key algorithm strength
	int _caInstance; 	
	int _challengeType; 	
	char *_challengeUsername;
	char *_challengePassword;
	char *_caCertFingerprint;
	char *_certSubject;
	char *_getCaCertUrl;
	int _getPkcsRequestUrl; 	
	int _getCertInitialUrl; 	
	TsStatus_t (*_messageCallback)(TsMessageRef_t, char *);	// pointer to function for sending the cert message
} TsScepConfig_t;

TsStatus_t ts_handle_certack( TsMessageRef_t fields );

/**
 * Handle a cert query message.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_cert_handle(TsMessageRef_t);

/**
 * Handle a cert renew message.
 * @param message
 * [in] The renew message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_certrenew_handle(TsMessageRef_t);

/**
 * Handle a cert rewoke message.
 * @param message
 * [in] The rewoke message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_certrewoke_handle(TsMessageRef_t);

/**
 * Create an update message containing cert info.
 * @param new
 * [out] Pointer to a TsMessageRef_t that will point to the new message.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_cert_make_update(TsMessageRef_t *);

/**
 * Handle a scep configuration message.
 * @param scepconfig
 * [in] TsLogConfigRef_t representing the log config to be modified/queried.
 * @param message
 * [in] The configuration message to be handled.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_scepconfig_handle(TsScepConfigRef_t, TsMessageRef_t);

/**
 * Create a scep configuration object.
 * @param scepconfig
 * [on/out] Pointer to a TsScepConfigRef_t in which the new config will be stored.
 * @param messageCallback
 * [in] Pointer to a function that will send a TsMessage with a specified kind.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_scepconfig_create(TsScepConfigRef_t *, TsStatus_t (*messageCallback)(TsMessageRef_t, char *));

/**
 * Destroy a scep configuration object.
 * @param scepconfig
 * [in] TsScepConfigRef_t representing the scep config to be destroyed.
 * @return
 * The return status (TsStatus_t) of the function, see ts_status.h for more information.
 * - TsStatusOk
 * - TsStatusError[Code]
 */
TsStatus_t ts_scepconfig_destroy(TsScepConfigRef_t);

#endif /* TS_CERT_H_ */
