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
mkdir -p "$GEN_DIR/drivers"
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
  "$DRIVERS_SRC/eth/" "$GEN_DIR/drivers/eth"

cp "$DRIVERS_SRC/Makefile" "$GEN_DIR/drivers/"

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
  "$COMMON_SRC/" "$GEN_DIR/common"

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
