#! /bin/bash

export PIPELINE=apulu
CUR_DIR=$( readlink -f $( dirname $0 ))
source $CUR_DIR/../../../tools/setup_env_mock.sh $PIPELINE
source $CUR_DIR/../setup_upgrade_gtests.sh

TEST_EXECUTOR=${PDSPKG_TOPDIR}/apollo/test/tools/apulu/fsm_test_executor.py
TEST_JSON=${PDSPKG_TOPDIR}/apollo/test/tools/apulu/upgrade_fsm_test_cases.json

python -u ${TEST_EXECUTOR} -i ${TEST_JSON}
if [[ $? -ne 0 ]]; then
    echo "pdsupgmgr test failed"
    exit 1
else
    echo "pdsupgmgr test passed"
    exit 0
fi
