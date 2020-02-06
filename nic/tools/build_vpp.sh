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
    return
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
SDK_DIR=${TOPDIR}/nic/sdk
DPDK_DIR=${SDK_DIR}/dpdk
VPP_DIR=${SDK_DIR}/third-party/vpp
DOCKER_TOPDIR=/usr/src/github.com/pensando/sw
DOCKER_SDK_DIR=${DOCKER_TOPDIR}/nic/sdk
DOCKER_DPDK_DIR=${DOCKER_SDK_DIR}/dpdk
DOCKER_VPP_DIR=${DOCKER_SDK_DIR}/third-party/vpp
DOCKER_VPP_PKG_DIR=${DOCKER_SDK_DIR}/third-party/vpp-pkg
DOCKER_NUMAROOT=${DOCKER_SDK_DIR}/third-party/libnuma
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
    [ ! -z "$DOCKER_ID" ] && docker_exec "cd $DOCKER_VPP_DIR && mv module.mk module.mk_bak && mv module_export.mk module_export.mk_bak"
    #echo "Delete VPP repo"
    #docker_exec "rm -rf $DOCKER_TOPDIR/nic/sdk/third-party/vpp"
    cleanup
}

copy_assets() {
    echo "Copying Assets"
    DOCKER_VPP_BUILD_ROOT=${DOCKER_VPP_DIR}/build-root
    DOCKER_VPP_BUILD_AARCH64=${DOCKER_VPP_BUILD_ROOT}/install-naples-aarch64/vpp
    DOCKER_VPP_BUILD_X86_64=${DOCKER_VPP_BUILD_ROOT}/install-vpp_debug-x86_64/vpp

    docker_exec "rm -rf ${DOCKER_VPP_PKG_DIR}/aarch64 ${DOCKER_VPP_PKG_DIR}/x86_64 ${DOCKER_VPP_PKG_DIR}/include ${DOCKER_VPP_PKG_DIR}/share"
    docker_exec "mkdir -p ${DOCKER_VPP_PKG_DIR}/aarch64/lib ${DOCKER_VPP_PKG_DIR}/aarch64/bin"
    docker_exec "mkdir -p ${DOCKER_VPP_PKG_DIR}/x86_64/lib ${DOCKER_VPP_PKG_DIR}/x86_64/bin"
    docker_exec "mkdir -p ${DOCKER_VPP_PKG_DIR}/include ${DOCKER_VPP_PKG_DIR}/share"

    docker_exec "command cp -f -L -r ${DOCKER_VPP_BUILD_AARCH64}/include/* $DOCKER_VPP_PKG_DIR/include/"
    if [[ $? -ne 0 ]]; then
        echo "VPP headers copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L -r ${DOCKER_VPP_BUILD_AARCH64}/share/* $DOCKER_VPP_PKG_DIR/share/"
    if [[ $? -ne 0 ]]; then
        echo "VPP share copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L ${DOCKER_VPP_BUILD_AARCH64}/lib/lib*.so.* ${DOCKER_VPP_PKG_DIR}/aarch64/lib/"
    if [[ $? -ne 0 ]]; then
        echo "VPP aarch64 libs copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L -r ${DOCKER_VPP_BUILD_AARCH64}/lib/vpp_plugins ${DOCKER_VPP_PKG_DIR}/aarch64/lib/"
    if [[ $? -ne 0 ]]; then
        echo "VPP aarch64 plugins copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L ${DOCKER_VPP_BUILD_AARCH64}/bin/* ${DOCKER_VPP_PKG_DIR}/aarch64/bin/"
    if [[ $? -ne 0 ]]; then
        echo "VPP aarch64 bin copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L ${DOCKER_VPP_BUILD_X86_64}/lib/lib*.so.* ${DOCKER_VPP_PKG_DIR}/x86_64/lib/"
    if [[ $? -ne 0 ]]; then
        echo "VPP x86_64 libs copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L -r ${DOCKER_VPP_BUILD_X86_64}/lib/vpp_plugins ${DOCKER_VPP_PKG_DIR}/x86_64/lib/"
    if [[ $? -ne 0 ]]; then
        echo "VPP x86_64 plugins copy failed"
        exit 1;
    fi

    docker_exec "command cp -f -L ${DOCKER_VPP_BUILD_X86_64}/bin/* ${DOCKER_VPP_PKG_DIR}/x86_64/bin/"
    if [[ $? -ne 0 ]]; then
        echo "VPP x86_64 bin copy failed"
        exit 1;
    fi

    echo "Successfully updated assets"
}

build_dpdk_aarch64() {
    echo "Building DPDK - aarch64, check $DPDK_DIR/dpdk_build_aarch64.log"
    docker_exec "rm -rf $DOCKER_DPDK_DIR/build"
    docker_exec "cd $DOCKER_DPDK_DIR && make -j8 -f REPOmakefile config T=arm64-armv8a-linuxapp-gcc && make -j8 V=1 -f REPOmakefile CROSS=${TOOLCHAIN_PREFIX}- CONFIG_RTE_KNI_KMOD=n CONFIG_RTE_EAL_IGB_UIO=n EXTRA_CFLAGS=\"-isystem ${DOCKER_NUMAROOT}/include -fPIC\" EXTRA_LDFLAGS=\"-L${DOCKER_NUMAROOT}/aarch64/lib -lnuma\" &> dpdk_build_aarch64.log"
    if [[ $? -ne 0 ]]; then
        echo "DPDK - aarch64 build Failed"
        exit 1;
    fi
    echo "DPDK - aarch64 build success"
}

build_vpp_aarch64() {
    build_dpdk_aarch64
    echo "Building VPP - aarch64, check $VPP_DIR/vpp_build_aarch64.log"
    docker_root "cd $DOCKER_VPP_DIR && make -f Makefile ARCH=aarch64 PLATFORM=hw RELEASE=1 SDKDIR=$DOCKER_SDK_DIR wipe_vpp &> vpp_clean_aarch64.log"
    docker_root "cd $DOCKER_VPP_DIR && make -f Makefile ARCH=aarch64 PLATFORM=hw RELEASE=1 SDKDIR=$DOCKER_SDK_DIR all &> vpp_build_aarch64.log"
    if [[ $? -ne 0 ]]; then
        echo "VPP - aarch64 build Failed"
        exit 1;
    fi
    echo "VPP - aarch64 build success"
}

build_sdk_x86_64() {
    echo "Building SDK - x86_64, check $SDK_DIR/sdk_build_x86_64.log"
    docker_root "mkdir -p /sdk && mount --bind ${DOCKER_SDK_DIR} /sdk && PLATFORM=sim make -C /sdk &> ${DOCKER_SDK_DIR}/sdk_build_x86_64.log"
    if [[ $? -ne 0 ]]; then
        echo "SDK - x86_64 build Failed, continue.."
        exit 1;
    else
        echo "SDK - x86_64 build success."
    fi
}

build_dpdk_x86_64() {
    docker_exec "rm -rf $DOCKER_DPDK_DIR/build"
    build_sdk_x86_64
    echo "Building DPDK - x86_64, check $DPDK_DIR/dpdk_build_x86_64.log"
    docker_exec "cd $DOCKER_DPDK_DIR && make -j8 -f REPOmakefile config T=x86_64-default-linuxapp-gcc && make V=1 -j8 -f REPOmakefile CONFIG_RTE_KNI_KMOD=n CONFIG_RTE_EAL_IGB_UIO=n EXTRA_CFLAGS=\"-I ${DOCKER_NUMAROOT}/include -I ${DOCKER_SDK_DIR} -DDPDK_SIM -g -fPIC -O0 -Wno-error -L ${DOCKER_NUMAROOT}/x86_64/lib\" EXTRA_LDFLAGS=\"-L${DOCKER_NUMAROOT}/x86_64/lib -L/sdk/build/x86_64/lib/ -lnuma -ldpdksim -lsdkpal -llogger\" &> dpdk_build_x86_64.log"
    if [[ $? -ne 0 ]]; then
        echo "DPDK - x86_64 build Failed"
        exit 1;
    fi
    echo "DPDK - x86_64 build success"
}

build_vpp_x86_64() {
    build_dpdk_x86_64
    echo "Building VPP - x86_64, check $VPP_DIR/vpp_build_x86_64.log"
    docker_root "cd $DOCKER_VPP_DIR && make -f Makefile ARCH=x86_64 SDKDIR=$DOCKER_SDK_DIR wipe_vpp &> vpp_clean_x86_64.log"
    docker_root "cd $DOCKER_VPP_DIR && make V=1 -f Makefile ARCH=x86_64 SDKDIR=$DOCKER_SDK_DIR all &> vpp_build_x86_64.log"
    if [[ $? -ne 0 ]]; then
        echo "VPP - x86_64 build Failed"
        exit 1;
    fi
    echo "VPP - x86_64 build success"
}

build_vpp() {
    [ ! -z "$DOCKER_ID" ] && docker_exec "cd $DOCKER_VPP_DIR && mv module.mk module.mk_bak && mv module_export.mk module_export.mk_bak"
    build_vpp_aarch64
    build_vpp_x86_64
}

upload_assets() {
    copy_assets
    sed -i "s/third_party_libs.*/third_party_libs ${VPP_ASSET_VERSION}/" $TOPDIR/minio/VERSIONS
    if [[ $? -ne 0 ]]; then
        echo "Asset version change failed"
        exit 1;
    fi
    echo "Uploading assets with version ${VPP_ASSET_VERSION}"
    docker_exec "cd $DOCKER_TOPDIR && UPLOAD=1 make create-assets &> $DOCKER_TOPDIR/asset_upload.txt"
    if [[ $? -ne 0 ]]; then
        echo "Upload-assets failed for version ${VPP_ASSET_VERSION}, check $TOPDIR/asset_upload.txt"
        exit 1;
    fi
    echo "Uploaded assets with version ${VPP_ASSET_VERSION}, verify $TOPDIR/asset_upload.txt"
}

docker_root "echo \"ip_resolve=IPv4\" >> /etc/yum.conf"
docker_root "rm ${DOCKER_VPP_DIR}/build-root/.deps_installed"
build_vpp

if [ -z "$VPP_ASSET_VERSION" ]; then
    copy_assets
    echo 'Updated assets but skipping upload'
else
#    upload_assets
#    echo 'Done. Please check your diffs, push and create a PR'
    copy_assets
    echo 'Uploading assets disabled with this script, please upload manually!'
fi

#clean_vpp_build
