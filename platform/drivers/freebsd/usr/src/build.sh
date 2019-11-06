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

# Hacky: "root@freebsd:/usr/obj/usr/src/sys/GENERIC"
# Can't rely on uname -i, unfortunately.
KERNCONF=`uname -v | awk -F/ '{print $NF}'`

make_ext() {
	make "-m$OS_DIR/share/mk" "SYSDIR=$OS_DIR/sys" "KERNCONF=$KERNCONF" \
		"DEBUG_FLAGS=-g" "-C$1" clean cleandepend || exit
	make "-m$OS_DIR/share/mk" "SYSDIR=$OS_DIR/sys" "KERNCONF=$KERNCONF" \
		"DEBUG_FLAGS=-g" "-C$1" || exit
	# uncomment for static analysys with clang
	# make "-m$OS_DIR/share/mk" "SYSDIR=$OS_DIR/sys" "KERNCONF=$KERNCONF" \
	# 	"DEBUG_FLAGS=-g" "-C$1" analyze
}

# Set FW_BUILD to point to naples_fw.tar
# Set FW_VERSION to actual firmware version
if [ -n "$FW_BUILD" ]; then
    cp $FW_BUILD/naples_fw.tar sys/modules/ionic_fw/
    make_ext sys/modules/ionic_fw
fi

make_ext sys/modules/ionic
make_ext sys/modules/ionic_rdma

# krping
make_ext krping

# Proceed building user space RDMA only if OFED was enabled
[ -e /usr/include/infiniband/verbs.h ] || exit 0
make_ext contrib/ofed/libionic

# Autotools is required for perftest and qperf
yes y | pkg install autotools || exit
aclocal --version || exit
autoconf --version || exit
automake --version || exit

cd perftest
./autogen.sh || exit
./configure || exit
make -j12 || exit
cd -

# In some network configurations, qperf will not work as a server until
# this setting is added to /etc/rc.conf:
# ipv6_ipv4mapping="YES"
#
cd qperf
./autogen.sh || exit
./configure || exit
make || exit
cd -
