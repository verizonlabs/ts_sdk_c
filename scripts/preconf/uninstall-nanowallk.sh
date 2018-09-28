#!/bin/bash

set -e

[ `id -u` = 0 ] || (echo "This script must be run as root"; exit -2)


echo "Removing NanoWall driver"

rm /lib/modules/$(uname -r)/nanowallk.ko 
depmod -a

rm /etc/modules-load.d/nanowallk.conf
#sed -i '/nanowallk/d' /etc/modules

rmmod nanowallk
