#!/bin/bash
# unload and reload the FW rules before starting the app. When the app finished,
# kill the firewall rules

# trap control c and kill the firewall

function kill() {
	echo "Kill the application and the firewalls"
	rmmod nanowallk
	echo "Done"
}
trap kill INT

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 
   exit 1
fi

set -x
rmmod nanowallk
insmod ../sdk_dependencies/mocana/projects/raspbian/nanowall/nanowallk/nanowallk.ko
../cmake-build-debug/examples/applications/example_simple
i# Unload if it returns
rmmod nanowallk

