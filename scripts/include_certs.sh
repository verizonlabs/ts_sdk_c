#!/bin/bash
# Copyright(C) 2017, 2018 Verizon. All rights reserved.

# Script generates c header files from the provided pem format of server certificate,
# device certificate and device private key and installs them at the relative provided path
# All the parameters are mandatory and script must be run from the root of the repository

# Must have a recent (2017) version of openssl installed

if [ $# -lt 4 ]; then
  echo "This script takes 4 params"
  echo
  echo "$0 <path to ca cert> <path to client certificate> <path to client private key> <path to install directory>"
  echo "Example (assuming script is being executed from the top level of the project):"
  echo "$0 ~/Downloads/thingspaceserver.pem ~/Downloads/357353080059530.cert.pem ~/Downloads/357353080059530.private.key ./examples/applications/simple/include"
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

#parse ca cert pem formated file
openssl x509 -C -in $1 -out cacert.temp
check_success "openssl command failed"

sed -n '/the_certificate/,/}/ w cacert' cacert.temp
check_success "sed command failed"
sed -i 's/unsigned char the_certificate/static const unsigned char cacert_buf/' cacert
check_success "sed command to generate cacert failed"
echo '#define VERIZON_CA_H' | cat - cacert > temp && mv temp cacert
echo '#ifndef VERIZON_CA_H' | cat - cacert > temp && mv temp cacert
echo '/* Copyright(C) 2017 Verizon. All rights reserved. */' | cat - cacert > temp && mv temp cacert
echo "#endif" >> cacert
mv cacert $APP_INC/cacert.h
check_success "generatig include header failed for ca cert"
rm -rf cacert.temp

#parse client certificate
openssl x509 -C -in $2 >clcert.temp
check_success "openssl command failed for parsig client certificate"
sed -n '/the_certificate/,/}/ w clcert' clcert.temp
check_success "sed command failed"
sed -i 's/unsigned char the_certificate/static const unsigned char client_cert/' clcert
check_success "sed command to generate cacert failed"
echo '#define VERIZON_CL_H' | cat - clcert > temp && mv temp clcert
echo '#ifndef VERIZON_CL_H' | cat - clcert > temp && mv temp clcert
echo '/* Copyright(C) 2017 Verizon. All rights reserved. */' | cat - clcert > temp && mv temp clcert
echo "#endif" >> clcert
mv clcert $APP_INC/client-crt.h
check_success "generatig include header failed for client cert"
rm -rf clcert.temp

#parse client private key
openssl rsa -in $3 -out clkey.der.temp -outform DER
check_success "openssl command failed for parsig client key"
cat clkey.der.temp | xxd -i >clkey
check_success "xxd command failed"
echo 'static const unsigned char client_key[] = {' | cat - clkey > temp && mv temp clkey
echo '#define VERIZON_CLK_H' | cat - clkey > temp && mv temp clkey
echo '#ifndef VERIZON_CLK_H' | cat - clkey > temp && mv temp clkey
echo '/* Copyright(C) 2017 Verizon. All rights reserved. */' | cat - clkey > temp && mv temp clkey
echo '};' >> clkey
echo "#endif" >> clkey
mv clkey $APP_INC/client-key.h
check_success "generatig include header failed for client key"
rm -rf clkey.der.temp

exit 0
