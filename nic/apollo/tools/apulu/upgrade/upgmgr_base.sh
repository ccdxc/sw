#!/bin/bash

unset FW_PATH RUNNING_IMAGE FW_UPDATE_TOOL SYS_UPDATE_TOOL

source $PDSPKG_TOPDIR/tools/upgmgr_core_base.sh

UPGRADE_STATUS_FILE='/update/pds_upg_status.txt'

function upgmgr_set_upgrade_status() {
    echo "success" > $UPGRADE_STATUS_FILE
}

function upgmgr_clear_upgrade_status() {
    rm -f $UPGRADE_STATUS_FILE
}

function upgmgr_setup() {
    FW_UPDATE_TOOL=$PDSPKG_TOPDIR/tools/fwupdate
    SYS_UPDATE_TOOL=$PDSPKG_TOPDIR/tools/sysupdate.sh

    # clear the status from previous run
    upgmgr_clear_upgrade_status

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
    local files_must="/update/pcieport_upgdata /update/pciemgr_upgdata "
    files_must+="/update/pds_api_upgdata "
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
    echo "Restore, nothing to do, skipping"
    return 0
}

function reload_drivers() {
    echo "Reloading mnic drivers"
    rmmod mnet mnet_uio_pdrv_genirq ionic_mnic

    # load kernel modules for mnics
    insmod $PDSPKG_TOPDIR/bin/ionic_mnic.ko &> $NON_PERSISTENT_LOGDIR/ionic_mnic_load.log
    [[ $? -ne 0 ]] && echo "Aborting reload, failed to load mnic driver!" && exit 1

    insmod $PDSPKG_TOPDIR/bin/mnet_uio_pdrv_genirq.ko &> $NON_PERSISTENT_LOGDIR/mnet_uio_pdrv_genirq_load.log
    [[ $? -ne 0 ]] && echo "Aborting reload, failed to load mnet_uio_pdrv_genirq driver!" && exit 1

    insmod $PDSPKG_TOPDIR/bin/mnet.ko &> $NON_PERSISTENT_LOGDIR/mnet_load.log
    [[ $? -ne 0 ]] && echo "Aborting reload, failed to load mnet driver!" && exit 1
}
