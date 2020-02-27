#!/bin/sh

set -e

pwd | grep -q "\/sw\/nic$" || { echo Please run from "sw/nic" directory; exit -1; }

GIT_REMOTE_NAME="__upstream_buildroot"
TOPDIR=$(dirname `pwd`)
USERNAME=$(id -nu)
BUILDROOT_HASH=`grep buildroot $TOPDIR/minio/VERSIONS | cut -d' ' -f2`

#max diagimg size is 127*1024*1024 i.e. 60M
max_diag_img_sz=133169152

echo "BUILDROOT_HASH is: $BUILDROOT_HASH"

#update_submodule() {
#    cd $TOPDIR/nic/buildroot
#    git remote add $GIT_REMOTE_NAME git@github.com:/pensando/buildroot
#    git fetch $GIT_REMOTE_NAME
#    git checkout $BUILDROOT_HASH
#    return 0
#}

start_shell() {
    cd $TOPDIR/nic
    make docker/background-shell | tail -n 1
}

cleanup() {
    # stop shell
    docker stop $DOCKER_ID
}

if [ "x${JOB_ID}" = "x" ] || [ "x${IGNORE_BUILD_PIPELINE}" != "x" ]; then
    trap cleanup EXIT

    echo "Starting docker instance"
    DOCKER_ID=$(start_shell)
    echo Docker_ID $DOCKER_ID

    #sleeping to let pull-asset finish
    sleep 90

    alias docker_exec="docker exec $DOCKER_ID sudo -i -u $USERNAME /bin/bash -c"
    alias docker_root="docker exec $DOCKER_ID /bin/bash -c"
else
    alias docker_exec="/bin/bash -c"
    alias docker_root="/bin/bash -c"
fi

echo 'Replacing make and gmake'
docker_root "cp /bin/make /bin/make_default"
docker_root "cp /bin/gmake /bin/gmake_default"
docker_root "cp /opt/rh/devtoolset-7/root/bin/make /bin/make"
docker_root "cp /opt/rh/devtoolset-7/root/bin/make /bin/gmake"

if [ "x${JOB_ID}" = "x" ] || [ "x${IGNORE_BUILD_PIPELINE}" != "x" ]; then
    echo 'Copying ssh keys into container'
    cp -a ~/.ssh $TOPDIR/
    docker_exec "cp -a /sw/.ssh ~/"
else
    export FORCE_UNSAFE_CONFIGURE=1
fi

echo 'Cleaning old files'
docker_exec "rm -rf /sw/nic/buildroot/output_diag"

echo 'Generating diag config'
docker_exec "cd /sw/nic/buildroot && make capri_diagimg_defconfig O=output_diag"

echo 'Copying fwupdate'
docker_exec "cd /sw/nic && mkdir -p buildroot/output_diag/images && cp tools/fwupdate buildroot/output_diag/images/"

echo 'Building diag buildroot'
PLATFORM_LINUX_DIR=/sw/nic/buildroot/output/build/platform-linux/
docker_exec "rm -rf $PLATFORM_LINUX_DIR && mkdir -p $PLATFORM_LINUX_DIR && cd $PLATFORM_LINUX_DIR && tar -xf /sw/nic/buildroot/output/build/platform-linux.tar.gz"

#Build the buildroot with LINUX_OVERRIDE
docker_exec "cd /sw/nic/buildroot && PATH=$PATH:/tool/toolchain/aarch64-1.1/bin/ make -j 24 LINUX_OVERRIDE_SRCDIR=$PLATFORM_LINUX_DIR O=output_diag"

#Replace the make and gmake to default
docker_root "cp /bin/make_default /bin/make"
docker_root "cp /bin/gmake_default /bin/gmake"

echo 'Building Naples diag firmware'
docker_exec "cd /usr/src/github.com/pensando/sw/nic && PATH=$PATH:/tool/toolchain/aarch64-1.1/bin/ make ARCH=aarch64 PLATFORM=hw clean"
docker_exec "cd /usr/src/github.com/pensando/sw/ && PATH=$PATH:/tool/toolchain/aarch64-1.1/bin/ make ws-tools"
docker_exec "cd /usr/src/github.com/pensando/sw/nic && PATH=$PATH:/tool/toolchain/aarch64-1.1/bin/ make ARCH=aarch64 PLATFORM=hw diag-firmware"

image_sz=`stat -c %s $TOPDIR/nic/buildroot/output_diag/images/naples_diagfw.tar`

if [ $image_sz -gt $max_diag_img_sz ]; then
    echo "Error: DiagFW size($image_sz bytes) is more than allowed size($max_diag_img_sz bytes) for Naples"
    exit 1;
else
    echo "DiagFW is ready under sw/nic/buildroot/output_diag/images/naples_diagfw.tar. Diagfw Size: $image_sz bytes"
    echo 'Please check diagfw sanity before publishing it'
    #Publish the artifacts if RELEASE is non-zero
    if [ ! -z $RELEASE ]; then
        docker_exec "cd /usr/src/github.com/pensando/sw/asset-build/asset-push && go build"
        cd $TOPDIR
        asset-build/asset-push/asset-push builds hourly $RELEASE $TOPDIR/nic/buildroot/output_diag/images/naples_diagfw.tar
    fi

    exit 0
fi

