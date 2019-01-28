#!/bin/sh

set -ex

TOPDIR=${TOPDIR:-/sw}
VERSION=$(grep buildroot $TOPDIR/minio/VERSIONS | awk '{print $NF}')

(cd $TOPDIR;
 rm -rf nic/buildroot/output;
 asset-pull buildroot $VERSION /dev/stdout | tar xvz) || exit -1

echo Fetched Buildroot binaries version $VERSION into $TOPDIR
