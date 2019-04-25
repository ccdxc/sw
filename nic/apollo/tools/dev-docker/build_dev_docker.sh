#!/bin/sh

# Checkout the repository and submodules
# Do pull-assets 
# Goto docker container, and execute below from <sw> directory.

DST='/sw/apollo_sw'
LIBDIR=$DST/nic/sdk/third-party/libs
agent=$1
set -x
copy_files() {
    # Add a space in end of each line.
    nicd='nic/sdk nic/buildroot nic/mkdefs nic/tools/ncc nic/utils/pack_bytes '
    nicf='nic/Makefile nic/include/capri_barco.h nic/tools/print-cores.sh nic/tools/savelogs.sh '
    nicf+='nic/tools/merge_model_debug.py nic/include/globals.hpp '
    nicf+='nic/include/notify.hpp nic/include/edma.hpp nic/include/eth_common.h '
    nicf+='nic/include/adminq.h nic/include/nvme_dev_if.h '

    p4d='nic/p4/include nic/p4/common nic/p4/common-p4+ nic/asm/common-p4+/include/ nic/p4-hlir ' 
    p4d+='nic/include/hal_pd_error.hpp nic/p4/eth nic/asm/eth '
    p4d+='nic/p4/adminq nic/p4/edma nic/p4/notify nic/asm/adminq nic/asm/edma nic/asm/notify '

    pkgf='nic/tools/package/package.py nic/tools/package/pack_host.txt nic/tools/package/pack_apollo.txt '
    pkgf+='nic/tools/update_version.sh nic/tools/core_count_check.sh nic/tools/package/pack_platform.txt '
    pkgf+='nic/tools/package/pack_debug.txt nic/tools/upgrade_version.sh nic/tools/gen_version.py '

    apollod='nic/apollo nic/conf/apollo '

    pack_apollo='nic/conf/init_bins nic/conf/catalog_hw.json nic/conf/serdes.json '
    pack_apollo+='nic/tools/sysupdate.sh nic/tools/apollo nic/tools/sysreset.sh nic/tools/fwupdate '
    pack_apollo+='nic/conf/captrace platform/src/app/pciemgrd nic/hal/third-party/spdlog/include/ ' 
    pack_apollo+='platform/src/app/memtun nic/hal/third-party/judy '
    pack_apollo+='platform/src/lib/pciemgr_if platform/drivers  platform/src/lib/nicmgr '
    pack_apollo+='platform/src/lib/rdmamgr_apollo '

    protobuf=" "
    if [ $agent == 1 ];then
        protobuf+='vendor/github.com vendor/golang.org vendor/google.golang.org vendor/gopkg.in '
        protobuf+='venice/utils/log venice/utils/testenv '
        protobuf+='nic/hal/third-party/google nic/proto/hal/module_gogo.mk nic/hal/third-party/grpc '
        protobuf+='nic/third-party/gflags/include nic/hal/third-party/openssl/include '
        protobuf+='nic/third-party/libz/include nic/third-party/liblmdb/include '
        mkdir -p $DST/proto_bin
        cp -r /usr/bin/proto* $DST/proto_bin
        cp -r /usr/local/go $DST/
    fi

    miscd='nic/upgrade_manager/meta nic/utils/trace nic/utils/ftl '

    pack_debug='nic/debug_cli nic/tools/p4ctl '

    files="$nicd $nicf $p4d $pkgf $apollod $apollof $pack_apollo $pack_debug $miscd $protobuf"


    cd /sw
    mkdir -p $DST
    for f in $files ; do
        if [ ! -e "$DST/$f" ];then
            cp  -r --parents -u $f $DST
        else
            echo "Skipping files/dir : $f"
        fi
    done
    touch $DST/nic/tools/fetch-buildroot.sh
    chmod 755 $DST/nic/tools/fetch-buildroot.sh
    rm $DST/nic/include/hal_pd_error.hpp
    cp -H nic/include/hal_pd_error.hpp  $DST/nic/include
    cp -H nic/include/trace.hpp  $DST/nic/include
    cp -H nic/include/accel_ring.h  $DST/nic/include
    sed -i '2,$d' $DST/nic/tools/gen_version.py
    if [ $agent == 0 ];then
        rm $DST/nic/apollo/tools/start-agent-sim.sh $DST/nic/apollo/tools/start-agent.sh
        rm -rf $DST/nic/apollo/agent
    fi
    cp nic/conf/catalog* $DST/nic/conf
    cp nic/conf/serdes* $DST/nic/conf
    cd -

    # libdir
    mkdir -p $LIBDIR
}

build() {
    cd $DST/nic
    make PIPELINE=apollo
    make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64
    make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64 firmware
    cd -
}

remove_files() {
    # Mention the  source and module.mk to be deleted
    cd $DST
    rm nic/sdk/platform/capri/csrint/module.mk
    find ./nic/sdk/third-party/asic -name "*.c" | xargs rm
    find ./nic/sdk/third-party/asic -name "*.cc" | xargs rm

    # Copy back drivers
    rm -rf platform/src/lib/*
    rm -rf platform/drivers
    rm -rf platform/gen
    cd -
    cd /tmp/drivers
    find . -name *.ko | xargs -i{} cp -H --parents -u {} $DST
    cd -

    # Copy back header files to platform/src/lib
    cd /tmp/platform/
    cp -r --parents -u . $DST/platform/src/lib
    cd -
}

save_files() {
    # Mention the .so to be saved and restored'
    files='libsdkcapri_csrint.so libnicmgr_apollo.so libpciemgr_if.so librdmamgr_apollo.so'
    # Platform includes used by nicmgr
    platform_inc='pciemgr_if/include/pciemgr_if.hpp '
    platform_inc+='nicmgr/include/dev.hpp nicmgr/include/pd_client.hpp nicmgr/include/device.hpp nicmgr/include/pal_compat.hpp '
    platform_inc+='nicmgr/include/eth_dev.hpp nicmgr/include/eth_lif.hpp nicmgr/include/logger.hpp '

    mkdir -p $LIBDIR
    cd $DST/nic/build
    for f in $files ; do
        find . -name $f | xargs -i{} cp -H --parents -u {} $LIBDIR
    done
    cd -

    # Keep only *.ko
    cd $DST
    rm -rf /tmp/drivers
    mkdir -p /tmp/drivers
    find ./platform -name *.ko | xargs -i{} cp -H --parents -u {} /tmp/drivers/
    cd -

    # Keep the required platform headers
    cd $DST/platform/src/lib
    rm -rf /tmp/platform
    mkdir -p /tmp/platform
    for f in $platform_inc ; do
        cp -r --parents -u $f /tmp/platform
    done
    cd -
}

remove_hiddens() {
    cd $DST
    rm -rf .job*
    rm -rf .clang*
    rm -rf .appro*
    rm -rf .warmd*
    rm -rf .git*
    cd -
}

remove_build() {
    cd $DST/nic
    rm -rf build ../fake_root_target/
    rm -f *.tar *.tgz *.log
    cd -
}

rebuild() {
    mkdir $DST/nic/build
    cd $LIBDIR
    cp -r --parents -u . $DST/nic/build/
    cd -
    build
}

run_test() {
    cd $DST/nic
    ./apollo/test/tools/run_apollo_scale_test.sh
    cd -
    remove_build
}

if [ $# != 1 ];then
    echo "Usage : ./build_dev_docker.sh <agent(1/0)>"
    exit;
fi

copy_files
build
# TODO check for build success before removing the asic files
save_files
remove_files
remove_hiddens
remove_build
rebuild
run_test
