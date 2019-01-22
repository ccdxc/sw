#! /bin/bash

set -e
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export WS_TOP="/sw"
export NICDIR=`dirname $ABS_TOOLS_DIR`
export HAL_LOG_DIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apollo/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
#export GDB='gdb --args'

if [[ "$1" ==  --coveragerun ]]; then
    export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
fi

function finish {
   echo "===== Collecting logs ====="
   ${NICDIR}/tools/savelogs.sh
}
trap finish EXIT

export PATH=${PATH}:${BUILD_DIR}/bin
$GDB apollo_scale_test -c hal.json -i ${NICDIR}/hal/apollo/test/scale_test/scale_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apollo_scale_test -c hal.json -i ${NICDIR}/hal/apollo/test/scale_test/scale_cfg.json
