#!/bin/sh
# Set OS_DIR to point to FreeBSD source code.
# example OS_DIR=/home/anish/freebsd

if [ -z ${OS_DIR} ]
then
	echo OS_DIR should be set, example /usr/src
	exit 1
fi

make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -C ionic clean cleandepend
make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -C ionic 

if [ ! -f ionic/ionic.ko ]
then
	echo Failed to build ionic.ko
	exit 1
fi
