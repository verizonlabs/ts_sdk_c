# Copyright (C) 2017, Verizon, Inc. All rights reserved.

# example platforms
add_subdirectory( platforms )

# example applications
add_subdirectory( applications )

# determine if this will build tests or not
#option( BUILD_TESTS "Compile example tests" ON )
option( BUILD_TESTS "Compile example tests" OFF )
message( STATUS "## Build Tests                  (ON|OFF) : ${BUILD_TESTS}" )

# example tests
if( BUILD_TESTS )
    add_subdirectory( tests )
endif()
