# Copyright (C) 2017, Verizon, Inc. All rights reserved.

# set initial value of SOURCES
set( SOURCES ../ts_platforms.c )

# example platforms
add_subdirectory( ts_sdk_c_platforms_${TS_PLATFORM} )
