/**
 * @file
 * ts_util.h
 *
 * @copyright
 * Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.
 *
 * @brief
 * Utility functions used in TS-SDK code.
 *
 * @details
 * Utility functions for doing routine operations in TS-SDK.
 */

#ifndef SDK_INCLUDE_TS_UTIL_H_
#define SDK_INCLUDE_TS_UTIL_H_

/**
 * Make a UUID.
 * @param out
 * [out] Pointer to a valid character array for the output. Must have room for 36 characters + 1 null termination.
 */
void ts_uuid( char *out );

/**
 * Allocate a buffer, and keep shrinking the size until we succeed or hit a minimum.
 * @param size
 * [in/out] Pointer to the initial size for the buffer. The actual size allocated, if any, will be returned here.
 * @param minimum
 * [in] The minimum buffer size to try.
 * @return
 * A pointer to the allocated buffer, or null if all fails.
 */
uint8_t* ts_get_buffer(size_t* size, size_t minimum);

#define UUID_SIZE 37

#endif /* SDK_INCLUDE_TS_UTIL_H_ */
