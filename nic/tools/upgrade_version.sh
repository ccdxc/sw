#! /bin/bash
TOPDIR="$GOPATH/src/github.com/pensando/sw/"
if [ -z $ARCH ]; then
    echo "Cannot create version file. ARCH not set."
    exit 1
fi

VERDIR="$TOPDIR/fake_root_target/${ARCH}/nic/upgrade/etc"
mkdir -p $VERDIR

VERFILE="${VERDIR}/version"

echo "$(cat $TOPDIR/nic/upgrade_manager/meta/upgrade_metadata.txt)" >> $VERFILE
