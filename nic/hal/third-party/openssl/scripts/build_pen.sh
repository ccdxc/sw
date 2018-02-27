#!/bin/bash
TOPDIR=`pwd`
EXPORTDIR=$TOPDIR/export
OPENSSLDIR=$EXPORTDIR/openssl
CONFIG_OPTION=-DOPENSSL_SSL_TRACE_CRYPTO 
#CONFIG_OPTION=-DOPENSSL_SSL_TRACE_CRYPTO -d 
echo "Compiling with output directory: $EXPORTDIR"
# Uncomment for Debug build 
./config no-afalgeng $CONFIG_OPTION --prefix=$EXPORTDIR --openssldir=$OPENSSLDIR && make -j 4 && make install_sw
