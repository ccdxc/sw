#!/bin/sh

# Checkout the repository and submodules
# Do pull-assets 
# Goto docker container, and execute below from /sw/ directory.

DST='/sw/apollo_sw'
LIBDIR=$DST/nic/sdk/third-party/libs
set -x
copy_files() {
    # Add a space in end of each line.
    nicd='nic/sdk nic/buildroot nic/mkdefs nic/tools/ncc nic/utils/pack_bytes'
    nicf='nic/Makefile nic/include/capri_barco.h nic/tools/print-cores.sh nic/tools/savelogs.sh '

    p4d='nic/p4/include nic/p4/common nic/p4/common-p4+ nic/asm/common-p4+/include/ nic/p4-hlir ' 

    pkgf='nic/tools/package/package.py nic/tools/package/pack_host.txt nic/tools/package/pack_apollo.txt '
    pkgf+='nic/tools/update_version.sh nic/tools/core_count_check.sh nic/tools/package/pack_platform.txt '
    pkgf+='nic/tools/package/pack_debug.txt nic/tools/upgrade_version.sh '

    apollod='nic/apollo nic/conf/apollo '
    apollof='nic/conf/catalog.json '

    simf='nic/run.py '

    files="$nicd $nicf $p4d $pkgf $apollod $apollof $simf"

    cd /sw
    mkdir -p $DST
    for f in $files ; do
        if [ ! -e "$DST/$f" ];then
            cp -r --parents -u $f $DST
        else
            echo "Skipping files/dir : $f"
        fi
    done
    cd -
}

apply_patches() {
    # Assets are already pulled and copied
    touch nic/tools/fetch-buildroot.sh
    chmod 755 nic/tools/fetch-buildroot.sh
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

build() {
    cd $DST/nic
    make PIPELINE=apollo 
    make PIPELINE=apollo  PLATFORM=hw ARCH=aarch64
    cd -
}

copy_files
build
save_files
remove_files
remove_hiddens
