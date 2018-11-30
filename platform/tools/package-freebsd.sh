#!/bin/bash

TOP=$(readlink -f "$(dirname "$0")/../..")
RDMA="$TOP/platform/src/third-party/rdma"

# Sources for generation
: ${DRIVERS_SRC:="$TOP/platform/drivers/freebsd/usr/src"}
: ${COMMON_SRC:="$TOP/platform/drivers/common"}
: ${SONIC_DRIVERS_SRC:="$TOP/storage/offload"}
: ${PERFTEST_SRC:="$RDMA/perftest"}

# Products generated
: ${GEN_DIR:="$TOP/platform/gen/drivers-freebsd"}
: ${COMMON_GEN_DIR:="$GEN_DIR/sys/dev/ionic/ionic_eth/"}
: ${SONIC_GEN_DIR:="$GEN_DIR/storage/"}
: ${GEN_PKG:=${GEN_DIR}.tar.xz}

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

# Generate tarball of the prepared package
cd "$GEN_DIR/.."
tar -cJ --exclude=.git -f "$GEN_PKG" "$(basename "$GEN_DIR")"
