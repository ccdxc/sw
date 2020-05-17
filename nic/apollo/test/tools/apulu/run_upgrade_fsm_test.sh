#! /bin/bash

######################################################################
# Usage :
#    $ sh /<PATH>/run_upgrade_fsm_test.sh
#               or
#    $ sh /<PATH>/run_upgrade_fsm_test.sh --test-case <test case id>
#####################################################################

ARGUMENT_LIST=(
    "test-case"
)

# read arguments
opts=$(getopt \
    --longoptions "$(printf "%s:," "${ARGUMENT_LIST[@]}")" \
    --name "$(basename "$0")" \
    --options "" \
    -- "$@"
)

eval set --$opts
test_case=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --test-case)
            test_case=$2
            shift 2
            ;;
        *)
            break
            ;;
    esac
done

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ))
source $CUR_DIR/../../../tools/setup_env_mock.sh $PIPELINE
source $CUR_DIR/../setup_upgrade_gtests.sh

# cleanup old run files
rm -rf /update/*

function trap_finish () {
    kill_process "fsm_test -s"
    kill_process "pdsupgmgr"
}

function graceful_test() {
    upg_setup $BUILD_DIR/gen/graceful_test.json upgrade_graceful.json

    TEST_EXECUTOR=${PDSPKG_TOPDIR}/apollo/test/upgrade/fsm/upgrade_fsm_test_exec.py
    TEST_JSON=${PDSPKG_TOPDIR}/apollo/test/upgrade/fsm/upgrade_fsm_test_cases.json

    if [ ! -z "${test_case}" ] ; then
        CMD="python -u ${TEST_EXECUTOR} -i ${TEST_JSON} -t ${test_case}"
    else
        CMD="python -u ${TEST_EXECUTOR} -i ${TEST_JSON}"
    fi

    ${CMD}
    if [[ $? -ne 0 ]]; then
        echo "pdsupgmgr test failed"
        return 1
    else
        echo "pdsupgmgr test passed"
        return 0
    fi
}

function hitless_test() {
    source $PDSPKG_TOPDIR/sdk/upgrade/core/upgmgr_core_base.sh
    upg_setup $BUILD_DIR/gen/hitless_test.json upgrade_hitless.json
    upg_operd_init
    upgmgr_clear_init_mode
    trap trap_finish EXIT
    $BUILD_DIR/bin/fsm_test -s svc1 -i 60  2>&1 &
    sleep 2
    $BUILD_DIR/bin/fsm_test -s svc2 -i 61 2>&1 &
    sleep 2
    $BUILD_DIR/bin/pdsupgmgr -t $PDSPKG_TOPDIR/sdk/test/upgrade/fsm/scripts > upgrade_doma.log 2>&1 &
    sleep 2
    # create another upgmgr for hitless
    upgmgr_set_init_mode "hitless"
    upgmgr_set_init_domain $CONFIG_PATH $UPGRADE_DOMAIN_B
    $BUILD_DIR/bin/fsm_test -s svc1 -i 58 2>&1 &
    sleep 2
    $BUILD_DIR/bin/fsm_test -s svc2 -i 59 2>&1 &
    sleep 2
    $BUILD_DIR/bin/pdsupgmgr -t $PDSPKG_TOPDIR/sdk/test/upgrade/fsm/scripts > upgrade_domb.log 2>&1 &
    sleep 5
    # run client
    pdsupgclient -m "hitless"
    [[ $? -ne 0 ]] && echo "upgrade command failed" && exit 1
    return 0
}

# graceful test exits its processes
graceful_test
[[ $? -ne 0 ]] && echo "upgrade graceful test failed" && exit 1
#sleep 5

#hitless_test
#[[ $? -ne 0 ]] && echo "upgrade hitless test failed" && exit 1
exit 0
