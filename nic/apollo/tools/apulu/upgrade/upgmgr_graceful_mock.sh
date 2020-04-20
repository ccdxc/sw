#!/bin/bash
# top level script which will be invoked by upgrade manager before
# and after every stage of execution

echo "In mock/sim, starting commands for $STAGE_NAME"

if [[ $STAGE_NAME = "UPG_STAGE_COMPAT_CHECK" && $STAGE_TYPE == "pre" ]];then
    echo "compat check, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_START" && $STAGE_TYPE == "post" ]]; then
    echo "start, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_PREP_SWITCHOVER" && $STAGE_TYPE == "post" ]]; then
    echo "prepare switchover, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_SWITCHOVER" && $STAGE_TYPE == "pre" ]]; then
    echo "switchover, skipping"

elif [[ $STAGE_NAME == "UPG_STAGE_READY" && $STAGE_TYPE == "post" ]]; then
    echo "finish, skipping"
else
    echo "unknown input"
    exit 1
fi

echo "Commands for $STAGE_NAME processed successfully"
exit 0
