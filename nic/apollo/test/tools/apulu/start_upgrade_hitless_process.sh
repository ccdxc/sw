#! /bin/bash

set -x
MY_DIR=$( readlink -f $( dirname $0 ))
HITLESS_DOM=$1

export PDSPKG_TOPDIR=/tmp/$HITLESS_DOM/
export ZMQ_SOC_DIR=/sw/nic/
source $MY_DIR/../../../tools/setup_env_sim.sh $PIPELINE

# setup upgrade
source $MY_DIR/../setup_upgrade_gtests.sh
hitless_copy_files
upg_operd_init

# use 8G configuration
cp $CONFIG_PATH/catalog_hw_68-0004.json $CONFIG_PATH/$PIPELINE/catalog.json
# set upgrade mode
source $PDSPKG_TOPDIR/sdk/upgrade/core/upgmgr_core_base.sh
upgmgr_set_init_domain $CONFIG_PATH $HITLESS_DOM

function trap_finish () {
    kill_process "$BUILD_DIR/bin/pciemgrd"
    kill_process "$BUILD_DIR/bin/pdsupgmgr"
    kill_process "$PDSPKG_TOPDIR/apollo/tools/apulu/start-agent-sim.sh"
    kill_process "$BUILD_DIR/bin/pdsagent"
    kill_process "${PDSPKG_TOPDIR}sdk/third-party/vpp-pkg/x86_64/bin/vpp"
    kill_process "$PDSPKG_TOPDIR/vpp/tools/start-vpp-sim.sh"
}
trap trap_finish EXIT SIGTERM

# start processes
$BUILD_DIR/bin/pciemgrd -d &
$PDSPKG_TOPDIR/apollo/tools/$PIPELINE/start-agent-sim.sh > $PDSPKG_TOPDIR/agent.log 2>&1 &
sudo $PDSPKG_TOPDIR/vpp/tools/start-vpp-sim.sh ${DOL_ARGS}

upg_wait_for_pdsagent
touch /tmp/agent_up

# start upgrade manager
upg_setup $BUILD_DIR/gen/upgrade_hitless.json upgrade_hitless.json
$BUILD_DIR/bin/pdsupgmgr -t $PDSPKG_TOPDIR/apollo/tools/apulu/upgrade > $PDSPKG_TOPDIR/upgrade_mgr.log 2>&1 &

wait
