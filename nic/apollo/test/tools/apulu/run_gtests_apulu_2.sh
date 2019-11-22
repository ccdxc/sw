#! /bin/bash

set -e
export NICDIR=`pwd`
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apulu/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
#export GDB='gdb --args'

if [[ "$1" ==  --coveragerun ]]; then
    export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
fi

function finish {
   echo "===== Collecting logs ====="
   ${NICDIR}/apollo/test/tools/savelogs.sh
   rm -f ${NICDIR}/conf/pipeline.json
}
trap finish EXIT

export PATH=${PATH}:${BUILD_DIR}/bin

rm -f ${NICDIR}/conf/pipeline.json
ln -s ${NICDIR}/conf/apulu/pipeline.json ${NICDIR}/conf/pipeline.json

# gtests
echo "Running policy test"
$GDB apulu_policy_test -c hal.json -n 1024 --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_policy_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "apulu_policy_test failed!" && exit 1

#echo "Running rfc rte bitmap test"
#$GDB apulu_rte_bitmap_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_rte_bitmap_test.xml" > apulu_rte_bitmap_test_log.txt
#[[ $? -ne 0 ]] && echo "apulu_rte_bitmap_test failed!" && exit 1

echo "Success"
