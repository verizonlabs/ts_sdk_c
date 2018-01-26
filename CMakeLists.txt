# Copyright (C) 2017, Verizon, Inc. All rights reserved.
cmake_minimum_required( VERSION 3.7 )

# force cmake build subdirectory
include( ${CMAKE_SOURCE_DIR}/toolchain/PreventInSourceBuilds.cmake )

# force cmake build type
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, Debug or Release." FORCE )
endif()

# (optional) toolchain
if( NOT CMAKE_TOOLCHAIN_FILE )
    include( ${CMAKE_SOURCE_DIR}/toolchain/Toolchain.cmake )
endif()

# project definition
project( ts_sdk_c C CXX ASM )
message( STATUS "## Build Type            (Debug|Release) : ${CMAKE_BUILD_TYPE}" )

# component and platform selection
add_definitions( -DTS_SERVICE_TS_JSON )     # TS_JSON, TS_CBOR, NONE or CUSTOM
add_definitions( -DTS_TRANSPORT_MQTT )      # MQTT or CUSTOM only
add_definitions( -DTS_SECURITY_MBED )       # MBED, MOCANA, NONE or CUSTOM
add_definitions( -DTS_CONTROLLER_NONE )     # MONARCH, NONE or CUSTOM

# libraries
add_subdirectory( sdk_dependencies )
add_subdirectory( sdk_components )
add_subdirectory( sdk )

# build common library
# (depends on components and dependencies)
add_library( ts_sdk STATIC $<TARGET_OBJECTS:ts_sdk_apis> $<TARGET_OBJECTS:ts_sdk_components> )
target_link_libraries( ts_sdk tinycbor cjson )

# example executables
add_subdirectory( examples )
