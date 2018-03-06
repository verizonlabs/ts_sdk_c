# Copyright (C) 2018, Verizon, Inc. All rights reserved.
# Synchronize private and public submodules

# TODO - should use .gitmodules as a source
# [submodule "sdk_dependencies/tinycbor"]
#   path = sdk_dependencies/tinycbor
#   url = https://github.com/verizonlabs/tinycbor.git
# [submodule "sdk_dependencies/cJSON"]
#   path = sdk_dependencies/cJSON
#   url = https://github.com/verizonlabs/cJSON.git
# [submodule "sdk_dependencies/paho.mqtt.embedded-c"]
#   path = sdk_dependencies/paho.mqtt.embedded-c
#   url = https://github.com/verizonlabs/paho.mqtt.embedded-c.git
# [submodule "sdk_dependencies/mbedtls"]
#   path = sdk_dependencies/mbedtls
#   url = https://github.com/verizonlabs/mbedtls.git
# [submodule "sdk_dependencies/mocana_ssl"]
#   path = sdk_dependencies/mocana_ssl
#   url = https://github.com/verizonlabs/mocana_ssl.git
# [submodule "examples/platforms/ts_sdk_c_platforms_unix"]
#   path = examples/platforms/ts_sdk_c_platforms_unix
#   url = https://github.com/verizonlabs/ts_sdk_c_platforms_unix.git
# [submodule "examples/platforms/ts_sdk_c_platforms_unix_raspberry-pi3"]
#   path = examples/platforms/ts_sdk_c_platforms_unix_raspberry-pi3
#   url = https://github.com/verizonlabs/ts_sdk_c_platforms_unix_raspberry-pi3.git
# [submodule "examples/platforms/ts_sdk_c_platforms_none_nucleo-f401re"]
#   path = examples/platforms/ts_sdk_c_platforms_none_nucleo-f401re
#   url = https://github.com/verizonlabs/ts_sdk_c_platforms_none_nucleo-f401re.git
# [submodule "examples/platforms/ts_sdk_c_platforms_none_nucleo-l476"]
#   path = examples/platforms/ts_sdk_c_platforms_none_nucleo-l476
#   url = https://github.com/verizonlabs/ts_sdk_c_platforms_none_nucleo-l476.git
# [submodule "sdk_dependencies/mbedtls_custom"]
#   path = sdk_dependencies/mbedtls_custom
#   url = https://github.com/verizonlabs/mbedtls_custom.git
# [submodule "sdk_dependencies/mocana"]
#   path = sdk_dependencies/mocana
#   url = https://github.com/verizonlabs/mocana.git
# [submodule "examples/platforms/ts_sdk_c_platforms_none_frdm-k82f"]
#   path = examples/platforms/ts_sdk_c_platforms_none_frdm-k82f
#   url = https://github.com/verizonlabs/ts_sdk_c_platforms_none_frdm-k82f

function(SynchronizeSubmodules)

    find_package(Git)
    if( NOT GIT_FOUND )
        message( STATUS "## error: git not found" )
        return()
    endif()

    set( TS_SUBMODULE_DEPENDENCIES
        sdk_dependencies/tinycbor
        sdk_dependencies/cJSON
        sdk_dependencies/paho.mqtt.embedded-c
        sdk_dependencies/mbedtls_custom
        sdk_dependencies/mocana )

    foreach( ARG ${TS_SUBMODULE_DEPENDENCIES} )

        #execute_process( COMMAND "${GIT_EXECUTABLE} submodule update --init -- ${ARG}" RESULT_VARIABLE CMD_ERROR WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        execute_process( COMMAND ${GIT_EXECUTABLE} submodule update --init ${ARG} RESULT_VARIABLE CMD_ERROR WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
        if( CMD_ERROR )
            message( STATUS "## dependency update error : ${CMD_ERROR}" )
        endif()

    endforeach()

    set( TS_SUBMODULE_PLATFORMS
        examples/platforms/ts_sdk_c_platforms_unix
        examples/platforms/ts_sdk_c_platforms_unix_raspberry-pi3
        examples/platforms/ts_sdk_c_platforms_none_nucleo-l476
        examples/platforms/ts_sdk_c_platforms_none_frdm-k82f )

    foreach( ARG ${TS_SUBMODULE_PLATFORMS} )

        execute_process( COMMAND ${GIT_EXECUTABLE} submodule update --init ${ARG} RESULT_VARIABLE CMD_ERROR WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
        if( CMD_ERROR )
            message( STATUS "## dependency update error : ${CMD_ERROR}" )
        endif()

    endforeach()

endfunction()

message( STATUS "## Synchronize Submodules" )
SynchronizeSubmodules()
