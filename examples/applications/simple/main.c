// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdio.h>

#include "ts_status.h"
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_file.h"
#include "ts_cert.h"

#ifndef OMIT_SCEP
// SCEP
#include "ts_scep.h"
#endif

#ifdef DO_IT_THE_OLD_WAY
#include "include/cacert.h"
#include "include/client-crt.h"
#include "include/client-key.h"
#endif



#if defined(TS_TRANSPORT_MQTT)

// application sensor cache
static TsMessageRef_t sensors;

// forward references
//static TsStatus_t initialize( TsServiceRef_t* );
static TsStatus_t handler( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port );

// Buffers for crypto object allocated dynamically from corresponding files
static uint8_t* cacert_buf;
static uint8_t* client_cert;
static uint8_t* client_key;

static uint32_t size_cacert_buf;
static uint32_t size_client_cert;
static uint32_t size_client_key;

#define MFG_CERT_PATH "/usr/share/thingspace/conf"
#define CA_CERT_FILE "cacert.der"
#define CLIENT_CERT_FILE "clcert.der"
#define CLIENT_PRIVATE_KEY "clkey.der"

static TsStatus_t loadFileIntoRam(char* directory, char* file_name, uint8_t** buffer, uint32_t* loaded_size);

int main( int argc, char *argv[] ) {
	TsStatus_t status;

	// initialize platform (see ts_platform.h)
	ts_platform_initialize();

	ts_security_initialize();

	ts_file_initialize();
#ifndef OMIT_SCEP
        {
          struct TsScepConfig config;
          TsScepConfigRef_t pConfig = &config;

        // SCEP
        ts_scep_initialize();
        // Set up a config - this is what is read from file for scep 
        config._enabled = false;
       
	config._generateNewPrivateKey = false;
        config._certExpiresAfter=12345;
        config._

	config._certEnrollmentType = 1;	

	config._numDaysBeforeAutoRenew=999;

	config._encryptionAlgorithm = "rsa";

	config._hashFunction = "sha-1";

	config._retries=456;

	config._retryDelayInSeconds=360;

	config._keySize=2048; 
        char keys[10][]= { "abd", "def", "ghi"};
	config._keyUsage=keys;

	config._keyAlgorithm="rsa";

	config._keyAlgorithmStrength="2048";

	config._caInstance=789; 	

	config._challengeType=1111; 	

	config._challengeUsername="frank";

	config.*_challengePassword="verizon1";

	config.*_caCertFingerprint="thumb";

	config.*_certSubject="who";

	config._getCaCertUrl="www.verizon.com";

	config._getPkcsRequestUrl = 98765; 

	config._getCertInitialUrl = 1; 	

        ts_scep_enroll(pConfig, scep_enroll);
        ts_scep_assert(0);
        }
#endif
	// initialize status reporting level (see ts_status.h)
	ts_status_set_level( TsStatusLevelDebug );
	ts_status_debug( "simple: initializing,...\n");

	// initialize hostname
	char * hostname_and_port = "63.98.10.34:8883";
	char * host = "63.98.10.34";
	char * port = "8883";
#if defined(TS_PLATFORM_UNIX)
	if( usage( argc, argv, &hostname_and_port, &host, &port ) != TsStatusOk ) {
		ts_status_debug( "simple: failed to parse host and port\n" );
		ts_platform_assert(0);
	}
#endif
	ts_status_debug( "simple: hostname(%s), host(%s), port(%s)\n", hostname_and_port, host, port );

	// initialize sensor cache (usually set from hardware)
	ts_message_create( &sensors );
	ts_message_set_float( sensors, "temperature", 50.2 );

	// initialize client (see ts_service.h)
	TsServiceRef_t service;
	ts_service_create( &service );

	// security initialization

	ts_status_debug( "simple: initializing certificates,...\n");

	// Read certs and keys into memory - Fatal is can't read them
	status = loadFileIntoRam(MFG_CERT_PATH, CA_CERT_FILE, &cacert_buf, &size_cacert_buf);
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to read CA Cert file %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}
	status = loadFileIntoRam(MFG_CERT_PATH, CLIENT_CERT_FILE, &client_cert, &size_client_cert);
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to read Client Cert File, %s\n", ts_status_string(status) );
		ts_platform_assert(0);
	}
	status = loadFileIntoRam(MFG_CERT_PATH , CLIENT_PRIVATE_KEY, &client_key, &size_client_key);
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to Client Private Key %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}



	ts_service_set_server_cert_hostname( service, (const char *)host );
	ts_service_set_server_cert( service, cacert_buf, size_cacert_buf );
	ts_service_set_client_cert( service, client_cert, size_client_cert );
	ts_service_set_client_key( service, client_key, size_client_key );

	// connect to thingspace server
	ts_status_debug( "simple: initializing connection,...\n");
	status = ts_service_dial( service, hostname_and_port );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}

	//  subscribe to field gets and sets
	ts_status_debug( "simple: initializing callback,...\n");
	status = ts_service_dequeue( service, TsServiceActionMaskAll, handler );
	if( status != TsStatusOk ) {
		ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
		ts_platform_assert(0);
	}

	// enter run loop,...
	ts_status_debug( "simple: entering run-loop,...\n");
	uint64_t timestamp = ts_platform_time();
	uint32_t interval = 1 * TS_TIME_SEC_TO_USEC;
	bool running = true;
	do {

		// perform update at particular delta
		if( ts_platform_time() - timestamp > interval ) {

			timestamp = ts_platform_time();
			status = ts_service_enqueue( service, sensors );
			if( status != TsStatusOk ) {
				ts_status_debug( "simple: ignoring failure to enqueue sensor data, %s\n", ts_status_string(status) );
				// do nothing
			}
		}

		// provide client w/some processing power
		// note - this will run continuously until the interval is complete
		//        other options include limiting the interval, and sleeping after
		status = ts_service_tick( service, interval );
		if( status != TsStatusOk ) {
			ts_status_debug( "simple: failed to perform tick, %s, shutting down,...\n", ts_status_string(status) );
			running = false;
		}

	} while( running );
	ts_status_debug( "simple: exited run-loop, cleaning up and exiting...\n");

	// disconnect from thingspace server
	ts_service_hangup( service );

	// clean-up and exit
	ts_service_destroy( service );
	ts_message_destroy( sensors );

	ts_platform_assert(0);
}

static TsStatus_t handler( TsServiceRef_t service, TsServiceAction_t action, TsMessageRef_t message ) {

	switch( action ) {
	case TsServiceActionSet: {

		float temperature;
		if( ts_message_get_float( message, "temperature", &temperature ) == TsStatusOk ) {
			ts_status_debug( "handler(set): temperature, %f\n", temperature );
			ts_message_set_float( sensors, "temperature", temperature );
		}
		break;
	}

	case TsServiceActionGet: {

		TsMessageRef_t object;
		if( ts_message_has( message, "temperature", &object ) == TsStatusOk ) {
			float temperature;
			if( ts_message_get_float( sensors, "temperature", &temperature ) == TsStatusOk ) {
				ts_status_debug( "handler(get): temperature, %f\n", temperature );
				ts_message_set_float( message, "temperature", temperature );
			}
		}
		break;
	}

	case TsServiceActionActivate: {

		float temperature;
		if( ts_message_get_float( sensors, "temperature", &temperature ) == TsStatusOk ) {
			ts_status_debug( "handler(activate): temperature, %f\n", temperature );
			ts_message_set_float( message, "temperature", temperature );
		}
		break;
	}

	case TsServiceActionDeactivate:
	case TsServiceActionSuspend:
	case TsServiceActionResume:

		// not supported by TS-JSON
		// fallthrough

	default:

		// do nothing
		break;
	}

	return TsStatusOk;
}

static char * xhostname_and_port = "simpm.thingspace.verizon.com:8883";
static char xhost[256], xport[6];
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port ) {

	bool has_given_cert_hostname = false;
	switch( argc ) {
#if defined(TS_SERVICE_TS_CBOR)
	default:
		ts_status_alarm("usage: example_simple [hostname_and_port]\n");
		return TsStatusError;
#else
	case 1:
		// allow default production if ts-json being tested (not ts-cbor)
		*hostname_and_port = xhostname_and_port;
		break;
	default:
		// fallthrough
#endif
	case 2:
		*hostname_and_port = argv[ 1 ];
		break;

	case 3:
		*hostname_and_port = argv[ 1 ];
		*host = argv[ 2 ];
		has_given_cert_hostname = true;
		break;
	}

	if( ts_address_parse( *hostname_and_port, xhost, xport ) != TsStatusOk ) {
		ts_status_alarm("failed to parse given address, %s\n", hostname_and_port);
		return TsStatusError;
	}
	if( !has_given_cert_hostname ) {
		*host = xhost;
	}
	*port = xport;

	return TsStatusOk;
}
#else

int main() {

	ts_status_alarm("missing one or many components, please check compile directives and build again\n");

}

#endif


// Loads a crypto object into memory from a files. Sizes the file and malloc needed memory
// Certificate storage and keys - base credentials

static TsStatus_t loadFileIntoRam(char* directory, char* file_name, uint8_t** buffer, uint32_t* loaded_size)
{
  	TsStatus_t iret = TsStatusOk;
	ts_file_handle handle;
	uint32_t actual_size, size;
	uint8_t* addr;

	// Set the default directory, then open and size the file. Malloc some ram and read it all it.

	iret = ts_file_directory_default_set(directory);
	if (TsStatusOk != iret)
		goto error;

	iret =  ts_file_open(&handle, file_name, TS_FILE_OPEN_FOR_READ);
	if (TsStatusOk != iret)
		goto error;

	iret = ts_file_size(&handle, &size);
	if (TsStatusOk != iret)
		goto error;

	addr = ts_platform_malloc( size);
	if (addr==0)
		goto error;

    *buffer = addr;
	iret = ts_file_read(&handle,addr, size, &actual_size);
	// Make sure we got the whole thing
	if (TsStatusOk != iret || size!=actual_size) {
		ts_platform_free(addr, size);
		goto error;
	}
	// The actual size of the object.  Users generall need to know how big it is
    *loaded_size = size;
	ts_file_close(&handle);


	error:
	return iret;

}

int freeCryptoMemory ()
{
	return 0;
}


