#!/bin/sh

# Checkout the repository and submodules
# Do pull-assets
# Goto docker container, and execute below from <sw> directory.

DST='/sw/apollo_sw'
LIBDIR=$DST/nic/sdk/third-party/libs
agent=$1
buildarch=$2
SWMNTDIR='/usr/src/github.com/pensando/sw'
CDIR=`pwd`
set -x
copy_files() {
    # Add a space in end of each line.
    nicd='nic/sdk nic/buildroot nic/mkdefs nic/tools/ncc nic/utils/pack_bytes '
    nicf='nic/Makefile nic/include/capri_barco.h nic/tools/print-cores.sh nic/tools/savelogs.sh '
    nicf+='nic/tools/merge_model_debug.py nic/tools/relative_link.sh nic/include/globals.hpp '
    nicf+='nic/include/notify.hpp nic/include/edmaq.h nic/include/eth_common.h '
    nicf+='nic/include/adminq.h nic/include/nvme_dev_if.h nic/include/virtio_dev_if.h'

    p4d='nic/p4/include nic/p4/common nic/p4/common-p4+ nic/asm/common-p4+/include/ nic/p4-hlir '
    p4d+='nic/include/hal_pd_error.hpp nic/p4/eth nic/asm/eth '
    p4d+='nic/p4/adminq nic/p4/edma nic/p4/notify nic/asm/adminq nic/asm/edma nic/asm/notify '

    pkgf='nic/tools/package/package.py nic/tools/package/pack_host.txt nic/tools/package/pack_apollo.txt '
    pkgf+='nic/tools/update_version.sh nic/tools/core_count_check.sh nic/tools/package/pack_platform.txt '
    pkgf+='nic/tools/package/pack_debug.txt nic/tools/upgrade_version.sh nic/tools/gen_version.py '
    pkgf+='nic/tools/package/pack_test_utils.txt nic/tools/hal/mem_parser.py nic/hal/module_memrgns.mk '

    apollod='nic/apollo nic/conf/apollo '

    utilsd='nic/utils/ftlite '

    pack_apollo='nic/conf/init_bins nic/conf/catalog_hw.json nic/conf/serdes.json '
    pack_apollo+='nic/tools/sysupdate.sh nic/tools/apollo nic/tools/sysreset.sh nic/tools/fwupdate '
    pack_apollo+='nic/conf/captrace platform/src/app/pciemgrd nic/hal/third-party/spdlog/include/ '
    pack_apollo+='platform/src/app/memtun nic/hal/third-party/judy '
    pack_apollo+='platform/src/lib/pciemgr_if platform/drivers  platform/src/lib/nicmgr '
    pack_apollo+='platform/src/lib/rdmamgr_apollo '

    protobuf=" "
    if [ $agent == 1 ];then
        export AGENT_MODE=1
        protobuf+='vendor/github.com vendor/golang.org vendor/google.golang.org vendor/gopkg.in '
        protobuf+='venice/utils/log venice/utils/testenv '
        protobuf+='nic/hal/third-party/google nic/proto/hal/module_gogo.mk nic/hal/third-party/grpc '
        protobuf+='nic/third-party/gflags/include nic/hal/third-party/openssl/include '
        protobuf+='nic/third-party/libz nic/third-party/liblmdb/include '
        protobuf+='bin/upx '
        mkdir -p $DST/proto_bin
        cp -r /usr/bin/proto* $DST/proto_bin
        cp -r /usr/local/go $DST/
    fi

    miscd='nic/upgrade_manager/meta nic/utils/trace nic/utils/ftl nic/utils/ftlv4 '

    pack_debug='nic/debug_cli nic/tools/p4ctl '

    metaswitch='nic/metaswitch nic/third-party/metaswitch nic/third-party/libxml2 nic/third-party/libicuuc nic/third-party/liblzma nic/third-party/libz '

    files="$nicd $nicf $p4d $pkgf $utilsd $apollod $apollof $pack_apollo $pack_debug $miscd $protobuf $metaswitch"


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

    # Patch for gen_version
    ./nic/tools/gen_version.py --output-dir $DST/nic/conf
    sed -i '2,$d' $DST/nic/tools/gen_version.py
    echo -e "import os\nimport sys\nos.system('cp /sw/nic/conf/VERSION.json %s' %sys.argv[2])\n" >> $DST/nic/tools/gen_version.py

    if [ $agent == 0 ];then
        rm $DST/nic/apollo/tools/start-agent-sim.sh $DST/nic/apollo/tools/start-agent.sh
        rm -rf $DST/nic/apollo/agent
        find $DST/nic/apollo -name module_agenthooks.mk | xargs rm
    fi
    cp nic/conf/catalog* $DST/nic/conf
    cp nic/conf/serdes* $DST/nic/conf

    # libdir
    mkdir -p $LIBDIR
}

script_exit() {
    rb=$1
    rv=$2
    if [ $rb == 1 ];then
        cd /
        umount /sw
        umount $SWMNTDIR
        mount --bind /sw $SWMNTDIR
    fi
    cd $CDIR
    exit $rv
}

build() {
    rb=$1
    cd $2
    if [[ "$buildarch" == "aarch64" || $buildarch == "all" ]];then
        make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64 firmware
        [[ $? -ne 0 ]] && echo "Aborting make!" && script_exit $rb 1
    fi

    if [[ "$buildarch" == "x86_64" || "$buildarch" == "all" ]];then
        make PIPELINE=apollo
        [[ $? -ne 0 ]] && echo "Aborting make!" && script_exit $rb 1
    fi
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
    cd /tmp/drivers
    find . -name *.ko | xargs -i{} cp -H --parents -u {} $DST

    # Copy back header files to platform/src/lib
    cd /tmp/platform/
    cp -r --parents -u . $DST/platform/src/lib
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

    # Keep only *.ko
    cd $DST
    rm -rf /tmp/drivers
    mkdir -p /tmp/drivers
    find ./platform -name *.ko | xargs -i{} cp -H --parents -u {} /tmp/drivers/

    # Keep the required platform headers
    cd $DST/platform/src/lib
    rm -rf /tmp/platform
    mkdir -p /tmp/platform
    for f in $platform_inc ; do
        cp -r --parents -u $f /tmp/platform
    done
}

remove_hiddens() {
    cd $DST
    rm -rf .job*
    rm -rf .clang*
    rm -rf .appro*
    rm -rf .warmd*
    rm -rf .git*
}

remove_build() {
    cd $1
    rm -rf build ../fake_root_target/
    rm -f *.tar *.tgz *.log
}

rebuild_and_runtest() {
    cd $LIBDIR
    mkdir -p $DST/nic/build
    cp -r --parents -u . $DST/nic/build/

    cd /
    umount $SWMNTDIR
    mount --bind /sw/apollo_sw $SWMNTDIR
    mount --bind /sw/apollo_sw /sw

    build 1 /sw/nic

    cd /sw/nic
    if [[ "$buildarch" == "x86_64" || "$buildarch" == "all" ]];then
        ./apollo/test/scale/run_scale_test_mock.sh --cfg scale_cfg_1vpc.json
        [[ $? -ne 0 ]] && echo "Test failed!" && script_exit 1 1
    fi
}

if [ $# != 2 ];then
    echo "Usage : ./build_dev_docker.sh <agent(1/0)> <buildarch(aarch64/x86_64/all)"
    exit;
fi

copy_files
build 0 $DST/nic
# TODO check for build success before removing the asic files
save_files
remove_files
remove_hiddens
remove_build $DST/nic
rebuild_and_runtest
# Comment the below calls, if we want to keep the images build for future debug
remove_build /sw/nic
script_exit 1 0
