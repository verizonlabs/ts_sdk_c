#! /bin/bash

set -e

[ `id -u` = 0 ] || (echo "This script must be run as root"; exit -2)

echo "Copying NanoWall driver"
cp /home/pi/nanowallk.ko /lib/modules/$(uname -r)/

echo "Installing NanoWall driver"
depmod -a

echo "nanowallk" > /tmp/nanowallk.conf
mv /tmp/nanowallk.conf /etc/modules-load.d/nanowallk.conf

echo "Starting NanoWall driver"
modprobe  nanowallk
