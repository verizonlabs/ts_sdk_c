#!/bin/bash

echo "Attempting to get all submodules and checking out the appropriate branches"
gitroot=$(git rev-parse --show-toplevel)

echo "GIT root: $gitroot"
(cd $gitroot && git checkout develop)
(cd $gitroot && git submodule update --init --recursive)

# Checkout master on all branches..
(cd $gitroot && git submodule foreach --recursive git checkout master)

# ..except the following.
for i in $( ls -d $gitroot/examples/platforms/*/ ); do
	(cd $i && git checkout develop)
done
(cd $gitroot/sdk_dependencies/mbedtls_custom && git checkout develop)
