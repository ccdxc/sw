#!/bin/sh

# Checkout the repository and submodules
# Do pull-assets 
# Goto docker container, and execute below from <sw> directory.

DST='/sw/apollo_sw'
LIBDIR=$DST/nic/sdk/third-party/libs
set -x
copy_files() {
    # Add a space in end of each line.
    nicd='nic/sdk nic/buildroot nic/mkdefs nic/tools/ncc nic/utils/pack_bytes '
    nicf='nic/Makefile nic/include/capri_barco.h nic/tools/print-cores.sh nic/tools/savelogs.sh '
    nicf+='nic/tools/merge_model_debug.py '

    p4d='nic/p4/include nic/p4/common nic/p4/common-p4+ nic/asm/common-p4+/include/ nic/p4-hlir ' 
    p4d+='nic/include/hal_pd_error.hpp '

    pkgf='nic/tools/package/package.py nic/tools/package/pack_host.txt nic/tools/package/pack_apollo.txt '
    pkgf+='nic/tools/update_version.sh nic/tools/core_count_check.sh nic/tools/package/pack_platform.txt '
    pkgf+='nic/tools/package/pack_debug.txt nic/tools/upgrade_version.sh nic/tools/gen_version.py '

    apollod='nic/apollo nic/conf/apollo '
    apollof='nic/conf/catalog.json '

    pack_apollo='nic/conf/init_bins nic/conf/catalog_hw.json nic/conf/serdes.json '
    pack_apollo+='nic/tools/sysupdate.sh nic/tools/apollo nic/tools/sysreset.sh nic/tools/fwupdate '
    pack_apollo+='nic/conf/captrace platform/src/app/pciemgrd nic/hal/third-party/spdlog/include/ ' 
    pack_apollo+='platform/src/app/memtun nic/hal/third-party/judy '

    pack_debug='nic/debug_cli nic/tools/p4ctl '

    files="$nicd $nicf $p4d $pkgf $apollod $apollof $pack_apollo $pack_debug"

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
    cd -

    # libdir
    mkdir -p $LIBDIR
}

build() {
    cd $DST/nic
    make PIPELINE=apollo
    make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64 PERF=1
    make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64 PERF=1 firmware
    cd -
}

remove_files() {
    # Mention the  source and module.mk to be deleted
    cd $DST
    rm nic/sdk/platform/capri/csrint/module.mk
    find ./nic/sdk/third-party/asic -name "*.c" | xargs rm
    find ./nic/sdk/third-party/asic -name "*.cc" | xargs rm
    cd -
}

save_files() {
    # Mention the .so to be saved and restored'
    files='libsdkcapri_csrint.so '

    mkdir -p $LIBDIR
    cd $DST/nic/build
    for f in $files ; do
        find . -name $f | xargs -i{} cp -H --parents -u {} $LIBDIR
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

copy_files
build
# TODO check for build success before removing the asic files
save_files
remove_files
remove_hiddens
