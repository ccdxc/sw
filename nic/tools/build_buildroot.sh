#!/bin/sh
#
#
# TLDR
# ------
# How to build a new buildroot and upload it to minio:
#
# 0a. Have a workspace
# 0b. Know the GIT HASH of the commit in buildroot tree you want to use
# 1. cd to `/sw/nic`
# 2. run `./tools/build_buildroot.sh <GIT_HASH_ON_BUILDROOT>`
#
# Long Version
# ------------
#
# The way we build the firmware now is by saving the buildroot
# binaries needed to assemble the final image to minio.
#
# We do that by adding an entry for buildroot in `/sw/minio/VERSIONS`
# and tagging the files we want to upload in
# `/sw/minio/buildroot.txt`
#
# The we run the `create-assets` script, that tars the files found in
# `buildroot.txt` and uploads the tar file to the minio server with a
# tag. Out of convention we are using the GIT HASH of the commit on
# buildroot we use.
#
# Example:
# ```
# $ cat /sw/minio/VERSIONS
# ...
# buildroot fa59708e2d3a25ad1e13992dde349c187b05abb4
# ...
#
# $ cat /sw/minio/buildroot.txt
# nic/buildroot/.config
# nic/buildroot/output/target/*
# nic/buildroot/output/images/Image
# nic/buildroot/output/images/kernel.img
# nic/buildroot/output/images/kernel.its
# nic/buildroot/output/images/u-boot.bin
# nic/buildroot/output/images/u-boot.met
# nic/buildroot/output/images/Image.gz
# nic/buildroot/output/images/kernel.itb
# nic/buildroot/output/images/kernel.met
# nic/buildroot/output/images/naples-100.dtb
# nic/buildroot/output/images/system.met
# nic/buildroot/output/images/u-boot.img
# nic/buildroot/output/host/bin/*
# nic/buildroot/output/host/doc/*
# nic/buildroot/output/host/etc/*
# nic/buildroot/output/host/include/*
# nic/buildroot/output/host/lib/*
# nic/buildroot/output/host/lib64/*
# nic/buildroot/output/host/libexec/*
# nic/buildroot/output/host/sbin/*
# nic/buildroot/output/host/share/*
# ```
#
# When the user runs `make firmware` we download these files,
# extract them to the original position, and assemble the final
# firmware. The makefile for this is: `/sw/nic/mkdefs/image.mk`
#
# To create a new version we have to do the following steps:
#
# 1. update the submodule `/sw/nic/buildroot` to the GITHASH we want to use
# 2. build the buildroot from inside the nic container
# 3. Update the `minio/VERSIONS` file with the same GITHASH
# 3b. Optionally update the `minio/buildroot.txt` file
# 4. Upload the new buildroot binaries to minio
#
#

set -e

[ $# -eq 1 ] || { echo "Usage: $0 <BUILDROOT_GIT_HASH>"; exit -1; }

pwd | grep -q "\/sw\/nic$" || { echo Please run from "sw/nic" directory; exit -1; }

BUILDROOT_HASH=$1
GIT_REMOTE_NAME="__upstream_buildroot"
TOPDIR=$(dirname `pwd`)
USERNAME=$(id -nu)

update_submodule() {
    cd $TOPDIR/nic/buildroot
    git remote add $GIT_REMOTE_NAME git@github.com:/pensando/buildroot
    git fetch $GIT_REMOTE_NAME
    git checkout $BUILDROOT_HASH
    return 0
}


start_shell() {
    cd $TOPDIR/nic
    make docker/background-shell | tail -n 1
}

cleanup() {
    # stop shell
    docker stop $DOCKER_ID
    # remove submodule remote
    (cd $TOPDIR/nic/buildroot && git remote remove $GIT_REMOTE_NAME)
}
trap cleanup EXIT INT TERM

echo "Creating buildroot remote $GIT_REMOTE_NAME"
update_submodule

echo "Starting docker instance"
DOCKER_ID=$(start_shell)
echo Docker_ID $DOCKER_ID

#sleeping to let pull-asset finish
sleep 90
 
docker_exec() { docker exec $DOCKER_ID sudo -i -u $USERNAME /bin/bash -c "${1}"; }
alias docker_root="docker exec $DOCKER_ID /bin/bash -c"

echo 'Replacing make and gmake'
docker_root "cp /opt/rh/devtoolset-7/root/bin/make /bin/make"
docker_root "cp /opt/rh/devtoolset-7/root/bin/make /bin/gmake"

echo 'Copying ssh keys into container'
cp -a ~/.ssh $TOPDIR/
docker_exec "cp -a /sw/.ssh ~/"

echo 'Cleaning old files'
docker_exec "rm -rf /sw/nic/buildroot/output"
docker_exec "rm -rf /sw/nic/buildroot/.config"

echo 'Generating config'
docker_exec "cd /sw/nic/buildroot && make capri_defconfig"

echo 'Building buildroot'
docker_exec "cd /sw/nic/buildroot && BUILDROOT_ASSET=1 make -j 24"

echo 'Preparing kernel headers for building external modules'
docker_exec "sh /sw/nic/tools/prepare_kernel_headers.sh"

echo 'Modifying VERSIONS'
sed -i "s/buildroot.*/buildroot ${BUILDROOT_HASH}/" $TOPDIR/minio/VERSIONS

echo 'Uploading new Buildroot binaries to minio'
docker_exec "cd /sw && UPLOAD=1 make create-assets"

echo 'Done. Please check your diffs, push and create a PR'
