# Copyright (C) 2017, Verizon, Inc. All rights reserved.
# ThingSpace Example Executables

include_directories( ${CMAKE_SOURCE_DIR}/sdk/include )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/cJSON )
include_directories( ${CMAKE_SOURCE_DIR}/sdk_dependencies/tinycbor/src )

include_directories( ${CMAKE_SOURCE_DIR}/examples/platforms )

# executable, SIMPLE application
add_executable( example_simple simple/main.c)
target_link_libraries( example_simple ts_sdk ts_sdk_platforms mbedtls pahomqtt )

# executable, CUSTOM-TRANSPORT application
add_executable( example_custom_transport custom_transport/main.c)
target_link_libraries( example_custom_transport ts_sdk ts_sdk_platforms )

# executable, CUSTOM-SECURITY application
add_executable( example_custom_security custom_security/main.c)
target_link_libraries( example_custom_security ts_sdk ts_sdk_platforms )

