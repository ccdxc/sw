#!/bin/bash

TOP=$(readlink -f "$(dirname "$0")/../..")
RDMA="$TOP/platform/src/third-party/rdma"

# Sources for generation
: ${DRIVERS_SRC:="$TOP/platform/drivers/freebsd/usr/src"}
: ${COMMON_SRC:="$TOP/platform/drivers/common"}
: ${SONIC_DRIVERS_SRC:="$TOP/storage/offload"}
: ${PERFTEST_SRC:="$RDMA/perftest"}
: ${QPERF_SRC:="$RDMA/qperf"}
: ${KRPING_SRC:="$RDMA/krping/freebsd/krping"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-freebsd"}
: ${GEN_ETH_DIR:="$TOP/platform/gen/drivers-freebsd-eth"}
: ${COMMON_GEN_DIR:="$GEN_DIR/sys/dev/ionic/ionic_eth/"}
: ${SONIC_GEN_DIR:="$GEN_DIR/storage/"}
: ${GEN_PKG:=${GEN_DIR}.tar.xz}
: ${GEN_ETH_PKG:=${GEN_ETH_DIR}.tar.xz}

# Always start clean
rm -fr "$GEN_DIR"
mkdir -p "$GEN_DIR"
mkdir -p "$SONIC_GEN_DIR"

# Copy BSD driver sources to gen dir
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
  "$DRIVERS_SRC/" "$GEN_DIR/"

rsync -r  \
  --exclude=".git/" \
  --exclude=".cache.mk" \
  --exclude=".*.cmd" \
  --exclude="*.o" \
  --exclude="*.ko" \
  --exclude="*.mod.c" \
  --exclude="modules.order" \
  --exclude="Module.symvers" \
  --exclude=".tmp_versions/" \
  "$COMMON_SRC/" "$COMMON_GEN_DIR/"

# Copy sonic driver
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
  "$SONIC_DRIVERS_SRC/" "$SONIC_GEN_DIR/"

# Copy other useful scripts from elsewhere
rsync -r --delete --delete-excluded \
  "$TOP/nic/tools/rdmactl.py" \
  "$GEN_DIR/"

#copy print-cores.sh to package dir
rsync "$TOP/nic/tools/print-cores.sh" "$GEN_DIR/"

# Copy perftest sources to gen dir
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

# Copy krping sources to gen dir
rsync -r --delete --delete-excluded \
  --exclude=".git/" \
  --exclude="*.o" \
  --exclude="*.ko" \
  "$KRPING_SRC/" "$GEN_DIR/krping"

# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi
sed -i "s/^\\(#define DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/" \
	"$GEN_DIR/sys/dev/ionic/ionic_rdma/ionic_ibdev.c"      \
	"$GEN_DIR/sys/dev/ionic/ionic_eth/ionic.h"             \
	"$SONIC_GEN_DIR/src/drv/linux/kernel/sonic.h"

if [ -n "$FW_PACKAGE" ]; then
  cp $TOP/nic/naples_fw.tar $GEN_DIR/sys/modules/ionic_fw/
fi

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
#TODO: Freebsd ETH driver should just have ethernet sources
mkdir -p "$GEN_ETH_DIR"
cp -r $GEN_DIR/* "$GEN_ETH_DIR"
cd "$GEN_ETH_DIR/.."
tar -cJ --exclude=.git -f "$GEN_ETH_PKG" "$(basename "$GEN_ETH_DIR")"
