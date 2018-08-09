# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Dependencies

# get current compiler directives
get_directory_property( DEFINITIONS COMPILE_DEFINITIONS )

# #############################################################################
# optional library, mocana
if( DEFINITIONS MATCHES TS_SECURITY_MOCANA )
    add_subdirectory( mocana )

    if ( DEFINITIONS MATCHES TS_FIREWALL_CUSTOM )
        add_subdirectory( mocana/projects/raspbian/nanowall/nanowallk )
    endif()

endif()

# #############################################################################
# optional library, mbedTLS

if( DEFINITIONS MATCHES TS_SECURITY_MBED )
    set( ENABLE_PROGRAMS OFF CACHE BOOL "Disable mbedtls program compilation" FORCE )
    set( ENABLE_TESTING OFF CACHE BOOL "Disable mbedtls testing compilation" FORCE )
    add_subdirectory( mbedtls )
endif()

# #############################################################################
# library, TINY-CBOR

set( TARGETS tinycbor )
file( GLOB SOURCES tinycbor/src/*.[ch] )

add_library( tinycbor OBJECT ${SOURCES} )
target_compile_definitions( tinycbor PRIVATE CBOR_PARSER_NO_STRICT_CHECKS=TRUE )

# #############################################################################
# library, CJSON

set( TARGETS cjson )
file( GLOB SOURCES cJSON/cJSON.[ch] )

add_library( cjson OBJECT ${SOURCES} )
target_compile_definitions( cjson PRIVATE ENABLE_CJSON_TEST=OFF )

# #############################################################################
# library, PAHO-MQTT

set( TARGETS pahomqtt )
file( GLOB SOURCES paho.mqtt.embedded-c/MQTTPacket/src/*.[ch] paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.c )

add_library( pahomqtt OBJECT ${SOURCES} )
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
