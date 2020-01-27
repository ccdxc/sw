#! /bin/bash

set -e

export NICDIR=`pwd`
export PIPELINE=apulu

export SDKDIR=${NICDIR}/sdk/
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export VPP_IPC_MOCK_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/${PIPELINE}/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
export PATH=${PATH}:${BUILD_DIR}/bin
export VAL_CMD=valgrind
#export GDB='gdb --args'

function finish () {
    # auto invoked on any exit
    echo "===== Collecting logs ====="
    ${NICDIR}/apollo/test/tools/savelogs.sh
    rm -f ${NICDIR}/conf/pipeline.json
}
trap finish EXIT

function setup () {
    rm -f ${NICDIR}/*log* ${NICDIR}/core*
    rm -f ${NICDIR}/conf/pipeline.json
    ln -s ${NICDIR}/conf/${PIPELINE}/pipeline.json ${NICDIR}/conf/pipeline.json
}

setup
$BUILD_DIR/bin/upg_func_test -c hal.json
[[ $? -ne 0 ]] && echo "upg_func_test failed!" && exit 1

exit 0
