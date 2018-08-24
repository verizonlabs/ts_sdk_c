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

	// Close the file

	iret = ts_file_close(&handle);
	printf("READ Close  error %d..\n\r", iret);
	// Delete the test file

	// Delete the test file again - ERROR







	ts_file_assert(0);
	/* Program should not reach beyond the assert(0). */





	return 0;
}
