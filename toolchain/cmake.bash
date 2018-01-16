#!/usr/bin/env bash
# note, this script is expected to run from build.bash,
# that is, via a docker container (hence the use of, '/source')

# build makefiles
cmake /source -B/source/cmake-build-debug -DTOOLCHAIN=ON -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles"

# build libraries
cd /source/cmake-build-debug
make
