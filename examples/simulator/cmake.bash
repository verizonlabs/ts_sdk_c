#!/usr/bin/env bash
# this script is expected to run from build.bash via a docker container

# remove cmake directory
rm -rf /source/cmake-build-debug

# build makefiles
cmake /source -B/source/cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -G "CodeBlocks - Unix Makefiles"

# build libraries
cd /source/cmake-build-debug
make
