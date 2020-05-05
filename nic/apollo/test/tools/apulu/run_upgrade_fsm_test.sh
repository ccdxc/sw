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
upg_setup $BUILD_DIR/gen/graceful_test.json

TEST_EXECUTOR=${PDSPKG_TOPDIR}/apollo/test/tools/apulu/fsm_test_executor.py
TEST_JSON=${PDSPKG_TOPDIR}/apollo/test/tools/apulu/upgrade_fsm_test_cases.json

if [ ! -z "${test_case}" ] ; then
    CMD="python -u ${TEST_EXECUTOR} -i ${TEST_JSON} -t ${test_case}"
else
    CMD="python -u ${TEST_EXECUTOR} -i ${TEST_JSON}"
fi

${CMD}
if [[ $? -ne 0 ]]; then
    echo "pdsupgmgr test failed"
    exit 1
else
    echo "pdsupgmgr test passed"
    exit 0
fi

