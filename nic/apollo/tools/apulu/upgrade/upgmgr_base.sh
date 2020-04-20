#!/bin/bash
# called in compat checks to validate the image present or not
unset HELP FW_PKG_NAME FW_PATH STAGE_NAME RUNNING_IMAGE
unset STAGE_TYPE STAGE_STATUS

# file to identify the bootup is regular or upgrade
UPGRADE_INIT_MODE_FILE='/data/upgrade_init_mode'
UPGRADE_BACKUP_TAR_FILE='/data/pds_upgrade_data.tgz'

function upgmgr_parse_inputs() {
    while getopts 'hf:s:t:r:' c
    do
        case $c in
            h) HELP=1 ;;
            f) FW_PKG_NAME=$OPTARG ;;
            s) STAGE_NAME=$OPTARG ;;
            t) STAGE_TYPE=$OPTARG ;;
            r) STAGE_STATUS=$OPTARG ;;
        esac
    done

    if [[ -z "$FW_PKG_NAME" || -z "$STAGE_NAME" ||
            -z "$STAGE_TYPE" || -z "$STAGE_STATUS" ]];then
        echo "Usage <command> -f <pkg-name> -s <stage-name> -t <pre/post> -r <status>"
        exit 1
    fi
}

function upgmgr_setup() {
    source /nic/tools/setup_env_hw.sh
    FW_UPDATE_TOOL=$PDSPKG_TOPDIR/tools/fwupdate

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
    CC_CHECK_TOOL=${PDSPKG_TOPDIR}/tools/upgmgr_cc_meta.py

    # verify the image
    $FW_UPDATE_TOOL -p $FW_PATH -v
    [[ $? -ne 0 ]] && echo "FW verfication failed!" && exit 1

    # extract meta files
    META_NEW='/tmp/meta_cc_new.json'
    META_RUNNING='/tmp/meta_cc_running.json'
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
    files_must="/update/pcieport_upgdata /update/pciemgr_upgdata "
    files_must+="/update/pds_upgrade "
    files_optional="/update/pciemgr_upgrollback "
    for e in $files_must; do
        if [[ ! -f $e ]]; then
            echo "File $e not present"
            return 1
        fi
    done
    tar -cvzf $UPGRADE_BACKUP_TAR_FILE $files_must $files_optional
    return 0
}

function upgmgr_restore() {
    mkdir -p /update/
    tar -C / -xvzf $UPGRADE_BACKUP_TAR_FILE
    return $?
}
