#! /bin/bash

export WS_TOP="/sw"
export TOPDIR="/sw/nic"
export HAL_LOG_DIR=${TOPDIR}
export CAPRI_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/apollo/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${TOPDIR}/conf

if [[ "$1" ==  --coveragerun ]]; then
    export COVFILE=${TOPDIR}/coverage/sim_bullseye_hal.cov
fi

function finish {
   ${TOPDIR}/tools/savelogs.sh
}
trap finish EXIT

set -e
export PATH=${PATH}:${BUILD_DIR}/bin
${CMD_OPTS} vcn_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/vcn_test.xml"
