#! /bin/bash

export PIPELINE=apulu

source  /nic/tools/setup_env_hw.sh $PIPELINE
source $PDSPKG_TOPDIR/tools/upgmgr_base.sh

export OPERD_REGIONS=$CONFIG_PATH/operd-regions.json

echo "Starting pdsupgmgr: `date +%x_%H:%M:%S:%N`"

# upgrade manager is spawned on demand for a new upgrade
# call with -n in that case

# $UPGRADE_INIT_MODE_FILE is created before switchroot and it will be
# deleted once the upgrade sequence is completed
# if it is present, upgmgr will be spawned to continue the upgrade sequence

if [[ -f "$UPGRADE_INIT_MODE_FILE" ]];then
    # restore the saved files
    upgmgr_restore
    [[ $? -ne 0 ]] && echo "Upgmgr restore failed!" && exit 1
fi

if [[ $1 == "-n" || -f "$UPGRADE_INIT_MODE_FILE" ]];then
    exec $PDSPKG_TOPDIR/bin/pdsupgmgr -t $PDSPKG_TOPDIR/tools/
fi

exit 0
