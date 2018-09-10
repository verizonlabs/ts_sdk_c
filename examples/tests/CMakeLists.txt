# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Tests

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_components )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/mbedtls/include )

function (load_link_time_settings target_dest target_platform)
	get_target_property(ldscript ${target_platform} LINK_FLAGS)
	if ( NOT ldscript )
		return ()
	endif()
	set_target_properties( ${target_dest} PROPERTIES LINK_FLAGS ${ldscript} )
endfunction()

# TODO: This currently compiles only for OS based systems.
#add_executable( test_driver test_driver.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#target_link_libraries( test_driver ts_sdk )

add_executable( test_message test_message.c $<TARGET_OBJECTS:ts_sdk_platforms> )
load_link_time_settings(test_message ts_sdk_platforms)
target_link_libraries( test_message ts_sdk )

add_executable( test_socket test_socket.c $<TARGET_OBJECTS:ts_sdk_platforms> )
load_link_time_settings(test_socket ts_sdk_platforms)
target_link_libraries( test_socket ts_sdk )

# add_executable( test_socket_tls test_socket_tls.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#load_link_time_settings(test_socket_tls ts_sdk_platforms)
#target_link_libraries( test_socket_tls ts_sdk )

#add_executable( test_transport_mqtt test_transport_mqtt.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#load_link_time_settings(test_transport_mqtt ts_sdk_platforms)
#target_link_libraries( test_transport_mqtt ts_sdk )

#add_executable( test_transport_mqtt_tls test_transport_mqtt_tls.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#load_link_time_settings(test_transport_mqtt_tls ts_sdk_platforms)
#target_link_libraries( test_transport_mqtt_tls ts_sdk )

#add_executable( test_debug_interface test_debug_interface.c $<TARGET_OBJECTS:ts_sdk_platforms> )
#load_link_time_settings(test_debug_interface ts_sdk_platforms)
#target_link_libraries( test_debug_interface ts_sdk )

#add_executable( test_firewall test_firewall.c $<TARGET_OBJECTS:ts_sdk_platforms> )

add_executable( test_files test_files.c $<TARGET_OBJECTS:ts_sdk_platforms> )
load_link_time_settings( test_files ts_sdk_platforms )
target_link_libraries( test_files ts_sdk )

