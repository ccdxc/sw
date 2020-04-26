#!/bin/bash
# top level script which will be invoked by upgrade manager before
# and after every stage of execution
# the base(hw/sim) environment variable is already set during upgmgr invocation
echo "argument -- $*"

# BUILD_DIR is defined only on sim/mock mode
if [[ ! -z $BUILD_DIR ]];then
     $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/upgrade/upgmgr_graceful_mock.sh $*
     [[ $? -ne 0 ]] && echo "Upgrade hooks, command execution failed for mock/sim!" && exit 1
     exit 0
fi

source $PDSPKG_TOPDIR/tools/upgmgr_base.sh
upgmgr_parse_inputs $*

echo "Starting commands for $STAGE_NAME"

if [[ $STAGE_NAME = "UPG_STAGE_COMPAT_CHECK" && $STAGE_TYPE == "pre" ]];then
    upgmgr_setup
    upgmgr_pkgcheck
    [[ $? -ne 0 ]] && echo "Package check failed!" && exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_START" && $STAGE_TYPE == "post" ]]; then
    upgmgr_setup
    # verification of the image is already done
    $SYS_UPDATE_TOOL -p $FW_PATH
    [[ $? -ne 0 ]] && echo "Firmware store failed!" && exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_PREP_SWITCHOVER" && $STAGE_TYPE == "post" ]]; then
    upgmgr_backup
    [[ $? -ne 0 ]] && echo "Files backup failed!" && exit 1
    pkill vpp

elif [[ $STAGE_NAME == "UPG_STAGE_SWITCHOVER" && $STAGE_TYPE == "pre" ]]; then
    wait_for_vpp_exit
    [[ $? -ne 0 ]] && echo "VPP exit failed!" && exit 1
    upgmgr_set_init_mode "graceful"
    $PDSPKG_TOPDIR/tools/switch_rootfs.sh altfw
    # we will not come back here, it is a failure in that case
    upgmgr_clear_init_mode
    echo "Switchroot failed"
    exit 1

elif [[ $STAGE_NAME == "UPG_STAGE_READY" && $STAGE_TYPE == "post" ]]; then
    # post ready below is no more in use
    upgmgr_clear_init_mode
else
    echo "Unknown stage name given"
    exit 1
fi

echo "Commands for $STAGE_NAME processed successfully"
exit 0
