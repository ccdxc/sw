#!/bin/bash
# top level script which will invoke this if mock/sim mode set

source $PDSPKG_TOPDIR/sdk/upgrade/core/upgmgr_core_base.sh
upgmgr_parse_inputs $*

echo "In mock/sim, starting commands for $STAGE_NAME"

if [[ $STAGE_NAME = "UPG_STAGE_COMPAT_CHECK" && $STAGE_TYPE == "pre" ]];then
    echo "compat check, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_START" && $STAGE_TYPE == "post" ]]; then
    echo "start, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_PREP_SWITCHOVER" && $STAGE_TYPE == "post" ]]; then
    echo "prepare switchover, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_SWITCHOVER" && $STAGE_TYPE == "pre" ]]; then
    upgmgr_set_init_mode "graceful"
    echo "switchover, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_READY" && $STAGE_TYPE == "post" ]]; then
    upgmgr_clear_init_mode
    echo "finish, skipping"
else
    echo "unknown input"
    exit 1
fi

echo "Commands for $STAGE_NAME processed successfully"
exit 0
