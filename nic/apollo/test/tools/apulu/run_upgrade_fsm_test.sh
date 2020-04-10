#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ))
source $CUR_DIR/../../../tools/setup_env_mock.sh $PIPELINE
source $CUR_DIR/../setup_upgrade_gtests.sh
upg_init

# override trap
function trap_finish () {
    pkill fsm_test
    pkill pdsupgmgr
    upg_finish upgmgr
}
trap trap_finish EXIT

# start test services
$PDSPKG_TOPDIR/sdk/test/upgrade/fsm/start_services.sh $BUILD_DIR/bin &
sleep 2

# start upgrade manager
upg_setup $BUILD_DIR/gen/graceful_test.json
echo "starting pdsupgmgr"
pdsupgmgr &
sleep 2

# run client
echo "starting pdsupgclient"
pdsupgclient
[[ $? -ne 0 ]] && echo "pdsupgmgr test failed" && exit 1
echo "pdsupgmgr test passed"
exit 0
