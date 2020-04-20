#! /bin/bash

MY_DIR=$( readlink -f $( dirname $0 ))

# setup dol
DOL_ARGS='--pipeline apulu --topo learn --feature learn'
source $MY_DIR/../../../tools/setup_dol.sh $DOL_ARGS

# start pciemgr
$BUILD_DIR/bin/pciemgrd -d &

# setup upgrade
source $MY_DIR/../setup_upgrade_gtests.sh
upg_operd_init

upg_wait_for_pdsagent

# override trap
function trap_finish () {
#   pkill fsm_test
    pkill pdsupgmgr
    stop_process
    pkill pciemgrd
    stop_model
    upg_finish upgmgr
}
trap trap_finish EXIT

# start DOL now
# comment out below for debugging any application where config
# is not needed
#$DOLDIR/main.py $CMDARGS 2>&1 | tee dol.log
#status=${PIPESTATUS[0]}

# create a dummy instance from test service for easy validations which sends
# ok for all events
#$BUILD_DIR/bin/fsm_test -s sysmgr -i 62 > upgrade_service.log 2>&1 &
sleep 2

# start upgrade manager
upg_setup $BUILD_DIR/gen/upgrade_graceful.json
$BUILD_DIR/bin/pdsupgmgr -t $PDSPKG_TOPDIR/apollo/tools/apulu/upgrade > upgrade_mgr.log 2>&1 &
sleep 2

# run client
pdsupgclient
[[ $? -ne 0 ]] && echo "upgrade command failed" && exit 1
echo "upgrade command successful"

# kill testing services
echo "stopping processes including upgrademgr"
stop_process
pkill pdsupgmgr

# set upgrade init mode
export UPG_INIT_MODE='graceful'

# respawn the services
echo "respawning processes"
start_process
upg_wait_for_pdsagent

# spawn upgrade mgr to continue the post restart states
echo "respawning upgrademgr"
$BUILD_DIR/bin/pdsupgmgr > upgrade_mgr.log 2>&1 &
wait
# stop all services
trap_finish

# end of script
exit $status
