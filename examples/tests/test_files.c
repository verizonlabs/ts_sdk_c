// Copyright (c) 2018 Verizon Inc. All rights reserved
#include "ts_status.h"
#include "ts_file.h"

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
	sprintf(outbuf, "dir delete retruns error %d\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

	// Create  a directory - TEST
	iret = ts_file_directory_create(TDIR_NAME);
	sprintf(outbuf, "dir create returns error %d\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

#ifdef DELETE_TDIR
	// Delete the directory just created
	iret = ts_file_directory_delete(TDIR_NAME);
	sprintf(outbuf, "dir delete retruns error %d\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
#endif
	// Get the current default directory
	iret = ts_file_directory_default_set(TDIR_NAME);
	sprintf(outbuf, "dir default SET returns  error %d\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, (uint8_t const *)"\r\n", TX_WAIT_FOREVER);


#if 0
	// Get the current default directory
	strcpy(name,"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
	iret = ts_file_directory_default_get(&namePtr);
	sprintf(outbuf, "dir default get returns  error %d \n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, (uint8_t const *)"\r\n", TX_WAIT_FOREVER);
#endif
	// Delete the test file in case its there
	iret = ts_file_delete(TFILE_NAME);
	sprintf(outbuf, "DELETE test file before write  error %d - Current default dir is now...\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

	// Create a test file
	iret =  ts_file_create(TFILE_NAME);
	sprintf(outbuf, "Create file  returns  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);



	// Open a file for writing
	iret =  ts_file_open(&handle, TFILE_NAME, TS_FILE_OPEN_FOR_WRITE);
	sprintf(outbuf, "Open file  returns  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);


	// Write some lines
	iret = ts_file_write(&handle,"12345678\n\r", 10);
	sprintf(outbuf, "WRITE 1 file  returns  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);



	iret = ts_file_write(&handle,"abcdefgh\n\r", 10);
	sprintf(outbuf, "WRITE 2 file  returns  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);


	iret = ts_file_close(&handle);
	sprintf(outbuf, "First CLOSE  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);


	iret = ts_file_close(&handle);
	sprintf(outbuf, "SECOND CLOSE  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);


	// Open the file for reading
	iret =  ts_file_open(&handle, TFILE_NAME, TS_FILE_OPEN_FOR_READ);
	sprintf(outbuf, "Open file  for READ error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

	// Read a couple of line from it
	actualRead=0;
	iret = ts_file_read(&handle,readbuf, 100, &actualRead);
	sprintf(outbuf, "READ 1 file  returns  error %d  LENGTH read %d..\n\r", iret, actualRead);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
	readbuf[actualRead+1]=0; // end of string in case binara
	sprintf(outbuf, "READ data >>%s<<\n\r", readbuf);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);



	// Seek pack to 0
	iret = ts_file_seek(&handle,0);
	sprintf(outbuf, "SEEK error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

	// Read the first line again
	iret = ts_file_read(&handle,readbuf, 10, &actualRead);

	sprintf(outbuf, "READ after seek file  returns  error %d  LENGTH read %d..\n\r", iret, actualRead);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
	readbuf[actualRead+1]=0; // end of string in case binara

	sprintf(outbuf, "READ data after seek >>%s<<\n\r", readbuf);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);

	// Close the file

	iret = ts_file_close(&handle);
	sprintf(outbuf, "READ Close  error %d..\n\r", iret);
	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, outbuf, TX_WAIT_FOREVER);
	// Delete the test file

	// Delete the test file again - ERROR

	g_sf_console0.p_api->write(g_sf_console0.p_ctrl, "Done with tests\n\r", TX_WAIT_FOREVER);






	ts_platform_assert(0);
	/* Program should not reach beyond the assert(0). */





	return 0;
}
