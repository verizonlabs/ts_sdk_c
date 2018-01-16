# Copyright (C) 2017, Verizon, Inc. All rights reserved.
cmake_minimum_required( VERSION 3.7 )

# force cmake build subdirectory
include( ${CMAKE_SOURCE_DIR}/PreventInSourceBuilds.cmake )

# force cmake build type
if( NOT CMAKE_BUILD_TYPE )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, Debug or Release." FORCE )
endif()
message( STATUS "## Build Type            (Debug|Release) : ${CMAKE_BUILD_TYPE}" )

# (optional) toolchain
if( NOT CMAKE_TOOLCHAIN_FILE )
    include( ${CMAKE_SOURCE_DIR}/toolchain/Toolchain.cmake )
endif()

# project definition
project( ts_sdk_c C CXX ASM )

# component and platform selection
add_definitions( -DTS_SERVICE_TS_JSON )     # TS_JSON, TS_CBOR, NONE or CUSTOM
add_definitions( -DTS_TRANSPORT_MQTT )      # MQTT or CUSTOM only
add_definitions( -DTS_SECURITY_MBED )       # MBED, MOCANA, NONE or CUSTOM
add_definitions( -DTS_CONTROLLER_SOCKET )   # MONARCH, SOCKET or CUSTOM
add_definitions( -DTS_PLATFORM_UNIX )       # UNIX, THREADX, NONE or CUSTOM

# package version
set( CPACK_PACKAGE_VERSION_MAJOR "2" )
set( CPACK_PACKAGE_VERSION_MINOR "0" )
set( CPACK_PACKAGE_VERSION_PATCH "0" )
set( LIB_VERSION_STRING ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH} )
message( STATUS "## Build Version                         : ${LIB_VERSION_STRING}" )

# libraries
add_subdirectory( sdk )
add_subdirectory( sdk_dependencies )
add_subdirectory( sdk_components )
add_subdirectory( sdk_platforms )

# build example applications (optional)
set( BUILD_APPLICATIONS ON CACHE BOOL "Build optional executables and/or reference binaries" FORCE)
message( STATUS "## Build Executables            (ON|OFF) : ${BUILD_EXECUTABLES}" )
if( BUILD_APPLICATIONS )
    add_subdirectory( examples/applications )
endif()

# build example tests (optional)
set( BUILD_TESTS ON CACHE BOOL "Build optional tests" FORCE)
message( STATUS "## Build Tests                  (ON|OFF) : ${BUILD_TESTS}" )
if( BUILD_TESTS )
    add_subdirectory( examples/tests )
endif()
