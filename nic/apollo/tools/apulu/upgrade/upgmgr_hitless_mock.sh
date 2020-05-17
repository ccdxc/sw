#!/bin/bash
# top level script which will invoke this if mock/sim mode set

source $PDSPKG_TOPDIR/sdk/upgrade/core/upgmgr_core_base.sh
upgmgr_parse_inputs $*

echo "In mock/sim, starting commands for $STAGE_NAME"

if [[ $STAGE_NAME == "UPG_STAGE_PREPARE" && $STAGE_TYPE == "PRE" ]]; then
    # get the next domain to be booted up
    dom=$( upgmgr_get_alt_domain  )
    upgmgr_set_init_mode "hitless"
    rm -f  /tmp/agent_up
    sh $PDSPKG_TOPDIR/apollo/test/tools/apulu/start_upgrade_hitless_process.sh $dom &
    # TODO : remove this once the nicmgr is ready, wait for this to be up
    count=0
    while [ ! -f /tmp/agent_up ];do
        echo "waiting for agent up"
        sleep 1
        count=`expr $count + 1`
        if [ $count -gt 240 ];then
            echo "PDS agent not up, exiting"
            exit 1
        fi
    done
else
    echo "unknown input"
    exit 1
fi

echo "Commands for $STAGE_NAME processed successfully"
wait
exit 0
