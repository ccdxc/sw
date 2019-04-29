#! /bin/bash

TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export WS_TOP="/sw"
export NICDIR=`dirname $ABS_TOOLS_DIR`
export TOPDIR=$NICDIR
export HAL_LOG_DIR=${TOPDIR}
export CAPRI_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/iris/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results

export SNORT_EXPORT_DIR=${TOPDIR}/hal/third-party/snort3/export
export SNORT_LUA_PATH=${SNORT_EXPORT_DIR}/lua/
export LUA_PATH="${SNORT_EXPORT_DIR}/lua/?.lua;;"
export SNORT_DAQ_PATH=${SNORT_EXPORT_DIR}/x86_64/lib/
export HAL_CONFIG_PATH=${TOPDIR}/conf
export DISABLE_AGING=1

if [[ "$1" ==  --coveragerun ]]; then
    export COVFILE=${TOPDIR}/coverage/sim_bullseye_hal.cov
fi
PRELOADS=${BUILD_DIR}/lib/libp4pd_mock.so
HBMHASH_PRELOADS=${BUILD_DIR}/lib/libhbmhashp4pd_mock.so
function finish {
   ${TOPDIR}/tools/savelogs.sh
}
trap finish EXIT
set -ex
# PI gtests
export PATH=${PATH}:${BUILD_DIR}/bin
# Disabling appid_test as it is failing after integrating with memhash library
export CAPRI_MOCK_MEMORY_MODE=1
# Any test using memhash library requires CAPRI_MOCK_MEMORY_MODE
${CMD_OPTS} net_fwding_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/net_fwding_test.xml"
