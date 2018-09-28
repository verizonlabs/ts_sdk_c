#!/bin/bash
# Copyright(C) 2018, 2018 Verizon. All rights reserved.

# Script generates DER files from the provided pem format of server certificate,
# device certificate and device private key and installs at the directory for read only certificates/keys for TS
# All the parameters are mandatory and script must be run from the root of the repository

if [[ $EUID -ne 0 ]]; then

   echo "This script must be run as sudo as it creates a directory in /usr" 

   exit 1

fi
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
mocana_nanowall_dir="../sdk_dependencies/mocana/projects/raspbian/nanowall/nanowallk/"
home_dir="/home/pi/"
cpu="/proc/cpuinfo"
rpi_id=$( [[ -e $cpu ]] && grep Hardware $cpu | cut -d ":" -f 2 | tr -d ' ')

cert_prefix=""
if [[ "$rpi_id" == "BCM2835" ]]; then
	echo "Running on the Raspberry Pi 3"
	cert_prefix="the"
        mf_cert_dir="/usr/share/thingspace/conf/"
        op_cert_dir="/var/lib/thingspace/"
else
	echo "Running on regular unix"
        mf_cert_dir="."
	mf_cert_prefix="XXX"
	op_cert_prefix="XXX"
fi
# Create the directory
if [ ! -d "${mf_cert_dir}" ]; then
	mkdir -p  ${mf_cert_dir}
else
	rm -rf ${mf_cert_dir}
	mkdir -p  ${mf_cert_dir}
fi
# Create the directory
if [ ! -d "${op_cert_dir}" ]; then
	mkdir -p  ${op_cert_dir}
else
	rm -rf ${op_cert_dir}
#create the opcert, firewll config and scep config folders
	mkdir -p /var/lib/thingspace/{certs,firewall,scep}
fi
#parse ca cert pem formated file
openssl x509 -C -in $1 > ${mf_cert_dir}cacert.der
echo "openssl x509 -C -in $1 > ${mf_cert_dir}cacert.der"
check_success "openssl command failed"

#parse client certificate
openssl x509 -in $2 -outform der -out  ${mf_cert_dir}clcert.der
openssl x509 -in $2 -outform der -out  ${op_cert_dir}scep/requester_cert.der
echo "openssl x509 -in $2 -outform der -out  ${mf_cert_dir}clcert.der"
echo "openssl x509 -in $2 -outform der -out  ${op_cert_dir}scep/requester_cert.der"
check_success "openssl command failed for parsing client certificate"

#parse client private key
openssl rsa -in $3 -out  ${mf_cert_dir}clkey.der -outform DER
echo "openssl rsa -in $3 -out  ${mf_cert_dir}clkey.der -outform DER"
check_success "openssl command failed for parsing client key"


cp $3 ${op_cert_dir}/scep/GenKeyBlob
cp $3 ${op_cert_dir}/scep/GenKeyBlob.pem
echo "copied Private keys to opcert dir"

cp ./preconf/sample_scep_csr.cnf ${op_cert_dir}/scep
cp ./preconf/vz_CA.der ${op_cert_dir}/scep
cp ./preconf/vz_CEP.der ${op_cert_dir}/scep
echo "copied SCEP files"

cp ./preconf/install-nanowallk.sh ${home_dir}
chmod +x ${home_dir}install-nanowallk.sh
cp ./preconf/uninstall-nanowallk.sh ${home_dir}
chmod +x ${home_dir}uninstall-nanowallk.sh

echo "copied install and uninstall scripts to home dir"

cd ${mocana_nanowall_dir}
make
cp ./nanowallk.ko ${home_dir}
echo "copied ko file to home dir"
cd -

echo "The Manufacturer Certificates are written to the key store in ${mf_cert_dir}"
echo "Whitelist the IP address under nameserver in the security profile on TS Portal. Port: 53  Protocol: UDP"

cat /etc/resolv.conf

exit 0

