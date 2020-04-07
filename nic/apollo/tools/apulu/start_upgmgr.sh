#! /bin/bash

export PIPELINE=apulu

source  /nic/tools/setup_env_hw.sh $PIPELINE
export OPERD_REGIONS=$CONFIG_PATH/operd-regions.json

echo "Starting pdsupgmgr: `date +%x_%H:%M:%S:%N`"

CMD="pdsupgmgr $* 2>&1"
$GDB $CMD
