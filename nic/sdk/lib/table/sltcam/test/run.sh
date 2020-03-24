#! /bin/bash
set -e
export TOOLDIR=`dirname $0`
export SDKDIR=`readlink -f $TOOLDIR/../../../..`
export BUILD_DIR=${SDKDIR}/build/x86_64/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1

# PI gtests
export PATH=${PATH}:${BUILD_DIR}/bin
LD_PRELOAD=${PRELOADS} $GDB sltcam_test $*
