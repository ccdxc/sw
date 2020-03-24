#! /bin/bash
export WS_TOP="/sw"
export TOPDIR="/sw/nic"
export NON_PERSISTENT_LOGDIR=${TOPDIR}
export ASIC_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/iris/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results

export SNORT_EXPORT_DIR=${TOPDIR}/hal/third-party/snort3/export
export SNORT_LUA_PATH=${SNORT_EXPORT_DIR}/lua/
export LUA_PATH="${SNORT_EXPORT_DIR}/lua/?.lua;;"
export SNORT_DAQ_PATH=${SNORT_EXPORT_DIR}/x86_64/lib/
export HAL_CONFIG_PATH=${TOPDIR}/conf
export DISABLE_AGING=1
export COVFILE=${TOPDIR}/coverage/sim_bullseye_hal.cov

export VAL_CMD=valgrind
export VALGRIND_DIR=${BUILD_DIR}/valgrind
export VAL_TR=${VALGRIND_DIR}/test_results
export VAL_INFO=${VALGRIND_DIR}/info

mkdir -p ${VAL_INFO}

if [[ "$1" ==  --coveragerun ]]; then
    CMD_OPTS="COVFILE\=${COVFILE}"
fi
PRELOADS=${BUILD_DIR}/lib/libp4pd_mock.so
HBMHASH_PRELOADS=${BUILD_DIR}/lib/libhbmhashp4pd_mock.so
function finish {
   ${TOPDIR}/tools/savelogs.sh
}
trap finish EXIT
set -e
# PI gtests
export PATH=${PATH}:${BUILD_DIR}/bin
${VAL_CMD} appid_test --xml=yes --xml-file=${VAL_INFO}/appid_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/appid_test.xml"
${VAL_CMD} port_test --xml=yes  --xml-file=${VAL_INFO}/port_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/port_test.xml"
${VAL_CMD} ipsec_policy_test --xml=yes  --xml-file=${VAL_INFO}/ipsec_policy_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/ipsec_policy_test.xml"
${VAL_CMD} tcp_proxy_policy_test --xml=yes  --xml-file=${VAL_INFO}/tcp_proxy_policy_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tcp_proxy_policy_test.xml"
${VAL_CMD} acl_test --xml=yes  --xml-file=${VAL_INFO}/acl_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/acl_test.xml"
${VAL_CMD} bm_allocator_test --xml=yes  --xml-file=${VAL_INFO}/bm_allocator_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/bm_allocator_test.xml"
${VAL_CMD} enicif_test --xml=yes  --xml-file=${VAL_INFO}/enicif_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/enicif_test.xml"
${VAL_CMD} lif_manager_test --xml=yes  --xml-file=${VAL_INFO}/lif_manager_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/lif_manager_test.xml"
${VAL_CMD} lif_test --xml=yes  --xml-file=${VAL_INFO}/lif_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/lif_test.xml"
${VAL_CMD} session_test --xml=yes  --xml-file=${VAL_INFO}/session_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/session_test.xml"
${VAL_CMD} tlscb_test --xml=yes  --xml-file=${VAL_INFO}/tlscb_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tlscb_test.xml"
#${VAL_CMD} uplinkif_test --xml=yes  --xml-file=${VAL_INFO}/uplinkif_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/uplinkif_test.xml"
${VAL_CMD} uplinkpc_test --xml=yes  --xml-file=${VAL_INFO}/uplinkpc_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/uplinkpc_test.xml"
${VAL_CMD} internal_test --xml=yes  --xml-file=${VAL_INFO}/internal_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/internal_test.xml"
${VAL_CMD} endpoint_test --xml=yes  --xml-file=${VAL_INFO}/endpoint_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/endpoint_test.xml"
${VAL_CMD} network_test --xml=yes  --xml-file=${VAL_INFO}/network_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/network_test.xml"
${VAL_CMD} vrf_test --xml=yes  --xml-file=${VAL_INFO}/vrf_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/vrf_test.xml"
${VAL_CMD} l2seg_test --xml=yes  --xml-file=${VAL_INFO}/l2seg_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/l2seg_test.xml"
${VAL_CMD} ipsec_sa_test --xml=yes  --xml-file=${VAL_INFO}/ipsec_sa_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/ipsec_sa_test.xml"
${VAL_CMD} multicast_test --xml=yes  --xml-file=${VAL_INFO}/multicast_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/multicast_test.xml"
${VAL_CMD} tunnelif_test --xml=yes  --xml-file=${VAL_INFO}/tunnelif_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tunnelif_test.xml"
${VAL_CMD} nwsec_test --xml=yes  --xml-file=${VAL_INFO}/nwsec_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_test.xml"
${VAL_CMD} scheduler_tx_test --xml=yes  --xml-file=${VAL_INFO}/scheduler_tx_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/scheduler_tx_test.xml"
${VAL_CMD} qos_test --xml=yes  --xml-file=${VAL_INFO}/qos_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/qos_test.xml"
${VAL_CMD} copp_test --xml=yes  --xml-file=${VAL_INFO}/copp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/copp_test.xml"
${VAL_CMD} nh_test --xml=yes  --xml-file=${VAL_INFO}/nh_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nh_test.xml"
${VAL_CMD} route_test --xml=yes  --xml-file=${VAL_INFO}/route_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/route_test.xml"
${VAL_CMD} nat_pool_test --xml=yes  --xml-file=${VAL_INFO}/nat_pool_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nat_pool_test.xml"
${VAL_CMD} nat_policy_test --xml=yes  --xml-file=${VAL_INFO}/nat_policy_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nat_policy_test.xml"
${VAL_CMD} nwsec_prof_upgrade_test --xml=yes  --xml-file=${VAL_INFO}/nwsec_prof_upgrade_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_prof_upgrade_test.xml"
${VAL_CMD} vrf_upgrade_test --xml=yes  --xml-file=${VAL_INFO}/vrf_upgrade_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/vrf_upgrade_test.xml"
${VAL_CMD} filter_test --xml=yes  --xml-file=${VAL_INFO}/filter_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/filter_test.xml"
${VAL_CMD} mgmt_test --xml=yes  --xml-file=${VAL_INFO}/mgmt_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/mgmt_test.xml"

#${VAL_CMD} fmiss_learn_test --xml=yes  --xml-file=${VAL_INFO}/fmiss_learn_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/fmiss_learn_test.xml"
#${VAL_CMD} arp_test --xml=yes  --xml-file=${VAL_INFO}/arp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/arp_test.xml"
#${VAL_CMD} dhcp_test --xml=yes  --xml-file=${VAL_INFO}/dhcp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/dhcp_test.xml"
#${VAL_CMD} learn_mix_test --xml=yes  --xml-file=${VAL_INFO}/learn_mix_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/learn_mix_test.xml"
${VAL_CMD} alg_utils_test --xml=yes  --xml-file=${VAL_INFO}/alg_utils_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_utils_test.xml"
${VAL_CMD} alg_rpc_test --xml=yes  --xml-file=${VAL_INFO}/alg_rpc_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_rpc_test.xml"
${VAL_CMD} alg_tftp_test --xml=yes  --xml-file=${VAL_INFO}/alg_tftp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_tftp_test.xml"
${VAL_CMD} alg_ftp_test --xml=yes  --xml-file=${VAL_INFO}/alg_ftp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_ftp_test.xml"
${VAL_CMD} alg_rtsp_test --xml=yes  --xml-file=${VAL_INFO}/alg_rtsp_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_rtsp_test.xml"
${VAL_CMD} alg_dns_test --xml=yes  --xml-file=${VAL_INFO}/alg_dns_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_dns_test.xml"
#${VAL_CMD} nwsec_policy_test --xml=yes  --xml-file=${VAL_INFO}/nwsec_policy_val.xml nwsec_policy_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_policy_test.xml"
${VAL_CMD} network_test --xml=yes  --xml-file=${VAL_INFO}/network_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/network_test.xml"
${VAL_CMD} fte_test --xml=yes  --xml-file=${VAL_INFO}/fte_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/fte_test.xml"
${VAL_CMD} agent_api_test --xml=yes  --xml-file=${VAL_INFO}/agent_api_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/agent_api_test.xml"
${VAL_CMD} events_queue_test --xml=yes  --xml-file=${VAL_INFO}/events_queue_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/events_queue_test.xml"
${VAL_CMD} events_recorder_test --xml=yes  --xml-file=${VAL_INFO}/events_recorder_val.xml --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/events_recorder_test.xml"
