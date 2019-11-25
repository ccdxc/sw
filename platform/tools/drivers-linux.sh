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
# tar xaf drivers-linux.tar.xz
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
: ${COMMON_SRC:="$TOP/platform/drivers/common"}
: ${RDMACORE_SRC:="$RDMA/rdma-core"}
: ${KRPING_SRC:="$RDMA/krping"}
: ${PERFTEST_SRC:="$RDMA/perftest"}
: ${QPERF_SRC:="$RDMA/qperf"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-linux"}
: ${COMMON_GEN_DIR:="$GEN_DIR/common/"}
: ${GEN_PKG:="$GEN_DIR.tar.xz"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

# Initialize gen dir with packaged scripts like build.sh
rsync -r --delete --delete-excluded \
  "$SCRIPTS_SRC/" "$GEN_DIR"

# Copy other useful scripts from elsewhere
rsync -r --delete --delete-excluded \
  "$TOP/nic/tools/rdmactl.py" \
  "$GEN_DIR/"

#copy print-cores.sh to package dir
rsync "$TOP/nic/tools/print-cores.sh" "$GEN_DIR/"

# Copy linux driver sources to gen dir
rsync -r --delete --delete-excluded --copy-links \
  --exclude=".git/" \
  --exclude=".cache.mk" \
  --exclude=".*.cmd" \
  --exclude="*.o" \
  --exclude="*.ko" \
  --exclude="*.mod.c" \
  --exclude="*.ur-safe" \
  --exclude=".*.swp" \
  --exclude="cscope.out" \
  --exclude="tags" \
  --exclude=".gitignore" \
  --exclude="modules.order" \
  --exclude="Module.symvers" \
  --exclude=".tmp_versions/" \
  --exclude="module.mk" \
  "$DRIVERS_SRC/" "$GEN_DIR/drivers"

rsync -r --copy-links \
  --exclude=".git/" \
  --exclude=".cache.mk" \
  --exclude=".*.cmd" \
  --exclude="*.o" \
  --exclude="*.ko" \
  --exclude="*.mod.c" \
  --exclude="*.ur-safe" \
  --exclude=".*.swp" \
  --exclude="cscope.out" \
  --exclude="tags" \
  --exclude=".gitignore" \
  --exclude="modules.order" \
  --exclude="Module.symvers" \
  --exclude=".tmp_versions/" \
  "$COMMON_SRC/" "$COMMON_GEN_DIR"

# Copy rdma-core sources to gen dir
# Use --links to preserve the link in rdma-core/pyverbs.
rsync -r --delete --delete-excluded --links \
  --exclude=".git/" \
  --exclude="build/" \
  "$RDMACORE_SRC/" "$GEN_DIR/rdma-core"
# Create relative symbolic links within the gen dir to driver sources
ln -s "../../drivers/rdma/lib/ionic" \
    "$GEN_DIR/rdma-core/providers/ionic"
ln -s "../../../drivers/rdma/drv/ionic/uapi/rdma/ionic-abi.h" \
    "$GEN_DIR/rdma-core/kernel-headers/rdma/ionic-abi.h"

# Copy krping sources to gen dir
rsync -r --delete --delete-excluded --exclude=".git/" --exclude="freebsd/" \
  "$KRPING_SRC/" "$GEN_DIR/krping"

# Copy perftest sources to gen dir
rsync -r --delete --delete-excluded --exclude=".git/" \
  "$PERFTEST_SRC/" "$GEN_DIR/perftest"

# Copy qperf sources to gen dir
rsync -r --delete --delete-excluded --exclude=".git/" \
  "$QPERF_SRC/" "$GEN_DIR/qperf"

# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi
sed -i "s/^\\(#define DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/" \
	"$GEN_DIR/drivers/eth/ionic/ionic.h"                           \
	"$GEN_DIR/drivers/rdma/drv/ionic/ionic_ibdev.c"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
