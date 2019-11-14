
#!/bin/bash

#
# Generate drivers-esx.tar.xz from the workspace.  The package can be used to do vib install on ESX
#
# Synopsis:
#
# #Generate and copy the package
# platform/tools/drivers-esx.sh
# scp platform/gen/drivers-esx.tar.xz root@some-host:
#
# #Unpack and build on the host
# ssh root@some-host
# tar xaf drivers-esx.tar.xz
# cd drivers-esx
# ./build.sh
#

set -x

# Where am I
TOP=$(readlink -f "$(dirname "$0")/../..")

# Sources for generation
: ${SCRIPTS_SRC:="$TOP/platform/tools/drivers-esx"}
: ${VIB_GEN:="$TOP/platform/tools/gen_esx_vib.py"}
: ${VIB:="$TOP/platform/drivers/esxi/vib/"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-esx-eth"}
#This has to be updated once ESX version is built
: ${DRIVERS_SRC:="$TOP/platform/drivers/"}
: ${ESXI_VIB_SRC:="$TOP/platform/drivers/"}
: ${COMMON_GEN_DIR:="$GEN_DIR/common/"}
: ${GEN_PKG:="$GEN_DIR.tar.xz"}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"


# Initialize gen dir with packaged scripts like build.sh
rsync -r --delete --delete-excluded \
  "$SCRIPTS_SRC/" "$GEN_DIR"

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
  --exclude="freebsd/" \
  --exclude="linux/" \
  --exclude="pxe/" \
  "$ESXI_VIB_SRC/" "$GEN_DIR/drivers"
  
# Set version string
if [ -n "$SW_VERSION" ] ; then
	VER=$SW_VERSION
else
	VER=`git describe --tags`
fi
# sed -i "s/^\\(#define DR\\w*_VER\\w*\\s\\+\"\\).*\\(\"\\)\$/\1$VER\2/" \
#	"$GEN_DIR/drivers/esxi/ionic_en/ionic.h"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"

#Finally generate vib too
"$VIB_GEN" --drivers-pkg $GEN_PKG
