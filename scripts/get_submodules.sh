#!/bin/bash

echo "Attempting to get all submodules and checking out the appropriate branches"
gitroot=$(git rev-parse --show-toplevel)

echo "GIT root: $gitroot"
(cd $gitroot && git checkout develop)
(cd $gitroot && git submodule update --init --recursive)
(cd $gitroot && git submodule foreach --recursive git checkout master)
(cd sdk_dependencies/mbedtls_custom && git checkout develop)
