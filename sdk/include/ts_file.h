/*
 * ts_file.h
 *
 *  Created on: Jun 28, 2018
 *      Author: Admin
 */

#ifndef TS_FILE_H
#define TS_FILE_H
/**
 * @file
 * ts_file.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * The OS-agnostic abstraction of for file access to an embedded file system
 * @details
 * The SDK uses this interface to save items permanently to the device
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

// #defines here
#define MAX_FILE_HANDLE 100

// File modes
#define TS_FILE_OPEN_FOR_READ                       0
#define TS_FILE_OPEN_FOR_WRITE                      1
/**
 * The file object reference
 */
typedef struct TsFile *TsFileRef_t;
/**
 * File Handle
 */
typedef struct
{
	uint32_t data[MAX_FILE_HANDLE];  // how know this is big enough
} ts_file_handle;

/**
 * The file object
 */
typedef struct TsFile {


} TsFile_t;

/**
 * The file vector table (i.e., the file "class" definition), used to define the file SDK-aspect.
 * See examples/platforms for available platform implementations, or use your own customized implementation.
 */
typedef struct TsFileVtable {

	/**
	 * Initialize the storage device (flash) and the file system
	 */
	void		(*initialize)();


	/**
	 * Create a directory on the file system
	 */
	TsStatus_t 		(*directory_create) (char* directory_name);

	/**
	 * Get the current default directory in the file system.
	 */
	TsStatus_t 		(*directory_default_get)(char** returned_path);

	/**
	 * Set the current default directory in the file system
	 */
	TsStatus_t	(*directory_default_set)(char* new_directory);

	/**
	 * Delete a directory in the file system
	 */
	TsStatus_t		(*directory_delete)(char* directory);

	/**
	 * Close a file.
	 */
	TsStatus_t		(*close)(ts_file_handle*);

	/**
	 * Create a file on the file system
	 */
     TsStatus_t		(*create)(char* file_name);

	/**
	 * Delete a file from the file system
	 */
     TsStatus_t		(*delete)(char* file_name);

 	/**

	 * Open a file on the file system
	 */
     TsStatus_t		(*open)(ts_file_handle *msg, const char *file, int type);

	/**
	 * Read a file from the file system
	 */
     TsStatus_t		(*read)(ts_file_handle* handle_ptr, void* buffer, uint32_t size,  uint32_t* act_size);

	/**
	 * Seek to a position in a file
	 */
     TsStatus_t		(*seek)(ts_file_handle* handle_ptr,  uint32_t offset);

	/**
	 * Write to a file in the file system.
	 */
     TsStatus_t		(*write)(ts_file_handle *handle_ptr, void* buffer, uint32_t size);

 	/**
 	 * Read a file from the file system up to and INCLUDING the end of the line
 	 */
      TsStatus_t		(*readline)(ts_file_handle* handle_ptr, void* buffer, uint32_t size);

 	/**
 	 * Returns the size of a file
 	 */
      TsStatus_t		(*size)(ts_file_handle* handle_ptr,  uint32_t* size);

 	/**
 	 * Write to a line file in the file system. Use supplies the '\n' in the string
 	 */
      TsStatus_t		(*writeline)(ts_file_handle *handle_ptr, char* buffer);
	/**
	 * Handle any assertion, i.e., this function doesnt perform the check, it simply performs the effect, e.g.,
	 * display the given message and halt, etc.
	 */
     void		(*assertion)(const char *msg, const char *file, int line);


} TsFileVtable_t;


extern const TsFileVtable_t *ts_file;

#define noerror 			xxx->directory_delete

#define ts_file_directory_delete 			ts_file->directory_delete
#define ts_file_directory_create	        ts_file->directory_create
#define ts_file_directory_default_set 		ts_file->directory_default_set
#define ts_file_directory_default_get 		ts_file->directory_default_get
#define ts_file_initialize  				ts_file->initialize
#define ts_file_close 						ts_file->close
#define ts_file_delete 						ts_file->delete
#define ts_file_open  						ts_file->open
#define ts_file_read 						ts_file->read
#define ts_file_seek  						ts_file->seek
#define ts_file_write 						ts_file->write
#define ts_file_create 						ts_file->create
#define ts_file_readline 					ts_file->readline
#define ts_file_size						ts_file->size
#define ts_file_writeline					ts_file->writeline


#ifdef NDEBUG
#define ts_file_assert(EX) (void)0
#else
#define ts_file_assert(EX)  (void)((EX) || (ts_file->assertion(#EX,__FILE__,__LINE__),0))
#endif

#endif // TS_FILE_H


