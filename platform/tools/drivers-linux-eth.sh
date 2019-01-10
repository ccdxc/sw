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

# Package will identify version of sources
report_version() {
  cd "$1"
  if ! git status &> /dev/null ; then
    echo 'Unable to identify version from git'
  else
    echo -n 'HEAD: '
    git log --oneline -n1 HEAD
    echo -n '@{u}: '
    git log --oneline -n1 @{u}
    echo '### commits not upstream ###'
    git log --oneline @{u}..
    echo '### git status ###'
    git status -uno
    cd - > /dev/null
  fi
}

# Always start clean
rm -rf "$GEN_DIR"
mkdir -p "$GEN_DIR"

# Initialize gen dir with packaged scripts like build.sh
rsync -r --delete --delete-excluded \
  "$SCRIPTS_SRC/" "$GEN_DIR"

# Copy linux driver sources to gen dir
mkdir -p "$GEN_DIR/drivers"
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
  "$DRIVERS_SRC/eth/" "$GEN_DIR/drivers/eth"

rsync -r --copy-links \
  --exclude=".git/" \
  --exclude=".cache.mk" \
  --exclude=".*.cmd" \
  --exclude="*.o" \
  --exclude="*.ko" \
  --exclude="*.mod.c" \
  --exclude="modules.order" \
  --exclude="Module.symvers" \
  --exclude=".tmp_versions/" \
  "$COMMON_SRC/" "$GEN_DIR/common"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
