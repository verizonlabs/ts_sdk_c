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

#define UUID_SIZE 37

#endif /* SDK_INCLUDE_TS_UTIL_H_ */
