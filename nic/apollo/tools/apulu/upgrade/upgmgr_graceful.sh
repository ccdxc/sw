#!/bin/bash
# top level script which will be invoked by upgrade manager before
# and after every stage of execution
echo "argument -- $*"

CUR_DIR=$( readlink -f $( dirname $0 ) )
source ${CUR_DIR}/upgmgr_base.sh $*

upgmgr_parse_inputs $*

# call the mock/sim mode routines if it is not in hw
# BUILD_DIR is defined only on sim/mock mode
if [[ ! -z $BUILD_DIR ]];then
     $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/upgrade/upgmgr_graceful_mock.sh
     [[ $? -ne 0 ]] && echo "Upgrade hooks, command execution failed for mock/sim!" && exit 1
     exit 0
fi

upgmgr_setup

echo "Starting commands for $STAGE_NAME"

if [[ $STAGE_NAME = "UPG_STAGE_COMPAT_CHECK" && $STAGE_TYPE == "pre" ]];then
    upgmgr_pkgcheck
    [[ $? -ne 0 ]] && echo "Package check failed!" && exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_START" && $STAGE_TYPE == "post" ]]; then
    # verification of the image is already done
    $SYS_UPDATE_TOOL -p $FW_PATH
    [[ $? -ne 0 ]] && echo "Firmware store failed!" && exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_PREP_SWITCHOVER" && $STAGE_TYPE == "post" ]]; then
    upgmgr_backup
    [[ $? -ne 0 ]] && echo "Files backup failed!" && exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_SWITCHOVER" && $STAGE_TYPE == "pre" ]]; then
    echo "graceful" > $UPGRADE_INIT_MODE_FILE
    $TOOLS_DIR/switch_rootfs.sh
    # we will not come back here, it is a failure in that case
    rm -rf $UPGRADE_INIT_MODE_FILE
    echo "Switchroot failed"
    exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_READY" && $STAGE_TYPE == "post" ]]; then
    # post ready below are no more in use
    rm -rf $UPGRADE_INIT_MODE_FILE
    rm -rf $UPGRADE_BACKUP_TAR_FILE
else
    echo "Unknown stage name given"
    exit 1
fi

echo "Commands for $STAGE_NAME processed successfully"
exit 0
