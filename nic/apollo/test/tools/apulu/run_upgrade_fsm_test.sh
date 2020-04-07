#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ))
source $CUR_DIR/../setup_upgrade_gtests.sh $PIPELINE

function trap_finish () {
    pkill fsm_test
    pkill pdsupgmgr
    finish upgmgr
}

setup
trap trap_finish EXIT

# start test services
$PDSPKG_TOPDIR/sdk/test/upgrade/fsm/start_services.sh $BUILD_DIR/bin &
sleep 2

# start upgrade manager
mkdir -p $CONFIG_PATH/gen/
rm -rf $CONFIG_PATH/gen/upgrade*.json
cp $BUILD_DIR/gen/graceful_test.json $CONFIG_PATH/gen/upgrade_graceful.json
$PDSPKG_TOPDIR/apollo/tools/apulu/start_upgmgr_mock.sh 1 &
sleep 2

# run client
pdsupgclient
[[ $? -ne 0 ]] && echo "pdsupgmgr test failed" && exit 1
echo "pdsupgmgr test passed"
exit 0
