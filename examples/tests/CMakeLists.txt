# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Tests

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_components )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include )
include_directories( ${CMAKE_SOURCE_DIR}/examples/platforms )

add_executable( test_message test_message.c)
target_link_libraries( test_message ts_sdk ts_sdk_components ts_sdk_platforms mbedtls pahomqtt )

add_executable( test_socket test_socket.c)
target_link_libraries( test_socket ts_sdk ts_sdk_components ts_sdk_platforms mbedtls pahomqtt )

add_executable( test_socket_tls test_socket_tls.c)
target_link_libraries( test_socket_tls ts_sdk ts_sdk_components ts_sdk_platforms mbedtls pahomqtt )

add_executable( test_transport_mqtt test_transport_mqtt.c)
target_link_libraries( test_transport_mqtt ts_sdk ts_sdk_components ts_sdk_platforms mbedtls pahomqtt )

add_executable( test_transport_mqtt_tls test_transport_mqtt_tls.c)
target_link_libraries( test_transport_mqtt_tls ts_sdk ts_sdk_components ts_sdk_platforms mbedtls pahomqtt )
