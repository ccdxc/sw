#!/bin/sh

# Checkout the repository and submodules
# Do pull-assets
# Goto docker container, and execute below from <sw> directory.

DST='/sw/apollo_sw'
LIBDIR=$DST/nic/sdk/third-party/libs
agent=$1
buildarch=$2
pipeline=$3
SWMNTDIR='/usr/src/github.com/pensando/sw'
CDIR=`pwd`
set -x

copy_job_files() {
    cp /sw/.job.yml $DST
    cp /sw/nic/.job.yml $DST/nic
    cp /sw/box* $DST
    cp /sw/nic/box* $DST/nic
    cp /sw/nic/run.py $DST/nic
}

copy_files() {
    # Add a space in end of each line.
    nicd='nic/sdk nic/buildroot nic/mkdefs nic/tools/ncc nic/utils/pack_bytes '
    nicf='nic/Makefile nic/include/capri_barco.h nic/tools/print-cores.sh nic/tools/savelogs.sh '
    nicf+='nic/tools/merge_model_debug.py nic/tools/relative_link.sh nic/include/globals.hpp '
    nicf+='nic/include/notify.hpp nic/include/edmaq.h nic/include/eth_common.h '
    nicf+='nic/include/adminq.h nic/include/nvme_dev_if.h nic/include/virtio_dev_if.h '

    p4d='nic/p4/include nic/p4/common nic/p4/common-p4+ nic/asm/common-p4+/include/ nic/p4-hlir '
    p4d+='nic/include/hal_pd_error.hpp nic/p4/eth nic/asm/eth '
    p4d+='nic/p4/adminq nic/p4/edma nic/p4/notify nic/asm/adminq nic/asm/edma nic/asm/notify '

    pkgf='nic/tools/package/package.py nic/tools/package/pack_host.txt '
    pkgf+='nic/tools/update_version.sh nic/tools/core_count_check.sh nic/tools/package/pack_platform.txt '
    pkgf+='nic/tools/package/pack_debug.txt nic/tools/upgrade_version.sh nic/tools/gen_version.py '
    pkgf+='nic/tools/package/pack_test_utils.txt nic/tools/hal/mem_parser.py nic/hal/module_memrgns.mk '

    vppd='nic/vpp '
    operd='nic/operd '

    apollod='nic/apollo '

    if [ "$pipeline" == "apulu" ];then
        pkgf+='nic/tools/package/pack_apulu_base.txt '
        pkgf+='nic/tools/package/pack_apulu_extern.txt '
        pkgf+='nic/tools/package/pack_apulu_venice.txt '
        apollod+='nic/conf/apulu '
        if [ $agent == 1 ];then
            pkgf+='nic/tools/package/pack_apulu_venice.txt '
        fi
    elif [ "$pipeline" == "apollo" ];then
        pkgf+='nic/tools/package/pack_apollo.txt '
        apollod+='nic/conf/apollo '
    elif [ "$pipeline" == "athena" ];then
        pkgf+='nic/tools/package/pack_athena.txt '
        apollod+='nic/conf/athena '
        nicf+='nic/include/base.hpp nic/include/ftl_dev_if.hpp nic/hal/hal_trace.hpp '
        #nicf+='nic/p4/ftl_dev/include/ftl_dev_shared.h nic/hal/pd/pd.hpp '
        nicf+='nic/hal/pd/pd.hpp '
        nicf+='nic/third-party/gflags/include/gflags '
        nicf+='nic/sdk/third-party/zmq/include '
        nicf+='nic/third-party/gflags/aarch64/lib '
        nicf+='nic/third-party/gflags/x86_64/lib '
        p4d+='nic/asm/ftl_dev nic/p4/ftl_dev '
    fi

    utilsd='nic/utils/ftlite '

    pack_apollo='nic/conf/init_bins nic/conf/catalog_hw.json nic/conf/serdes.json '
    pack_apollo+='nic/tools/sysupdate.sh nic/tools/apollo nic/tools/sysreset.sh nic/tools/fwupdate '
    pack_apollo+='nic/tools/postinstall.sh '
    pack_apollo+='platform/src/app/cpld '
    pack_apollo+='platform/src/boot/boot0 '
    pack_apollo+='nic/conf/captrace platform/src/app/pciemgrd nic/hal/third-party/spdlog/include/ '
    pack_apollo+='platform/src/app/memtun nic/hal/third-party/judy '
    pack_apollo+='platform/src/lib/pciemgr_if platform/drivers  platform/src/lib/nicmgr '
    pack_apollo+='platform/src/lib/eth_p4plus '
    pack_apollo+='platform/src/lib/rdmamgr_apollo '

    protobuf=" "
    metaswitch=" "
    if [ $agent == 1 ];then
        if [ "$pipeline" != "athena" ];then
            export AGENT_MODE=1
        fi
        protobuf+='vendor/github.com vendor/golang.org vendor/google.golang.org vendor/gopkg.in '
        protobuf+='venice/utils/log venice/utils/testenv venice/utils/apigen/annotations venice/Makefile.proto '
        protobuf+='nic/hal/third-party/google nic/proto/hal/module_gogo.mk nic/hal/third-party/grpc '
        protobuf+='nic/third-party/gflags/include nic/hal/third-party/openssl/include '
        protobuf+='nic/third-party/libz nic/third-party/liblmdb/include '
        protobuf+='bin/upx '
        metaswitch+='nic/metaswitch nic/third-party/metaswitch nic/third-party/libxml2 '
        metaswitch+='nic/third-party/libicuuc nic/third-party/liblzma nic/third-party/libz '

        mkdir -p $DST/proto_bin
        cp -r /usr/bin/proto* $DST/proto_bin
        cp -r /usr/local/go $DST/
    fi

    miscd='nic/upgrade_manager/meta nic/utils/trace nic/utils/ftl nic/utils/ftlv4 '

    pack_debug='nic/debug_cli nic/tools/p4ctl '

    files="$nicd $nicf $p4d $pkgf $utilsd $apollod $apollof $pack_apollo $pack_debug $miscd $protobuf $metaswitch $vppd $operd"


    cd /sw
    mkdir -p $DST
    for f in $files ; do
        if [ ! -e "$DST/$f" ];then
            cp  -r --parents -u $f $DST
            echo "Copied file/dir : $f"
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
    cp -r nic/conf/dhcp-server $DST/nic/conf/dhcp-server

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

    # exclude list
    rm -rf $DST/nic/metaswitch/rtrctl
}

script_exit() {
    rb=$1
    rv=$2
    if [ $rb == 1 ];then
        cd /
        #umount /sw
        #umount $SWMNTDIR
        #mount --bind /sw $SWMNTDIR
    fi
    cd $CDIR
    echo "Done with script_exit"
    exit $rv
}

build() {
    rb=$1
    cd $2
    if [[ "$buildarch" == "aarch64" || $buildarch == "all" ]];then
        make PIPELINE=$pipeline  PLATFORM=hw ARCH=aarch64 firmware
        [[ $? -ne 0 ]] && echo "Aborting make!" && script_exit $rb 1
    fi

    if [[ "$buildarch" == "x86_64" || "$buildarch" == "all" ]];then
        make PIPELINE=$pipeline
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
    files='libsdkcapri_csrint.so libnicmgr_'$pipeline'.so libpciemgr_if.so librdmamgr_apollo.so pdsctl.gobin libeth_p4plus.so '

    # Platform includes used by nicmgr
    platform_inc='pciemgr_if/include/pciemgr_if.hpp '
    platform_inc+='nicmgr/include/dev.hpp nicmgr/include/pd_client.hpp nicmgr/include/device.hpp nicmgr/include/pal_compat.hpp nicmgr/include/upgrade.hpp '
    platform_inc+='nicmgr/include/eth_dev.hpp nicmgr/include/eth_lif.hpp nicmgr/include/logger.hpp nicmgr/include/nicmgr_utils.hpp nicmgr/include/ftl_dev.hpp nicmgr/include/ftl_lif.hpp eth_p4plus/eth_p4pd.hpp '

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

remove_all_job_files() {
    cd $DST
    rm -rf .job*
    rm box*
}

remove_hiddens() {
    echo "Skip .job removal"
    cd $DST
    #rm -rf .job*
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
    #umount $SWMNTDIR
    #mount --bind /sw/apollo_sw $SWMNTDIR
    sudo mount --bind /sw/apollo_sw /sw

    #build 1 /sw/nic

    #Dont need tests here in this script
    #cd /sw/nic
    #if [[ "$buildarch" == "x86_64" || "$buildarch" == "all" ]];then
    #    ./apollo/test/scale/run_scale_test_mock.sh --cfg scale_cfg_1vpc.json
    #    [[ $? -ne 0 ]] && echo "Test failed!" && script_exit 1 1
    #fi
}

build_docker() {
    cd $SWMNTDIR
    tar cf  sw.tar /sw
    tar cf inc.tar /usr/local/include
    sh -c "cp /sw/nic/apollo/tools/athena/sanity/customer-docker/Dockerfile . && docker build -t customimage:v2 . && docker save customimage:v2 | gzip > customimage.tar.gz"
    echo "done with Docker Save"
    docker images
    docker image rm customimage:v2
    rm sw.tar
    rm inc.tar
    rm -rf $SWMNTDIR/apollo_sw
}

if [ $# lt 3 ];then
    echo "Usage : ./build_dev_docker.sh <agent(1/0)> <buildarch(aarch64/x86_64/all)> <pipeline(apollo/apulu/athena)"
    exit;
fi

echo "Invoke copy_files"
copy_files
echo "Invoke copy_job_files"
copy_job_files
#remove_all_job_files
echo "Invoke build"
build 0 $DST/nic
# TODO check for build success before removing the asic files
echo "Invoke save_files"
save_files
echo "Invoke remove_files"
remove_files
echo "Invoke remove_hiddens"
remove_hiddens
echo "Invoke remove_build"
remove_build $DST/nic
echo "Invoke rebuild_and_runtest"
rebuild_and_runtest
# Comment the below calls, if we want to keep the images build for future debug
echo "Invoke remove_build"
remove_build /sw/nic
echo "Invoke script_exit"
if [ -z "$4" ]; then
    script_exit 1 0
else 
    if [ "$pipeline" == "athena" ]; then
        build_docker
        script_exit 1 0
    fi
fi
echo "Post script_exit"
