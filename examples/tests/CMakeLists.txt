# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Tests

include_directories( ../platforms )
include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_components )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include )

# TODO: This currently compiles only for OS based systems.
#add_executable( test_driver test_driver.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#target_link_libraries( test_driver ts_sdk )

add_executable( test_message test_message.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_message ts_sdk )

add_executable( test_socket test_socket.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_socket ts_sdk )

add_executable( test_socket_tls test_socket_tls.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_socket_tls ts_sdk )

add_executable( test_transport_mqtt test_transport_mqtt.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_transport_mqtt ts_sdk )

add_executable( test_transport_mqtt_tls test_transport_mqtt_tls.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_transport_mqtt_tls ts_sdk )

add_executable( test_debug_interface test_debug_interface.c $<TARGET_OBJECTS:ts_sdk_platforms> )
target_link_libraries( test_debug_interface ts_sdk )

# TODO: This is platform specific. It should be moved to example/platforms/...
set( STM32_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32L476RGTx.ld" )
STM32_SET_TARGET_PROPERTIES(test_debug_interface)
