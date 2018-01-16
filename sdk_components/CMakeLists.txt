# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace SDK Components

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTPacket/src )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/paho.mqtt.embedded-c/MQTTClient-C/src )

file( GLOB SOURCES
        "*.c"
        "service/*.c"
        "transport/*.c"
        "security/*.c"
        "controller/*.c" )

add_library( ts_sdk_components STATIC ${SOURCES} )
