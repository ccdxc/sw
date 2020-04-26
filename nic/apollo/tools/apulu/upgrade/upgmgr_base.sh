#!/bin/bash

unset FW_PATH RUNNING_IMAGE FW_UPDATE_TOOL SYS_UPDATE_TOOL

source $PDSPKG_TOPDIR/tools/upgmgr_core_base.sh

function upgmgr_setup() {
    FW_UPDATE_TOOL=$PDSPKG_TOPDIR/tools/fwupdate
    SYS_UPDATE_TOOL=$PDSPKG_TOPDIR/tools/sysupdate.sh

    if [[ -f "$FW_PKG_NAME" ]];then
        FW_PATH=$FW_PKG_NAME
    elif [[ -f "/update/$FW_PKG_NAME" ]];then
        FW_PATH="/update/$FW_PKG_NAME"
    else
        echo "$FW_PKG_NAME could not locate"
        exit 1
    fi

    if [[ ! -x "$FW_UPDATE_TOOL" ]];then
        echo "$FW_UPDATE_TOOL could not locate"
        exit 1
    fi

    RUNNING_IMAGE=`$FW_UPDATE_TOOL -r`
}

function upgmgr_pkgcheck() {
    local CC_CHECK_TOOL=${PDSPKG_TOPDIR}/tools/upgmgr_cc_meta.py

    # verify the image
    $FW_UPDATE_TOOL -p $FW_PATH -v
    [[ $? -ne 0 ]] && echo "FW verfication failed!" && exit 1

    # extract meta files
    local META_NEW='/tmp/meta_cc_new.json'
    local META_RUNNING='/tmp/meta_cc_running.json'
    rm -rf $META_NEW $META_RUNNING

    # extract meta file from new firmware
    tar -xf $FW_PATH MANIFEST -O >  $META_NEW
    [[ $? -ne 0 ]] && echo "Meta extraction from new fw failed!" && exit 1

    # extract meta file from running firmware
    $FW_UPDATE_TOOL -l > $META_RUNNING
    [[ $? -ne 0 ]] && echo "Meta extraction from running fw failed!" && exit 1

    # compare the extracted files
    python $CC_CHECK_TOOL $META_RUNNING $RUNNING_IMAGE $META_NEW
    if [[ $? -ne 0 ]];then
        rm -rf $META_NEW $META_RUNNING
        echo "Meta comparison failed!"
        exit 1
    fi
    rm -rf $META_NEW $META_RUNNING
    return 0
}

function upgmgr_backup() {
    cp /dev/shm/pds_upgrade /update/pds_upgdata
    local files_must="/update/pcieport_upgdata /update/pciemgr_upgdata "
    files_must+="/update/pds_upgdata "
    local files_optional="/update/pciemgr_upgrollback "
    for e in $files_must; do
        if [[ ! -f $e ]]; then
            echo "File $e not present"
            return 1
        fi
    done
    return 0
}

function upgmgr_restore() {
    # TODO : check graceful upgrade and validate the file existance
    cp /update/pds_upgdata /dev/shm/pds_upgrade
}

function wait_for_vpp_exit() {
# vpp uses mnet uio, and its exis is mandatory for switch_rootfs.sh
# to succeed in rmmod of this driver
    local refcnt=`lsmod | grep mnet_uio_pdrv_genirq | awk '{print $3}'`
    local count=0
    while [[ $refcnt -gt 1 && $count -lt 5000 ]];do
        refcnt=`lsmod | grep mnet_uio_pdrv_genirq | awk '{print $3}'`
        usleep 10
        count=`expr $count + 1`
    done
    if  [[ $count -lt 5000 ]];then
        return 0
    else
        echo "vpp exit, uio driver refcount check exceeded the limit $count"
        return 1
    fi
}
