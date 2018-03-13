# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Example Executables

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )


function (load_link_time_settings target_dest target_platform)
	get_target_property(ldscript ${target_platform} LINK_FLAGS)
	if ( NOT ldscript )
		return ()
	endif()
	set_target_properties( ${target_dest} PROPERTIES LINK_FLAGS ${ldscript} )
endfunction()

# executable, SIMPLE application
add_executable( example_simple simple/main.c $<TARGET_OBJECTS:ts_sdk_platforms> )
load_link_time_settings(example_simple ts_sdk_platforms)
target_link_libraries( example_simple ts_sdk )
