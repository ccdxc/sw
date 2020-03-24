#! /bin/bash
TOOLS_DIR=`dirname $0`
ABS_TOOLS_DIR=`readlink -f $TOOLS_DIR`
export WS_TOP="/sw"
export NICDIR=`dirname $ABS_TOOLS_DIR`
export TOPDIR=$NICDIR
export NON_PERSISTENT_LOGDIR=${TOPDIR}
export ASIC_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/apollo
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${TOPDIR}/conf
export DISABLE_AGING=1
export COVFILE=${TOPDIR}/coverage/sim_bullseye_hal.cov

rm -f ${TOPDIR}/core.*

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi
MEMHASH_PRELOADS=${BUILD_DIR}/lib/libmemhashp4pd_mock.so
set -e
# PI gtests
export PATH=${PATH}:${BUILD_DIR}/bin
LD_PRELOAD=${MEMHASH_PRELOADS} $ARGS memhash_test $*
