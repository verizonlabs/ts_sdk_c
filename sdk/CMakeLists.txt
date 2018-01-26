# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace SDK

set( TARGETS ts_sdk_apis )

file( GLOB SOURCES source/*.[ch] )

add_library( ts_sdk_apis OBJECT ${SOURCES} )

target_include_directories( ts_sdk_apis PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )


