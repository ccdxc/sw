#! /bin/bash
export WS_TOP="/sw"
export TOPDIR="/local/vikasd/ap/src/github.com/pensando/sw/nic"
export HAL_LOG_DIR=${TOPDIR}
export CAPRI_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/iris/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${TOPDIR}/conf
export DISABLE_AGING=1
export COVFILE=${TOPDIR}/coverage/sim_bullseye_hal.cov


rm -f ${TOPDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi
PRELOADS=${BUILD_DIR}/lib/libp4pd_mock.so
MEMHASH_PRELOADS=${BUILD_DIR}/lib/libmemhashp4pd_mock.so
set -e
# PI gtests
export PATH=${PATH}:${BUILD_DIR}/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${TOPDIR}/build/x86_64/iris/lib
LD_PRELOAD=${MEMHASH_PRELOADS} $ARGS memhash_test $*
