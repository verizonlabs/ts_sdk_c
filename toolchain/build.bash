#!/usr/bin/env bash

# create toolchain docker (if not done already)
docker build -t toolchain .

# build project
cd ..
docker run -v $(pwd):/source -ti toolchain ./toolchain/cmake.bash
