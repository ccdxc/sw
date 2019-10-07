#! /bin/bash

set -e
export NICDIR=`pwd`
export NON_PERSISTENT_LOGDIR=${NICDIR}
export ZMQ_SOC_DIR=${NICDIR}
export CAPRI_MOCK_MODE=1
export CAPRI_MOCK_MEMORY_MODE=1
export SKIP_VERIFY=1
export BUILD_DIR=${NICDIR}/build/x86_64/artemis/
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
}
trap finish EXIT

export PATH=${PATH}:${BUILD_DIR}/bin

# gtests
echo "Running device test"
$GDB artemis_device_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_device_test.xml" > artemis_device_test.log.txt
[[ $? -ne 0 ]] && echo "artemis_device_test failed!" && exit 1

echo "Running vpc peer test"
$GDB artemis_vpc_peer_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_vpc_peer_test.xml" > artemis_vpc_peer_test.log.txt
[[ $? -ne 0 ]] && echo "artemis_vpc_peer_test failed!" && exit 1

exit 0

#echo "Running tep test"
#$GDB artemis_tep_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_tep_test.xml" > artemis_tep_test.log.txt
#[[ $? -ne 0 ]] && echo "artemis_tep_test failed!" && exit 1

echo "Running nh test"
$GDB artemis_nh_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_nh_test.xml" > artemis_nh_test.log.txt
[[ $? -ne 0 ]] && echo "artemis_nh_test failed!" && exit 1

echo "Running route table test"
$GDB artemis_route_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_route_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "artemis_route_test failed!" && exit 1

echo "Running vnic test"
$GDB artemis_vnic_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_vnic_test.xml" > artemis_vnic_test.log.txt
[[ $? -ne 0 ]] && echo "artemis_vnic_test failed!" && exit 1

echo "Running svc mapping test"
$GDB artemis_svc_mapping_test -c hal.json --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_svc_mapping_test.xml" > artemis_svc_mapping_test.log.txt
[[ $? -ne 0 ]] && echo "artemis_svc_mapping_test failed!" && exit 1

echo "Running tag test"
$GDB artemis_tag_test -c hal.json -f artemis --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/artemis_tag_test.xml" > /dev/null
[[ $? -ne 0 ]] && echo "artemis_tag_test failed!" && exit 1

echo "Success"
