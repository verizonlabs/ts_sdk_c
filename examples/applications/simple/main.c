// Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
#include <stdio.h>

#include "ts_status.h"
#include "ts_platform.h"
#include "ts_service.h"
#include "ts_file.h"

#ifdef DO_IT_THE_OLD_WAY
#include "include/cacert.h"
#include "include/client-crt.h"
#include "include/client-key.h"
#endif



#define TS_TRANSPORT_MQTT
#if defined(TS_TRANSPORT_MQTT)

// application sensor cache
static TsMessageRef_t sensors;

// forward references
//static TsStatus_t initialize( TsServiceRef_t* );
static TsStatus_t handler( TsServiceRef_t, TsServiceAction_t, TsMessageRef_t );
static TsStatus_t usage(int argc, char *argv[], char ** hostname_and_port, char ** host, char ** port );


static uint8_t* caCertBuffPtr;
static uint8_t* cleintCertBuffOPtr;
static uint8_t clientKeyBuffPtr;

static loadFileIntoRam(char* file_name, uint8_t buffer);

int main( int argc, char *argv[] ) {

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
	ts_service_create( &service );

	// security initialization

	ts_status_debug( "simple: initializing certificates,...\n");

	// Read certs and keys into memory

	ts_service_set_server_cert_hostname( service, (const char *)host );
	ts_service_set_server_cert( service, cacert_buf, sizeof( cacert_buf ) );
	ts_service_set_client_cert( service, client_cert, sizeof( client_cert ) );
	ts_service_set_client_key( service, client_key, sizeof( client_key ) );

	// connect to thingspace server
	ts_status_debug( "simple: initializing connection,...\n");
	TsStatus_t status = ts_service_dial( service, hostname_and_port );
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
#define MFG_CERT_PATH /usr/share/thingspace/conf
#error "fix rthese and set up the file"
#define CA_CERT_FILE "file1"
#define CLIENT_CERT_FILE "file2"
#define CLIENT_PRIVATE_KEY "file3"
static TsStatus_t loadFileIntoRam(char* directory, char* file_name, uint8_t** buffer)
{
  	TsStatus_t iret = TsStatusOk;
	ts_file_handle handle;
	uint32_t actual_size, size;
	uint8_t* addr;

	// Set the default directory, then open and size the file. Malloc some ram and read it all it.

	iret = ts_file_directory_default_set(directory);
	if (TsStatusOK != iret)
		goto error;

	iret =  ts_file_open(&handle, file_name, TS_FILE_OPEN_FOR_WRITE);
	if (TsStatusOK != iret)
		goto error;

	iret = ts_file_size(&handle, &size);
	if (TsStatusOK != iret)
		goto error;

	addr = ts_platform_malloc( actualRead);
	if (addr==0)
		goto error;

    *buffer = addr;
	iret = ts_file_read(&handle,addr, size, &actualRead);
	if (TsStatusOK != iret) {
		ts_platform_free(addr);
		goto error;
	}

	ts_file_close(&handle);


	error:
	return iret;

}

int freeCryptoMemory ()
{
#warning "file this in!!!!"
}


#if 0
int main(void)
{
	ts_file_initialize();


	TsStatus_t iret;
	uint32_t  rngbuf[1] =
	{
			0
	};
	static char outbuf[120];
	static char readbuf[120];
	static char name[120];
	char* namePtr = &name[0];
	const char* TDIR_NAME="subdir";
	const char* TFILE_NAME="vzw.dat";
	int actualRead;

	ts_file_handle handle;


	// Delete a directory that doesn't exist - ERROR
	iret = ts_file_directory_delete(TDIR_NAME);
	printf("dir delete retruns error %d\n\r", iret);

	// Create  a directory - TEST
	iret = ts_file_directory_create(TDIR_NAME);
	printf("dir create returns error %d\n\r", iret);

#ifdef DELETE_TDIR
	// Delete the directory just created
	iret = ts_file_directory_delete(TDIR_NAME);
	printf("dir delete retruns error %d\n\r", iret);
#endif
	// Get the current default directory
	iret = ts_file_directory_default_set(TDIR_NAME);
	printf("dir default SET returns  error %d\n\r", iret);



#if 0
	// Get the current default directory
	strcpy(name,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	iret = ts_file_directory_default_get(&namePtr);
	printf("dir default get returns  error %d \n\r", iret);

#endif
	// Delete the test file in case its there
	iret = ts_file_delete(TFILE_NAME);
	printf("DELETE test file before write  error %d - Current default dir is now...\n\r", iret);


	// Create a test file
	iret =  ts_file_create(TFILE_NAME);
	printf("Create file  returns  error %d..\n\r", iret);



	// Open a file for writing
	iret =  ts_file_open(&handle, TFILE_NAME, TS_FILE_OPEN_FOR_WRITE);
	printf("Open file  returns  error %d..\n\r", iret);


	// Write some lines
	iret = ts_file_write(&handle,"12345678\n\r", 10);
	printf("WRITE 1 file  returns  error %d..\n\r", iret);



	iret = ts_file_write(&handle,"abcdefgh\n\r", 10);
	printf("WRITE 2 file  returns  error %d..\n\r", iret);


	iret = ts_file_close(&handle);
	printf("First CLOSE  error %d..\n\r", iret);


	iret = ts_file_close(&handle);
	printf("SECOND CLOSE  error %d..\n\r", iret);


	// Open the file for reading
	iret =  ts_file_open(&handle, TFILE_NAME, TS_FILE_OPEN_FOR_READ);
	printf("Open file  for READ error %d..\n\r", iret);

	// Size the file
	actualRead=0;
	iret = ts_file_size(&handle, &actualRead);
	printf("SIZE 1 file  returns  error %d  LENGTH size %d..\n\r", iret, actualRead);
	// Read a couple of line from it
	actualRead=0;
	iret = ts_file_read(&handle,readbuf, 100, &actualRead);
	printf("READ 1 file  returns  error %d  LENGTH read %d..\n\r", iret, actualRead);
	readbuf[actualRead+1]=0; // end of string in case binara
	printf("READ data >>%s<<\n\r", readbuf);



	// Seek pack to 0
	iret = ts_file_seek(&handle,0);
	printf("SEEK error %d..\n\r", iret);

	// Read the first line again
	iret = ts_file_read(&handle,readbuf, 10, &actualRead);

	printf("READ after seek file  returns  error %d  LENGTH read %d..\n\r", iret, actualRead);
	readbuf[actualRead+1]=0; // end of string in case binara

	printf("READ data after seek >>%s<<\n\r", readbuf);


       // Read an existing file by line
	iret = ts_file_close(&handle);

      ts_file_directory_default_set("..");  // up from subdir
	iret =  ts_file_open(&handle, "line.txt", TS_FILE_OPEN_FOR_READ);

       iret = TsStatusOk;
       char text_line[3];
       while(iret==TsStatusOk) {

         iret = ts_file_readline(&handle,text_line, sizeof(text_line));
         printf("Line read status %d len %d>>>%s<\n",iret, strlen(text_line),text_line);
       sleep(1);

       }



        // Close the file

	iret = ts_file_close(&handle);


        // Writeline test
	iret =  ts_file_create("newline.txt");
	printf("Create file  returns  error %d..\n\r", iret);
	iret =  ts_file_open(&handle, "newline.txt", TS_FILE_OPEN_FOR_WRITE);
         iret = ts_file_writeline(&handle,"Line 1\n");
         iret = ts_file_writeline(&handle,"Line 222222\n");
         iret = ts_file_writeline(&handle,"Line 3\n");
	iret = ts_file_close(&handle);







	ts_file_assert(0);
	/* Program should not reach beyond the assert(0). */





	return 0;
}

#endif

