#! /bin/bash

set -e
export NICDIR=`pwd`
export SDKDIR=${NICDIR}/sdk/
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/apollo/
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
echo "Running scale test"
$GDB apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_scale_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "apollo_scale_test failed!" && exit 1

echo "Running device test"
$GDB apollo_device_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_device_test.xml" > apollo_device_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_device_test failed!" && exit 1

echo "Running vpc test"
$GDB apollo_vpc_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_vpc_test.xml" > apollo_vpc_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_vpc_test failed!" && exit 1

echo "Running subnet test"
$GDB apollo_subnet_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_subnet_test.xml" > apollo_subnet_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_subnet_test failed!" && exit 1

echo "Running route test"
$GDB apollo_route_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_route_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "apollo_route_test failed!" && exit 1

echo "Running vnic test"
$GDB apollo_vnic_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_vnic_test.xml" > apollo_vnic_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_vnic_test failed!" && exit 1

echo "Running tep test"
$GDB apollo_tep_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_tep_test.xml" > apollo_tep_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_tep_test failed!" && exit 1

echo "Running mapping test"
$GDB apollo_mapping_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_mapping_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "apollo_mapping_test failed!" && exit 1

echo "Running mirror session test"
$GDB apollo_mirror_session_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/apollo_mirror_session_test.xml" > apollo_mirror_session_test.log.txt
[[ $? -ne 0 ]] && echo "apollo_mirror_session_test failed!" && exit 1
#valgrind --track-origins=yes --xml=yes --xml-file=out.xml apollo_scale_test -c hal.json -i ${NICDIR}/apollo/test/scale/scale_cfg.json

echo "Success"
