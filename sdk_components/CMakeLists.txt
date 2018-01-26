# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace SDK Components

set( TARGETS ts_sdk_components )

file( GLOB SOURCES
    "*.c"
    "service/*.c"
    "transport/*.c"
    "security/*.c"
    "controller/*.c" )

add_library( ts_sdk_components OBJECT ${SOURCES} )

target_include_directories( ts_sdk_components PRIVATE
    ${CMAKE_SOURCE_DIR}/sdk/include
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTPacket/src
    ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTClient-C/src )
