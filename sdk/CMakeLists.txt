# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace SDK

# (optional) toolchain
if( NOT CMAKE_TOOLCHAIN_FILE )
    include( ${CMAKE_SOURCE_DIR}/toolchain/Toolchain.cmake )
endif()

include_directories( include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )

file( GLOB SOURCES source/*.c )

add_library( ts_sdk STATIC ${SOURCES} )
set_target_properties( ts_sdk PROPERTIES VERSION ${LIB_VERSION_STRING} )
target_link_libraries( ts_sdk tinycbor cjson )

