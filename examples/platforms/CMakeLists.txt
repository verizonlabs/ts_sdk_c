# Copyright (C) 2017, 2018 Verizon, Inc. All rights reserved.

set( SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ts_platforms.c )
add_subdirectory( ts_sdk_c_platforms_${TS_PLATFORM} )
