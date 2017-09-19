#!/bin/bash

# Generate key and cert files for GRPC
# Usage ./genkey.sh <filename-prefix>

PASSWORD=`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1`
PREFIX=$1
if [ -z $PREFIX ]; then PREFIX="dev"; fi

# Generate Root CA files
openssl genrsa -passout pass:$PASSWORD -des3 -out $PREFIX'CA.key' 4096
openssl req -passin pass:$PASSWORD -new -x509 -days 3650 -key $PREFIX'CA.key' -out $PREFIX'CA.crt' -subj '/C=US/ST=CA/L=San Jose/O=MyApp/CN=grpc.local/emailAddress=guest@.pensando.io'
openssl rsa -passin pass:$PASSWORD -in $PREFIX'CA.key' -out $PREFIX'CA.key'

# Generate server certificates
openssl genrsa -passout pass:$PASSWORD -des3 -out $PREFIX'Server.key' 4096
openssl req -passin pass:$PASSWORD -new -key $PREFIX'Server.key' -out $PREFIX'Server.csr' -subj '/C=US/ST=CA/L=San Jose/O=MyApp/CN=grpc.local/emailAddress=guest@pensando.io'
openssl x509 -passin pass:$PASSWORD -req -days 365 -in $PREFIX'Server.csr' -CA $PREFIX'CA.crt' -CAkey $PREFIX'CA.key' -set_serial 01 -out $PREFIX'Server.crt'
openssl rsa -passin pass:$PASSWORD -in $PREFIX'Server.key' -out $PREFIX'Server.key'
rm $PREFIX'Server.csr'

# Generate client certificates
openssl genrsa -passout pass:$PASSWORD -des3 -out $PREFIX'Client.key' 4096
openssl req -passin pass:$PASSWORD -new -key $PREFIX'Client.key' -out $PREFIX'Client.csr' -subj '/C=US/ST=CA/L=San Jose/O=MyApp/CN=grpc.local/emailAddress=guest@pensando.io'
openssl x509 -passin pass:$PASSWORD -req -days 365 -in $PREFIX'Client.csr' -CA $PREFIX'CA.crt' -CAkey $PREFIX'CA.key' -set_serial 01 -out $PREFIX'Client.crt'
openssl rsa -passin pass:$PASSWORD -in $PREFIX'Client.key' -out $PREFIX'Client.key'
rm $PREFIX'Client.csr'
