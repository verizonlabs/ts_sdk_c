# Copyright (C) 2017, Verizon, Inc. All rights reserved.
cmake_minimum_required( VERSION 3.6 )

# determine if this will build examples or not (affects submodules)
option( BUILD_EXAMPLES "Compile example platforms and applications" OFF )
message( STATUS "## Build Examples               (ON|OFF) : ${BUILD_EXAMPLES}" )

# force cmake build in subdirectory
include( ${CMAKE_SOURCE_DIR}/tools/PreventInSourceBuilds.cmake )

# force cmake submodule creation
include( ${CMAKE_SOURCE_DIR}/tools/SynchronizeSubmodules.cmake )

# force cmake build type
if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, Debug or Release." FORCE )
endif()

# project definition
project( ts_sdk_c C CXX ASM )
set( CMAKE_C_STANDARD 99 )

# (optional) toolchain
if( CMAKE_TOOLCHAIN_FILE )
	message( STATUS "## Toolchain file chosen		          : ${CMAKE_TOOLCHAIN_FILE}" )
	message( STATUS "## Target triplet			              : ${TARGET_TRIPLET}" )
	message( STATUS "## Floating point hardware		          : ${FP_HARD}" )
	message( STATUS "## CPU chosen                            : ${CPU_TYPE}" )
endif()

message( STATUS "## Build Type            (Debug|Release) : ${CMAKE_BUILD_TYPE}" )

# component selection
add_definitions( -DTS_SERVICE_TS_JSON )     # TS_JSON, TS_CBOR or CUSTOM
add_definitions( -DTS_TRANSPORT_MQTT )      # MQTT or CUSTOM
add_definitions( -DTS_SECURITY_CUSTOM )       # MBED, MOCANA, NONE or CUSTOM
add_definitions( -DTS_CONTROLLER_CUSTOM )     # MONARCH, NONE or CUSTOM
add_definitions( -DTS_FIREWALL_CUSTOM )       # NONE or CUSTOM (rarely used)
add_definitions( -DTS_MUTEX_NONE )          # NONE or CUSTOM (rarely used)

# build vendor libraries
add_definitions( -D__GLIBC__ )              # TODO, remove tinyCBOR HACK
add_subdirectory( sdk_dependencies )

# build object libraries
add_subdirectory( sdk_components )
add_subdirectory( sdk )

# build common static library
# (depends on components and dependencies)
add_library( ts_sdk STATIC
    $<TARGET_OBJECTS:ts_sdk_apis>
    $<TARGET_OBJECTS:ts_sdk_components>
    $<TARGET_OBJECTS:tinycbor>
    $<TARGET_OBJECTS:cjson>
    $<TARGET_OBJECTS:pahomqtt> )

# link only one version from the security dependencies
get_directory_property( DEFINITIONS COMPILE_DEFINITIONS )
if( DEFINITIONS MATCHES TS_SECURITY_MBED )
    target_link_libraries( ts_sdk mbedtls )

elseif( DEFINITIONS MATCHES TS_SECURITY_MOCANA )
    target_link_libraries( ts_sdk mocana )

endif()

# build optional examples
if( BUILD_EXAMPLES )

    # platform kernal and board configuration
    #
    #   unix_raspberry-pi3      Raspberry Pi3 Linux and Unix variants
    #   threadx_synergy-s5d9    ThreadX Renesas Synergy S5D9
    #   none_freedom-k82f       Bare-metal NXP Freedom K82F
    #   none_nucleo-l476        Bare-metal STMicro Nucleo L476
    set( TS_PLATFORM unix_raspberry-pi3 CACHE STRING "(RT)OS and Hardware Platform" FORCE )

    # platform driver configuration
    add_definitions( -DTS_DRIVER_SOCKET )   # SERIAL, SOCKET, UART or CUSTOM
    # add_definitions( -DTS_UNIX_SIMPLE_SOCKET ) # Use simplified socket connection (required for mocana)
    # add_definitions( -DTS_PLATFORM_UNIX ) # Use for OS based command-line tools

    # build example executables
    add_subdirectory( examples )

endif()


