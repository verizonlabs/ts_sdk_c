# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Platforms

set( TARGETS ts_sdk_platforms )

file( GLOB SOURCES
        "*.c"
        "unix/*.c" )

add_library( ts_sdk_platforms OBJECT ${SOURCES} )

target_include_directories( ts_sdk_platforms PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )
