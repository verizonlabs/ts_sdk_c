# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Dependencies

# #############################################################################
# library, mbedTLS

set( ENABLE_PROGRAMS OFF CACHE BOOL "Disable mbedtls program compilation" FORCE )
set( ENABLE_TESTING OFF CACHE BOOL "Disable mbedtls testing compilation" FORCE )
add_subdirectory( mbedtls )

# #############################################################################
# library, TINY-CBOR

set( TARGETS tinycbor )
file( GLOB SOURCES tinycbor/src/*.[ch] )

add_library( tinycbor STATIC ${SOURCES} )
target_compile_definitions( tinycbor PRIVATE CBOR_PARSER_NO_STRICT_CHECKS=TRUE )

# #############################################################################
# library, CJSON

set( TARGETS cjson )
file( GLOB SOURCES cJSON/cJSON.[ch] )

add_library( cjson STATIC ${SOURCES} )
target_compile_definitions( cjson PRIVATE ENABLE_CJSON_TEST=OFF )

# #############################################################################
# library, PAHO-MQTT

set( TARGETS pahomqtt )
file( GLOB SOURCES paho.mqtt.embedded-c/MQTTPacket/src/*.[ch] paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c )

add_library( pahomqtt STATIC ${SOURCES} )
target_include_directories( pahomqtt PRIVATE
    paho.mqtt.embedded-c/MQTTPacket/src
    paho.mqtt.embedded-c/MQTTClient-C/src
    cJSON
    tinycbor/src
    ${CMAKE_SOURCE_DIR}/sdk/include
    ${CMAKE_SOURCE_DIR}/sdk_components/transport )
target_compile_definitions( pahomqtt PRIVATE
    MQTTCLIENT_PLATFORM_HEADER=ts_transport_mqtt.h
    MQTTCLIENT_QOS2=1 )
