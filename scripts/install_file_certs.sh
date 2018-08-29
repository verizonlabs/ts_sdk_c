#!/bin/bash
# Copyright(C) 2018, 2018 Verizon. All rights reserved.

# Script generates DER files from the provided pem format of server certificate,
# device certificate and device private key and installs them at the relative provided path
# All the parameters are mandatory and script must be run from the root of the repository

# Must have GNU sed installed

if [ $# -lt 3 ]; then
  echo "This script takes 3 (T h r e e)  params"
  echo "This script takes 3 (T h r e e)  params"
  echo "This script takes 3 (T h r e e)  params"
  echo "This script takes 3 (T h r e e)  params"
  echo
  echo "$0 <path to ca cert> <path to client certificate> <path to client private key> "
  echo "Example (assuming script is being executed from the top level of the project):"
  echo "$0 ~/Downloads/thingspaceserver.pem ~/Downloads/357353080059530.cert.pem ~/Downloads/357353080059530.private.key "
  echo
  exit 1
fi

APP_INC=$4

check_success()
{
	if ! [ $? -eq 0 ]; then
		echo "$1"
		rm *.temp
		exit 1
	fi
}

cpu="/proc/cpuinfo"
rpi_id=$( [[ -e $cpu ]] && grep Hardware $cpu | cut -d ":" -f 2 | tr -d ' ')

cert_prefix=""
if [[ "$rpi_id" == "BCM2835" ]]; then
	echo "Running on the Raspberry Pi 3"
	cert_prefix="the"
        cert_dir="/usr/share/thingspace/conf/"
else
	echo "Running on regular unix"
        cert_dir="."
	cert_prefix="XXX"
fi

#parse ca cert pem formated file
openssl x509 -C -in $1 > ${cert_dir}cacert.der
echo "openssl x509 -C -in $1 > ${cert_dir}cacert.der"
check_success "openssl command failed"


#parse client certificate
openssl x509 -in $2 -outform der -out  ${cert_dir}clcert.der
echo "openssl x509 -in $2 -outform der -out  ${cert_dir}clcert.der"
check_success "openssl command failed for parsig client certificate"

#parse client private key
openssl rsa -in $3 -out  ${cert_dir}clkey.der -outform DER
echo "openssl rsa -in $3 -out  ${cert_dir}clkey.der -outform DER"
check_success "openssl command failed for parsig client key"

echo "Your files should be in the key store in ${cert_dir}"
exit 0

