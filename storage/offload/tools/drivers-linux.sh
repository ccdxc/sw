#!/bin/bash

#
# Generate storage-offload.tar from the workspace.  The package can be copied
# to a test host and unpacked for building the drivers and running offload
# applications on the host.
#
# Synopsis:
#
# #Generate and copy the package
# storage/offload/drivers-linux.sh
# scp storage/offload/gen/storage-offload.tar root@some-host:
#
# #Unpack and build on the host
# ssh root@some-host
# tar xaf storage-offload.tar
# cd storage-offload
# make modules
#
# #Setup environment (PATH), load drivers:
# . env.sh
# insmod storage/offload/sonic.ko
# insmod storage/offload/pencake.ko
#
#

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../../..")

# Sources for generation
: ${DRIVERS_SRC:="$TOP/storage/offload"}

# Products generated
: ${GEN_DIR:="$TOP/storage/gen/storage-offload"}
: ${GEN_PKG:="$GEN_DIR.tar.xz"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

# Copy linux driver sources to gen dir
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
  --exclude="cscope.*" \
  --exclude="tags" \
  --exclude="tools/" \
  "$DRIVERS_SRC/" "$GEN_DIR"

# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi
sed -i "s/^\\(#define DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/" \
	"$GEN_DIR/src/drv/linux/kernel/sonic.h"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
