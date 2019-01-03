#!/bin/sh
#
# Script for building freebsd modules and libraries out-of-tree.
#
# Copy or mount the Pensando freebsd drivers sources on the freebsd system.
# Run this script, with OS_DIR referring to the freebsd os source tree.
#
#   env OS_DIR=/home/anish/freebsd_11 ./build.sh
#
# The following system changes are prerequisite for RDMA:
#
# edit in /etc/src.conf:
#
#   WITH_OFED='yes'
#
# edit in $OS_DIR/sys/amd64/conf/GENERIC:
#
#   option OFED
#   option COMPAT_LINUXKPI
#
# run in $OS_DIR:
#
#   make -j <num> buildworld buildkernel installworld installkernel
#

if [ "x" == "x$OS_DIR" ] ; then
	OS_DIR=/usr/src
fi

# if OS_DIR was relative, make it absolute here
OS_DIR=$(readlink -f "$OS_DIR")

make_ext() {
	make "-m$OS_DIR/share/mk" "SYSDIR=$OS_DIR/sys" \
		"DEBUG_FLAGS=-g" "-C$1" clean cleandepend || exit
	make "-m$OS_DIR/share/mk" "SYSDIR=$OS_DIR/sys" \
		"DEBUG_FLAGS=-g" "-C$1" || exit
}

make_ext sys/modules/ionic
make_ext sys/modules/ionic_rdma

# Proceed building user space RDMA only if OFED was enabled
[ -e /usr/include/infiniband/verbs.h ] || exit 0
make_ext contrib/ofed/libionic

cd perftest
patch -f -p1 < ../perftest-freebsd.patch
./configure || exit
make -j12 || exit
cd -
