#!/bin/sh
# Set OS_DIR to point to FreeBSD source code.
# example OS_DIR=/home/anish/freebsd
: ${OS_DIR:=/usr/src}
export OS_DIR=/home/anish/freebsd

if [ -z ${OS_DIR} ]
then
	echo OS_DIR should be set, example /usr/src
	exit 1
fi


make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -f Makefile.bsd clean cleandepend
make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -f Makefile.bsd

make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -f Makefile-pencake.bsd clean cleandepend
make -m $OS_DIR/share/mk SYSDIR=$OS_DIR/sys -f Makefile-pencake.bsd

if [ ! -f pencake.ko ]
then
	echo Failed to build pnso.ko
	exit 1
fi
