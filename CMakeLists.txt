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
add_definitions( -D__GLIBC__ )              # TODO, remove tinyCBOR HACK
add_subdirectory( sdk_dependencies )

# component selection
add_definitions( -DTS_SERVICE_TS_JSON )     # TS_JSON, TS_CBOR or CUSTOM
add_definitions( -DTS_TRANSPORT_MQTT )      # MQTT or CUSTOM
add_definitions( -DTS_SECURITY_MBED )       # MBED, MOCANA, NONE or CUSTOM
add_definitions( -DTS_CONTROLLER_MONARCH )     # MONARCH, NONE or CUSTOM

# build object libraries
add_subdirectory( sdk_components )
add_subdirectory( sdk )

# build common static library
# (depends on components and dependencies)
add_library( ts_sdk STATIC $<TARGET_OBJECTS:ts_sdk_apis> $<TARGET_OBJECTS:ts_sdk_components> )
target_link_libraries( ts_sdk tinycbor cjson mbedtls pahomqtt )

# build optional examples
option( BUILD_EXAMPLES "Compile example platforms and applications" OFF )
message( STATUS "## Build Examples               (ON|OFF) : ${BUILD_EXAMPLES}" )
if( BUILD_EXAMPLES )

    # platform kernal and board configuration
    #
    #   unix                    Linux and Unix variants
    #   unix_raspberry-pi3      Raspberry Pi3 Linux and Unix variants
    #   threadx_synergy-s5d9    ThreadX Renesas Synergy S5D9
    #   none_freedom-k82f       Bare-metal NXP Freedom K82F
    #   none_nucleo-f401re      Bare-metal STMicro Nucleo F401RE
    #   none_nucleo-l476        Bare-metal STMicro Nucleo L476
    #   windows                 Microsoft Windows variants
    set( TS_PLATFORM none_nucleo-l476 CACHE STRING "RTOS and Hardware Platform" FORCE )

    # platform driver configuration
    add_definitions( -DTS_DRIVER_UART )   # SERIAL, SOCKET, UART or CUSTOM
    add_definitions( -DTS_PLATFORM_NONE )   # TODO, remove,...

    # build example executables
    add_subdirectory( examples )

endif()


