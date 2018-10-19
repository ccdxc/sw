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
  --exclude="tools/" \
  "$DRIVERS_SRC/" "$GEN_DIR"
report_version "$DRIVERS_SRC" > "$GEN_DIR/version.drivers"

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
