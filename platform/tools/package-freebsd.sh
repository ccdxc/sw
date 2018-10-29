#!/bin/bash

TOP=$(readlink -f "$(dirname "$0")/../..")

# Sources for generation
: ${DRIVERS_SRC:="$TOP/platform/drivers/freebsd/usr/src"}
: ${SONIC_DRIVERS_SRC:="$TOP/storage/offload"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/pensando-freebsd"}
: ${SONIC_GEN_DIR:="$TOP/platform/gen/pensando-freebsd/storage/"}
date=$( date +%Y%m%d_%H%M%S )
: ${GEN_PKG:=${GEN_DIR}-${date}.tar.xz}

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
rm -fr "$GEN_DIR"
mkdir -p "$GEN_DIR"
mkdir -p "$SONIC_GEN_DIR"

# Copy BSD driver sources to gen dir
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
  "$DRIVERS_SRC/" "$GEN_DIR/"

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

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
