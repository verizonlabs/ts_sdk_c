#!/usr/bin/env bash
# this script is run from your host machine
# it will run the docker container execute build.bash

# create simulator builder container (if not done already)
docker build -t simulator-builder -f Dockerfile.builder .

# build the ts-sdk project with the examples
docker run -v $(pwd)/../..:/source -ti simulator-builder /source/examples/simulator/cmake.bash

# copy resulting build to local binary folder
cp -R $(pwd)/../../cmake-build-debug binary

# now, create the simulator simulate container
docker build -t simulator-simulate -f Dockerfile.simulate .

# clean up
rm -rf binary
