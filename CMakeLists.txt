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

# build vendor libraries
add_subdirectory( sdk_dependencies )

# component selection
add_definitions( -DTS_SERVICE_TS_JSON )     # TS_JSON, TS_CBOR or CUSTOM
add_definitions( -DTS_TRANSPORT_MQTT )      # MQTT or CUSTOM only
add_definitions( -DTS_SECURITY_NONE )       # MBED, MOCANA, NONE or CUSTOM
add_definitions( -DTS_CONTROLLER_NONE )     # MONARCH, NONE or CUSTOM

# platform configuration
add_definitions( -DTS_PLATFORM_UNIX )       # UNIX, THREADX, NONE or CUSTOM

# platform OS and board configuration
# unix, unix/raspberry_pi, threadX/renesas_s5_d9, none/freedom, none/nucleo
set( TS_BOARD unix CACHE STRING "rtos and board" FORCE )

# build object libraries
add_subdirectory( sdk_components )
add_subdirectory( sdk_platforms )
add_subdirectory( sdk )

# build common static library
# (depends on components and dependencies)
add_library( ts_sdk STATIC $<TARGET_OBJECTS:ts_sdk_apis> $<TARGET_OBJECTS:ts_sdk_components> $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( ts_sdk tinycbor cjson mbedtls pahomqtt )

# build example executables
add_subdirectory( examples )
