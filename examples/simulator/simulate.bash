#!/usr/bin/env bash
# this script is run from your host machine
# it will run the docker container and execute the simulator

# run the simple example
docker run -ti simulator-simulate /binary/examples/applications/example_simple ${1}
