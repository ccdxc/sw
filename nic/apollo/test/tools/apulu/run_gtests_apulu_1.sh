#! /bin/bash

set -e
export NICDIR=`pwd`
export SDKDIR=${NICDIR}/sdk/
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apulu/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results
export HAL_CONFIG_PATH=${NICDIR}/conf
export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
# export GDB='gdb --args'

function finish {
   echo "===== Collecting logs ====="
   ${NICDIR}/apollo/test/tools/savelogs.sh
}
trap finish EXIT

export PATH=${PATH}:${BUILD_DIR}/bin

if [[ "$1" ==  --coveragerun ]]; then
    export COVFILE=${NICDIR}/coverage/sim_bullseye_hal.cov
    # Run sdk tests for code coverage
    ${SDKDIR}/tools/run_sdk_gtests.sh
    [[ $? -ne 0 ]] && echo "sdk gtest failed" && exit 1
fi

# gtests
#echo "Running scale test"
#$GDB apulu_scale_test -c hal.json -i ${NICDIR}/apulu/test/scale/scale_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_scale_test.xml" > /dev/null
#[[ $? -ne 0 ]] && echo "apulu_scale_test failed!" && exit 1

echo "Running device test"
$GDB apulu_device_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_device_test.xml" > apulu_device_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_device_test failed!" && exit 1

echo "Running vpc test"
$GDB apulu_vpc_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_vpc_test.xml" > apulu_vpc_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_vpc_test failed!" && exit 1

echo "Running subnet test"
$GDB apulu_subnet_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_subnet_test.xml" > apulu_subnet_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_subnet_test failed!" && exit 1

#echo "Running route test"
#$GDB apulu_route_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_route_test.xml" > /dev/null
#[[ $? -ne 0 ]] && echo "apulu_route_test failed!" && exit 1

echo "Running nexthop test"
$GDB apulu_nh_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_nh_test.xml" > apulu_nh_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_nh_test failed!" && exit 1

echo "Running vnic test"
$GDB apulu_vnic_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_vnic_test.xml" > apulu_vnic_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_vnic_test failed!" && exit 1

echo "Running tep test"
$GDB apulu_tep_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_tep_test.xml" > apulu_tep_test.log.txt
[[ $? -ne 0 ]] && echo "apulu_tep_test failed!" && exit 1

#echo "Running mapping test"
#$GDB apulu_mapping_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_mapping_test.xml" > /dev/null
#[[ $? -ne 0 ]] && echo "apulu_mapping_test failed!" && exit 1

#echo "Running mirror session test"
#$GDB apulu_mirror_session_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apulu_mirror_session_test.xml" > apulu_mirror_session_test.log.txt
#[[ $? -ne 0 ]] && echo "apulu_mirror_session_test failed!" && exit 1
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apulu_scale_test -c hal.json -i ${NICDIR}/apulu/test/scale/scale_cfg.json

echo "Success"
