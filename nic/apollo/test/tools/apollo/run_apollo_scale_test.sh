#! /bin/bash

set -ex
export NICDIR=`pwd`
export ASIC="${ASIC:-capri}"
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export ASIC_MOCK_MODE=1
export ASIC_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apollo/${ASIC}
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export CONFIG_PATH=${NICDIR}/conf
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
ln -s ${NICDIR}/conf/apollo/pipeline.json ${NICDIR}/conf/pipeline.json
# gtests
#$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/lite_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_lite_test.xml"

$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml"
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json
