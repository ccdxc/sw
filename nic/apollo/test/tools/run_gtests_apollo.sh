#! /bin/bash

set -e
export NICDIR=`pwd`
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
   ${NICDIR}/apollo/test/tools/savelogs.sh
}
trap finish EXIT

export PATH=${PATH}:${BUILD_DIR}/bin

# gtests
$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml" > apollo_scale_test.log.txt
$GDB apollo_device_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_device_test.xml"
$GDB apollo_vpc_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_vpc_test.xml"
$GDB apollo_subnet_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_subnet_test.xml"
$GDB apollo_vnic_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_vnic_test.xml"
$GDB apollo_tep_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_tep_test.xml"
$GDB apollo_route_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_route_test.xml"
$GDB apollo_mapping_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_mapping_test.xml"
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json

MEMHASH_PRELOADS=${BUILD_DIR}/lib/libmemhashp4pd_mock.so
LD_PRELOAD=${MEMHASH_PRELOADS} $ARGS memhash_test > memhash_test.log.txt
