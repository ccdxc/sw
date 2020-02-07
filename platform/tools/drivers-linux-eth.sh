#!/bin/bash

#
# Generate drivers-linux-eth.tar.xz from the workspace.
#
# Synopsis:
#
# #Generate and copy the package
# platform/tools/drivers-linux-eth.sh
# scp platform/gen/drivers-linux-eth.tar.xz root@some-host:
#
# #Unpack and build on the host
# ssh root@some-host
# tar xaf drivers-linux-eth.tar.xz
# cd drivers-linux-eth
# ./build.sh
#

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../..")

# Sources for generation
: ${SCRIPTS_SRC:="$TOP/platform/tools/drivers-linux-eth"}
: ${DRIVERS_SRC:="$TOP/platform/drivers/linux"}
: ${COMMON_SRC:="$TOP/platform/drivers/common"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-linux-eth"}
: ${GEN_PKG:="$GEN_DIR.tar.xz"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

# Initialize gen dir with packaged scripts like build.sh
rsync -r --delete --delete-excluded \
  "$SCRIPTS_SRC/" "$GEN_DIR"

#copy print-cores.sh to package dir
rsync "$TOP/nic/tools/print-cores.sh" "$GEN_DIR/"

# Copy linux driver sources to gen dir
mkdir -p "$GEN_DIR/drivers/eth/ionic"
cp "$DRIVERS_SRC/eth/ionic/ionic.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_api.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_api.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_bus.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_bus_pci.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_bus_platform.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_debugfs.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_debugfs.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_dev.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_dev.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_devlink.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_devlink.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_ethtool.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_ethtool.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_lif.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_lif.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_main.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_rx_filter.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_rx_filter.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_stats.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_stats.h" \
   "$DRIVERS_SRC/eth/ionic/ionic_txrx.c" \
   "$DRIVERS_SRC/eth/ionic/ionic_txrx.h" \
   "$DRIVERS_SRC/eth/ionic/kcompat.c" \
   "$DRIVERS_SRC/eth/ionic/kcompat.h" \
   "$DRIVERS_SRC/eth/ionic/kcompat_overflow.h" \
   "$DRIVERS_SRC/eth/ionic/Kconfig" \
   "$DRIVERS_SRC/eth/ionic/Makefile" \
   "$DRIVERS_SRC/eth/ionic/README" \
   "$GEN_DIR/drivers/eth/ionic"

cp "$DRIVERS_SRC/Makefile" "$GEN_DIR/drivers/"

mkdir -p "$GEN_DIR/common"
cp "$COMMON_SRC/ionic_if.h" "$GEN_DIR/common"
cp "$COMMON_SRC/ionic_regs.h" "$GEN_DIR/common"

# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi
echo "Setting Linux driver version to '$VER'"
sed -i "s/^\\(#define IONIC_DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/" \
	"$GEN_DIR/drivers/eth/ionic/ionic.h"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
