# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Dependencies

# library, CBOR
set( CBOR_PARSER_NO_STRICT_CHECKS TRUE )
file( GLOB SOURCES tinycbor/src/*.[ch] )
add_library( tinycbor STATIC ${SOURCES} )

# library, JSON
set( ENABLE_CJSON_TEST OFF )
file( GLOB SOURCES cJSON/cJSON.[ch] )
add_library( cjson STATIC ${SOURCES} )

# library, mbedTLS
# TODO - run( cd mbedtls; ./vz_configure_mbedtls no-net )
set( ENABLE_PROGRAMS OFF CACHE BOOL "Disable mbedtls program compilation" FORCE )
set( ENABLE_TESTING OFF CACHE BOOL "Disable mbedtls testing compilation" FORCE )
add_subdirectory( mbedtls )
# TODO - rerun toolchain?

# library, paho.mqtt.embeded-c
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )

set( TARGETS pahomqtt )
file( GLOB SOURCES paho.mqtt.embedded-c/MQTTPacket/src/*.[ch] paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c )
add_library( pahomqtt STATIC ${SOURCES} )
target_include_directories( pahomqtt PRIVATE
        paho.mqtt.embedded-c/MQTTPacket/src
        paho.mqtt.embedded-c/MQTTClient-C/src
        ${CMAKE_SOURCE_DIR}/sdk/include
        ${CMAKE_SOURCE_DIR}/sdk_components/transport )
target_compile_definitions( pahomqtt PRIVATE
        MQTTCLIENT_PLATFORM_HEADER=ts_transport_mqtt.h
        MQTTCLIENT_QOS2=1 )
