#!/bin/bash

# parameter extractor which should be sourceed by pipeline specific scripts.
unset HELP FW_PKG_NAME STAGE_NAME STAGE_TYPE STAGE_STATUS

# file to identify the bootup is regular or upgrade
# this should be set by pipeline specific implemenation in apropriate place
UPGRADE_INIT_MODE_FILE='/data/upgrade_init_mode.txt'

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

    if [[ -z "$FW_PKG_NAME" || -z "$STAGE_NAME" || -z "$STAGE_TYPE" ]];then
        echo "Usage <command> -f <pkg-name> -s <stage-name> -t <pre>"
        exit 1
    fi

    if [[ "$STAGE_TYPE" == "post" && -z "$STAGE_STATUS" ]];then
        echo "Usage <command> -f <pkg-name> -s <stage-name> -t <post> -r <status>"
        exit 1
    fi
}

function upgmgr_set_init_mode() {
    mkdir -p /data/
    echo $1 > $UPGRADE_INIT_MODE_FILE
}

function upgmgr_clear_init_mode() {
    rm -rf $UPGRADE_INIT_MODE_FILE
}
