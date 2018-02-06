# Copyright (C) 2017, Verizon, Inc. All rights reserved.
set( TARGETS ts_sdk_platforms )

file( GLOB SOURCES
        "*.c"
        "ts_sdk_c_platforms_${TS_PLATFORM}/*.c" )

add_library( ts_sdk_platforms OBJECT ${SOURCES} )

target_include_directories( ts_sdk_platforms PRIVATE
        ${CMAKE_SOURCE_DIR}/sdk/include
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTPacket/src
        ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTClient-C/src )
