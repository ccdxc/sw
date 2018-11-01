#! /bin/bash
TOPDIR="$GOPATH/src/github.com/pensando/sw/"
if [ -z $ARCH ]; then
    echo "Cannot create version file. ARCH not set."
    exit 1
fi

VERDIR="$TOPDIR/fake_root_target/${ARCH}/nic/etc"
mkdir -p $VERDIR

VERFILE="${VERDIR}/version"

version=`git describe --dirty --always`
buildtime=`date`

echo "Version       : $version" > $VERFILE 
echo "Build TIme    : $buildtime" >> $VERFILE
