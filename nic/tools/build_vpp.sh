#!/bin/sh
#
#
# Usage
# ------
# How to build a new VPP and upload it to minio:
#
# 0a. Have a workspace
# 0b. Know the GIT HASH of the commit in pensando/vpp tree you want to use
# 1. cd to `/sw/nic`
# 2. run `./tools/build_vpp.sh -v <GIT_REPO_PATH:GIT_HASH_OF_VPP> -d <GIT_REPO_PATH:GIT_HASH_OF_DPDK>`
#    Note that arguemnts are optional, default is pensando/vpp and pensando/dpdk - pds-master tot.
#

#Uncomment for debugging script
#set -e

usage() {
    echo "Usage: $0 [OPTION]"
    echo "Options available:"
    echo "    -v <GIT_REPO_PATH:GIT_HASH_OF_VPP>   Default is git@github.com:pensando/vpp, pds-master tot commit hash"
    echo "    -d <GIT_REPO_PATH:GIT_HASH_OF_DPDK>  Default is git@github.com:pensando/dpdk, pds-master tot commit hash"
    echo "    -a <third_party_libs minio version>  minio VERSION to use for asset-upload"
    echo "    -s                                   Skip VPP/DPDK repo download"
    exit 1;
}

start_shell() {
    cd $TOPDIR/nic
    make docker/background-shell | tail -n 1
}

cleanup() {
    # stop shell
    [ ! -z "$DOCKER_ID" ] && echo "Stopping docker container" && docker stop $DOCKER_ID
}

get_repo() {
    [ ! -z $SKIP_GET_REPO ] && echo "Skipping VPP/DPDK download!" && return
    echo "Download VPP/DPDK Repo"
    cd $TOPDIR/nic/sdk/third-party
    git clone $VPP_REPO
    if [[ $? -ne 0 ]]; then
        echo "VPP repo download failed"
        exit 1;
    fi
    if [ ! -z "$VPP_COMMIT" ]; then
        cd $TOPDIR/nic/sdk/third-party/vpp && git checkout $VPP_COMMIT
        if [[ $? -ne 0 ]]; then
            echo "git checkout failed for VPP"
            exit 1;
        fi
    fi

    cd $TOPDIR/nic/sdk/dpdk
    git fetch $DPDK_REPO
    if [[ $? -ne 0 ]]; then
        echo "DPDK repo download failed"
        exit 1;
    fi
    if [ ! -z "$DPDK_COMMIT" ]; then
        git checkout $DPDK_COMMIT
    else
        git checkout pds-master
        git pull
    fi
    if [[ $? -ne 0 ]]; then
        echo "git checkout failed for DPDK"
        exit 1;
    fi
}

pwd | grep -q "\/sw\/nic$" || { echo Please run from "sw/nic" directory; exit -1; }

VPP_REPO="git@github.com:pensando/vpp"
DPDK_REPO="git@github.com:pensando/dpdk"

while getopts 'v:d:a:sh' opt
do
    case $opt in
        v)
            VPP_REPO=${OPTARG%:*}
            VPP_COMMIT=${OPTARG##*:}
            ;;
        d)
            DPDK_REPO=${OPTARG%:*}
            DPDK_COMMIT=${OPTARG##*:}
            ;;
        a)
            VPP_ASSET_VERSION=${OPTARG}
            ;;
        s)
            SKIP_GET_REPO=1
            ;;
        h | *)
            usage
            ;;
    esac
done

TOPDIR=$(dirname `pwd`)
DOCKER_TOPDIR=/sw
NUMAROOT=$DOCKER_TOPDIR/nic/sdk/third-party/libnuma
TOOLCHAIN_DIR=/tool/toolchain/aarch64-1.1
TOOLCHAIN_PREFIX=$TOOLCHAIN_DIR/bin/aarch64-linux-gnu
USERNAME=$(id -nu)

echo "Input variables:"
printf "%-25s = %-40s\n" "VPP_REPO" "$VPP_REPO"
printf "%-25s = %-40s\n" "DPDK_REPO" "$DPDK_REPO"
if [ -z "$VPP_COMMIT" ]; then
    printf "%-25s = %-40s\n" "VPP_COMMIT" "Not specified, using tot"
else
    printf "%-25s = %-40s\n" "VPP_COMMIT" "$VPP_COMMIT"
fi
if [ -z "$DPDK_COMMIT" ]; then
    printf "%-25s = %-40s\n" "DPDK_COMMIT" "Not specified, using tot"
else
    printf "%-25s = %-40s\n" "DPDK_COMMIT" "$DPDK_COMMIT"
fi
if [ -z "$VPP_ASSET_VERSION" ]; then
    printf "%-25s = %-40s\n\n" "VPP_ASSET_VERSION" "Not specified, skip upload"
else
    printf "%-25s = %-40s\n\n" "VPP_ASSET_VERSION" "$VPP_ASSET_VERSION"
fi

trap clean_vpp_build EXIT

get_repo

echo "Starting docker instance"
DOCKER_ID=$(start_shell)
echo Docker_ID $DOCKER_ID

#sleeping to let pull-asset finish
#sleep 90

alias docker_exec="docker exec $DOCKER_ID sudo -i -u $USERNAME /bin/bash -c"
alias docker_root="docker exec $DOCKER_ID /bin/bash -c"

clean_vpp_build() {
    echo "Clean VPP build"
    [ ! -z "$DOCKER_ID" ] && docker_exec "cd $DOCKER_TOPDIR/nic/sdk/third-party/vpp && mv module.mk module.mk_bak && mv module_export.mk module_export.mk_bak"
    #echo "Delete VPP repo"
    #docker_exec "rm -rf $DOCKER_TOPDIR/nic/sdk/third-party/vpp"
    cleanup
}

copy_assets() {
    echo "Copying Assets"
    docker_exec "command cp -f -L $DOCKER_TOPDIR/nic/sdk/third-party/vpp/build-root/pensando/lib/dpdk_plugin.so $DOCKER_TOPDIR/nic/sdk/third-party/vpp-pkg/aarch64/lib"
    if [[ $? -ne 0 ]]; then
        echo "DPDK plugin copy failed"
        exit 1;
    fi
    docker_exec "command cp -f -L $DOCKER_TOPDIR/nic/sdk/third-party/vpp/build-root/pensando/lib/lib*.so.* $DOCKER_TOPDIR/nic/sdk/third-party/vpp-pkg/aarch64/lib"
    if [[ $? -ne 0 ]]; then
        echo "VPP libs copy failed"
        exit 1;
    fi
    docker_exec "command cp -f -L $DOCKER_TOPDIR/nic/sdk/third-party/vpp/build-root/pensando/bin/vpp $DOCKER_TOPDIR/nic/sdk/third-party/vpp-pkg/aarch64/bin"
    if [[ $? -ne 0 ]]; then
        echo "VPP bins copy failed"
        exit 1;
    fi
    docker_exec "command cp -f -L -r $DOCKER_TOPDIR/nic/sdk/third-party/vpp/build-root/pensando/include/* $DOCKER_TOPDIR/nic/sdk/third-party/vpp-pkg/include/"
    if [[ $? -ne 0 ]]; then
        echo "VPP headers copy failed"
        exit 1;
    fi
    echo "Successfully updated assets"
}

build_dpdk() {
    echo "Building DPDK, check $TOPDIR/nic/sdk/dpdk/dpdk_build.log"
    docker_exec "rm -rf $DOCKER_TOPDIR/nic/sdk/dpdk/build"
    docker_exec "cd $DOCKER_TOPDIR/nic/sdk/dpdk && make -j8 -f REPOmakefile config T=arm64-armv8a-linuxapp-gcc && make -f REPOmakefile CROSS=${TOOLCHAIN_PREFIX}- CONFIG_RTE_KNI_KMOD=n CONFIG_RTE_EAL_IGB_UIO=n EXTRA_CFLAGS=\"-isystem  ${NUMAROOT}/include -fPIC\" EXTRA_LDFLAGS=\"-L${NUMAROOT}/aarch64/lib -lnuma\" &> dpdk_build.log"
    if [[ $? -ne 0 ]]; then
        echo "DPDK build Failed"
        exit 1;
    fi
    echo "DPDK build success"
}

build_vpp() {
    build_dpdk
    echo "Building VPP, check $TOPDIR/nic/sdk/third-party/vpp/vpp_build.log"
    docker_root "cd $DOCKER_TOPDIR/nic/sdk/third-party/vpp && make -f Makefile ARCH=aarch64 PLATFORM=hw RELEASE=1 SDKDIR=$DOCKER_TOPDIR/nic/sdk all &> vpp_build.log"
    if [[ $? -ne 0 ]]; then
        echo "VPP build Failed"
        exit 1;
    fi
    echo "VPP build success"
}

upload_assets() {
    copy_assets
    sed -i "s/third_party_libs.*/third_party_libs ${VPP_ASSET_VERSION}/" $TOPDIR/minio/VERSIONS
    if [[ $? -ne 0 ]]; then
        echo "Asset version change failed"
        exit 1;
    fi
    echo "Uploading assets with version ${VPP_ASSET_VERSION}"
    docker_exec "cd $DOCKER_TOPDIR && UPLOAD=1 make create-assets"
    if [[ $? -ne 0 ]]; then
        echo "Upload-assets failed for version ${VPP_ASSET_VERSION}"
        exit 1;
    fi
    echo "Uploaded assets with version ${VPP_ASSET_VERSION}"
}

build_vpp

if [ -z "$VPP_ASSET_VERSION" ]; then
    copy_assets
    echo 'Updated assets but skipping upload'
else
    upload_assets
    echo 'Done. Please check your diffs, push and create a PR'
fi

#clean_vpp_build
