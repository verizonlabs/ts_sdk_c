# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Platforms

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )

# TODO - should only add platform sources
file( GLOB SOURCES
        "*.c"
        "unix/*.c" )

add_library( ts_sdk_platforms STATIC ${SOURCES} )
