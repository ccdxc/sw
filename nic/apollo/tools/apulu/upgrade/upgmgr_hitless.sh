#!/bin/bash
# top level script which will be invoked by upgrade manager before
# and after every stage of execution
# the base(hw/sim) environment variable is already set during upgmgr invocation
echo "argument -- $*"

# BUILD_DIR is defined only on sim/mock mode
if [[ ! -z $BUILD_DIR ]];then
     $PDSPKG_TOPDIR/apollo/tools/$PIPELINE/upgrade/upgmgr_hitless_mock.sh $*
     [[ $? -ne 0 ]] && echo "Upgrade hooks, command execution failed for mock/sim!" && exit 1
     exit 0
fi

source $PDSPKG_TOPDIR/tools/upgmgr_base.sh
upgmgr_parse_inputs $*

echo "Starting commands for $STAGE_NAME"
// TODO
echo "Commands for $STAGE_NAME processed successfully"
exit 0
