#!/bin/bash

#
# Generate drivers-linux.tar.xz from the workspace.  The package can be copied
# to a test host and unpacked for building the drivers and running rdma
# applications on the host.
#
# Synopsis:
#
# #Generate and copy the package
# platform/tools/drivers-linux.sh
# scp platform/gen/drivers-linux.tar.xz root@some-host:
#
# #Unpack and build on the host
# ssh root@some-host
# tar xaf drivers-linux.sh
# cd drivers-linux
# ./build.sh
#
# #Setup environment (PATH), load drivers:
# . env.sh
# insmod drivers/eth/ionic/ionic.ko
# modprobe ib_uverbs
# insmod drivers/rdma/drv/ionic/ionic_rdma.ko
#
# #Network setup script presumed to be installed separately:
# ~/net.sh up
# ping 1.2.3.4 #some-peer
#
# #Rdma applications in the package can be run from PATH
# ibv_devinfo   #same as rdma-core/build/bin/ibv_devinfo
# ib_send_bw    #same as perftest/ib_send_bw
#

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../..")
RDMA="$TOP/platform/src/third-party/rdma"

# Sources for generation
: ${SCRIPTS_SRC:="$TOP/platform/tools/drivers-linux"}
: ${DRIVERS_SRC:="$TOP/platform/drivers/linux"}
: ${RDMACORE_SRC:="$RDMA/rdma-core"}
: ${PERFTEST_SRC:="$RDMA/perftest"}
: ${QPERF_SRC:="$RDMA/qperf"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-linux"}
: ${GEN_PKG:="$GEN_DIR.tar.xz"}

# Package will identify version of sources
report_version() {
  cd "$1"
  echo -n 'HEAD: '
  git log --oneline -n1 HEAD
  echo -n '@{u}: '
  git log --oneline -n1 @{u}
  echo '### commits not upstream ###'
  git log --oneline @{u}..
  echo '### git status ###'
  git status -uno
  cd - > /dev/null
}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

# Initialize gen dir with packaged scripts like build.sh
rsync -r --delete --delete-excluded \
  "$SCRIPTS_SRC/" "$GEN_DIR"

# Copy linux driver sources to gen dir
report_version "$DRIVERS_SRC" > "$GEN_DIR/version.drivers"
rsync -r --delete --delete-excluded --copy-links \
  --exclude=".git/" \
  --exclude=".cache.mk" \
  --exclude=".*.cmd" \
  --exclude="*.o" \
  --exclude="*.ko" \
  --exclude="*.mod.c" \
  --exclude="modules.order" \
  --exclude="Module.symvers" \
  --exclude=".tmp_versions/" \
  "$DRIVERS_SRC/" "$GEN_DIR/drivers"

# Copy rdma-core sources to gen dir
report_version "$RDMACORE_SRC" > "$GEN_DIR/version.rdma-core"
rsync -r --delete --delete-excluded --copy-links \
  --exclude=".git/" \
  --exclude="build/" \
  --exclude="ionic/" \
  --exclude="ionic-abi.h" \
  "$RDMACORE_SRC/" "$GEN_DIR/rdma-core"
# Create relative symbolic links within the gen dir to driver sources
ln -s "../../drivers/rdma/lib/ionic" \
    "$GEN_DIR/rdma-core/providers/ionic"
ln -s "../../../drivers/rdma/drv/ionic/uapi/rdma/ionic-abi.h" \
    "$GEN_DIR/rdma-core/kernel-headers/rdma/ionic-abi.h"

# Copy perftest sources to gen dir
report_version "$PERFTEST_SRC" > "$GEN_DIR/version.perftest"
rsync -r --delete --delete-excluded \
  --exclude=".git/" \
  --exclude="*.a" \
  --exclude="*.o" \
  --exclude="ib_send_bw" \
  --exclude="ib_send_lat" \
  --exclude="ib_write_bw" \
  --exclude="ib_write_lat" \
  --exclude="ib_read_bw" \
  --exclude="ib_read_lat" \
  --exclude="ib_atomic_bw" \
  --exclude="ib_atomic_lat" \
  --exclude="raw_ethernet_burst_lat" \
  --exclude="raw_ethernet_bw" \
  --exclude="raw_ethernet_fs_rate" \
  --exclude="raw_ethernet_lat" \
  --exclude="src/.deps/" \
  --exclude="src/.dirstamp" \
  --exclude="libtool" \
  --exclude="stamp-h1" \
  --exclude="m4/" \
  "$PERFTEST_SRC/" "$GEN_DIR/perftest"

# Copy qperf sources to gen dir
report_version "$QPERF_SRC" > "$GEN_DIR/version.qperf"
rsync -r --delete --delete-excluded \
  --exclude=".git/" \
  --exclude="ChangeLog" \
  --exclude="Makefile" \
  --exclude="Makefile.in" \
  --exclude="NEWS" \
  --exclude="aclocal.m4" \
  --exclude="autom4te.cache/" \
  --exclude="compile" \
  --exclude="config.log" \
  --exclude="config.status" \
  --exclude="configure" \
  --exclude="depcomp" \
  --exclude="install-sh" \
  --exclude="missing" \
  --exclude="src/.deps/" \
  --exclude="src/Makefile" \
  --exclude="src/Makefile.in" \
  --exclude="src/help.c" \
  --exclude="*.o" \
  --exclude="src/qperf" \
  --exclude="src/qperf.1" \
  "$QPERF_SRC/" "$GEN_DIR/qperf"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
