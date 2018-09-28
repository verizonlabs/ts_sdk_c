// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>

#include "ts_status.h"
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_file.h"
#include "ts_cert.h"

#ifdef DO_IT_THE_OLD_WAY
#include "include/cacert.h"
#include "include/client-crt.h"
#include "include/client-key.h"
#endif

#if defined(TS_TRANSPORT_MQTT)

#define MFG_CERT_PATH "/usr/share/thingspace/conf"
#define OP_CERT_PATH "/var/lib/thingspace/certs/"
#define CA_CERT_FILE "cacert.der"
#define CLIENT_CERT_FILE "clcert.der"
#define CLIENT_PRIVATE_KEY "clkey.der"
#define CLIENT_OPCERT_FILE "opcert.der"
#define CLIENT_OPPRIVATE_KEY "opkey.der"
#define ODS_FIREWALL	//TO-DO Add to CMAKElistst.txt

// application sensor cache
static TsMessageRef_t sensors;

// forward references
//static TsStatus_t initialize( TsServiceRef_t* );
static TsStatus_t handler( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port );
bool cert = false;

// Buffers for crypto object allocated dynamically from corresponding files
static uint8_t* cacert_buf;
static uint8_t* client_cert;
static uint8_t* client_key;

static uint32_t size_cacert_buf;
static uint32_t size_client_cert;
static uint32_t size_client_key;

bool g_useOpCert = false;
bool g_reboot_now = false;
bool g_scep_complete = false;

#ifndef OMIT_SCEP
static TsStatus_t ts_scep_function(){
#include "ts_scep.h"
	TsScepConfig_t Config;
	TsScepConfigRef_t *pConfig= &Config;
	/*enrol renew and rekey calling example */
	ts_scepconfig_restore(pConfig, "/var/lib/thingspace/","scepconfig");
	ts_scepconfig_save(pConfig, "/var/lib/thingspace/","scepconfig");
    ts_platform_assert(0);
	//ts_scep_enroll(pConfig, scep_ca);
	//ts_scep_enroll(pConfig, scep_renew);
	g_scep_complete = true;
	ts_status_debug("Exiting Thread now\r\n");
	// security initialization
}
#endif

static TsStatus_t loadFileIntoRam(char* directory, char* file_name, uint8_t** buffer, uint32_t* loaded_size);
int main( int argc, char *argv[] ) {
	TsStatus_t status;

#ifdef ODS_FIREWALL	
	/*Remove and insert kernel module*/
	system("/home/pi/uninstall-nanowallk.sh");

	system("/home/pi/install-nanowallk.sh");
#endif
	// initialize platform (see ts_platform.h)
	ts_platform_initialize();

	ts_security_initialize();

	ts_file_initialize();
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
	bool connected;
	bool running;
	char *cert_path = NULL;
	char *cert_name = NULL;
	char *key_name = NULL;
	ts_service_create( &service );

	
	// TEST TEST TEST
	ts_scep_function();
	
	ts_status_debug( "simple: initializing certificates,...\n");
	g_useOpCert = ts_check_opcert_available();
	do {
		
		if(g_useOpCert){
			ts_status_debug("Connecting with Operational Certificates\r\n");
			cert_path = OP_CERT_PATH;
			cert_name = CLIENT_OPCERT_FILE;
			key_name = CLIENT_OPPRIVATE_KEY;
		}
		else {
			ts_status_debug("Connecting with Manufacturer Certificates\r\n");
			cert_path = MFG_CERT_PATH;
			cert_name = CLIENT_CERT_FILE;
			key_name = CLIENT_PRIVATE_KEY;
		}
		// Read certs and keys into memory - Fatal is can't read them
		status = loadFileIntoRam(cert_path, CA_CERT_FILE, &cacert_buf, &size_cacert_buf);
		if( status != TsStatusOk ) {
			ts_status_debug("simple: failed to read CA Cert file %s\n", ts_status_string(status));
			ts_platform_assert(0);
		}
		status = loadFileIntoRam(cert_path, cert_name, &client_cert, &size_client_cert);
		if( status != TsStatusOk ) {
			ts_status_debug("simple: failed to read Client Cert File, %s\n", ts_status_string(status) );
			ts_platform_assert(0);
		}
		status = loadFileIntoRam(cert_path , key_name, &client_key, &size_client_key);
		if( status != TsStatusOk ) {
			ts_status_debug("simple: failed to Client Private Key %s\n", ts_status_string(status));
			ts_platform_assert(0);
		}


		ts_service_set_server_cert_hostname( service, (const char *)host );
		ts_service_set_server_cert( service, cacert_buf, size_cacert_buf );
		ts_service_set_client_cert( service, client_cert, size_client_cert );
		ts_service_set_client_key( service, client_key, size_client_key );

	
	
	// connect to thingspace server
	// enter running loop,...
	
		ts_status_debug( "simple: initializing connection,...\n");
		status = ts_service_dial( service, hostname_and_port );
		if( status != TsStatusOk ) {
			ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
			continue;
		}
		connected = true;

		//  subscribe to field gets and sets
		ts_status_debug( "simple: initializing callback,...\n");
		status = ts_service_dequeue( service, TsServiceActionMaskAll, handler );
		if( status != TsStatusOk ) {
			ts_status_debug("simple: failed to dial, %s\n", ts_status_string(status));
			continue;
		}
		running = true;
		// enter connected loop,...
		ts_status_debug( "simple: entering run-loop,...\n");
		uint64_t timestamp = ts_platform_time();
		uint32_t interval = 1 * TS_TIME_SEC_TO_USEC;
		do {

			// perform update at particular delta
			if( ts_platform_time() - timestamp > interval ) {

				timestamp = ts_platform_time();
				status = ts_service_enqueue( service, sensors );
				if( status != TsStatusOk ) {
					ts_status_debug( "simple: ignoring failure to enqueue sensor data, %s\n", ts_status_string(status) );
					// do nothing
				}
				
				
				if (cert == true){
					cert = false;
					/*Creating SCEP Thread here for testing*/
					pthread_t tid;
					int err;
					err = pthread_create(&tid, NULL, &ts_scep_function, NULL);
					if(0 != err){
						ts_status_alarm("Cannot create Thread!!\r\n");
					}
					else{
						ts_status_debug("Thread Created Successfully\r\n");
					}
				}
				if(g_scep_complete)
				{
					g_scep_complete = false;
					// Send an cert event message representing operation cert information	
					TsMessageRef_t certMessage;
					if (ts_cert_make_update( &certMessage ) == TsStatusOk) {
						ts_message_dump(certMessage);
						ts_service_enqueue_typed(service, "ts.event.cert", certMessage);
						ts_message_destroy(certMessage);
					}
					ts_status_debug("DONE::Thread Indicated Exit!\r\n");
				}

			}

			// provide client w/some processing power
			// note - this will run continuously until the interval is complete
			//        other options include limiting the interval, and sleeping after
			status = ts_service_tick( service, interval );
			if( status != TsStatusOk ) {
				ts_status_debug( "simple: failed to perform tick, %s, shutting down,...\n", ts_status_string(status) );
				connected = false;
			}
			
			/*Check if Re-boot of Application is needed*/
			if(g_reboot_now)
			{
				ts_status_debug("Re-starting the Application now\r\n");
				g_reboot_now = false;
				break;
			}
			
			
			

		} while( connected );
		ts_status_debug( "simple: exited run-loop, cleaning up rebooting...\n");

		// disconnect from thingspace server
		ts_service_hangup( service );

	} while( running );
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


