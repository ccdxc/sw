#!/bin/sh
# Set OS_DIR to point to FreeBSD source code.
# example OS_DIR=/home/anish/freebsd
if [ -z ${OS_DIR} ]
then
	echo OS_DIR should point to FreeBSD src, example /usr/src
	exit 1
fi

: ${IONIC_BUILD:=sys/modules/ionic}

make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -C $IONIC_BUILD clean cleandepend
make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -C $IONIC_BUILD

if [ ! -f $IONIC_BUILD/ionic.ko ]
then
	echo Failed to build ionic.ko
	exit 1
fi

