#! /bin/bash
export WS_TOP="/sw"
export TOPDIR="/sw/nic"
export HAL_LOG_DIR=${TOPDIR}
export CAPRI_MOCK_MODE=1
export BUILD_DIR=${TOPDIR}/build/x86_64/iris/
export GEN_TEST_RESULTS_DIR=${BUILD_DIR}/gtest_results

export SNORT_EXPORT_DIR=${TOPDIR}/hal/third-party/snort3/export
export SNORT_LUA_PATH=${SNORT_EXPORT_DIR}/lua/
export LUA_PATH="${SNORT_EXPORT_DIR}/lua/?.lua;;"
export SNORT_DAQ_PATH=${SNORT_EXPORT_DIR}/x86_64/lib/
export HAL_CONFIG_PATH=${TOPDIR}/conf
export DISABLE_AGING=1
export COVFILE=${TOPDIR}/coverage/bullseye_hal.cov

export VAL_CMD = valgrind --xml=yes
export VAL_CMD_OPTS = $(CMD_OPTS) $(VAL_CMD) 
export VALGRIND_DIR= $(BUILD_DIR)/valgrind
export VAL_TR = $(VALGRIND_DIR)/test_results
export VAL_INFO = $(VALGRIND_DIR)/info

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
${VAL_CMD_OPTS} appid_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/appid_test.xml" --xml-file=$(VAL_INFO)/appid_val.xml
${VAL_CMD_OPTS} port_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/port_test.xml"  --xml-file=$(VAL_INFO)/port_val.xml
${VAL_CMD_OPTS} ipsec_policy_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/ipsec_policy_test.xml"  --xml-file=$(VAL_INFO)/ipsec_policy_val.xml
${VAL_CMD_OPTS} tcp_proxy_policy_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tcp_proxy_policy_test.xml"  --xml-file=$(VAL_INFO)/tcp_proxy_policy_val.xml
${VAL_CMD_OPTS} acl_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/acl_test.xml"  --xml-file=$(VAL_INFO)/acl_val.xml
${VAL_CMD_OPTS} bm_allocator_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/bm_allocator_test.xml"  --xml-file=$(VAL_INFO)/bm_allocator_val.xml
${VAL_CMD_OPTS} enicif_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/enicif_test.xml"  --xml-file=$(VAL_INFO)/enicif_val.xml
${VAL_CMD_OPTS} lif_manager_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/lif_manager_test.xml"  --xml-file=$(VAL_INFO)/lif_manager_val.xml
${VAL_CMD_OPTS} lif_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/lif_test.xml"  --xml-file=$(VAL_INFO)/lif_val.xml
${VAL_CMD_OPTS} session_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/session_test.xml"  --xml-file=$(VAL_INFO)/session_val.xml
${VAL_CMD_OPTS} tlscb_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tlscb_test.xml"  --xml-file=$(VAL_INFO)/tlscb_val.xml
#${VAL_CMD_OPTS} uplinkif_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/uplinkif_test.xml"  --xml-file=$(VAL_INFO)/uplinkif_val.xml
${VAL_CMD_OPTS} uplinkpc_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/uplinkpc_test.xml"  --xml-file=$(VAL_INFO)/uplinkpc_val.xml
${VAL_CMD_OPTS} internal_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/internal_test.xml"  --xml-file=$(VAL_INFO)/internal_val.xml
${VAL_CMD_OPTS} endpoint_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/endpoint_test.xml"  --xml-file=$(VAL_INFO)/endpoint_val.xml
${VAL_CMD_OPTS} network_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/network_test.xml"  --xml-file=$(VAL_INFO)/network_val.xml
${VAL_CMD_OPTS} vrf_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/vrf_test.xml"  --xml-file=$(VAL_INFO)/vrf_val.xml
${VAL_CMD_OPTS} l2seg_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/l2seg_test.xml"  --xml-file=$(VAL_INFO)/l2seg_val.xml
${VAL_CMD_OPTS} ipsec_sa_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/ipsec_sa_test.xml"  --xml-file=$(VAL_INFO)/ipsec_sa_val.xml
${VAL_CMD_OPTS} multicast_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/multicast_test.xml"  --xml-file=$(VAL_INFO)/multicast_val.xml
${VAL_CMD_OPTS} tunnelif_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/tunnelif_test.xml"  --xml-file=$(VAL_INFO)/tunnelif_val.xml
${VAL_CMD_OPTS} nwsec_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_test.xml"  --xml-file=$(VAL_INFO)/nwsec_val.xml
${VAL_CMD_OPTS} scheduler_tx_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/scheduler_tx_test.xml"  --xml-file=$(VAL_INFO)/scheduler_tx_val.xml
${VAL_CMD_OPTS} qos_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/qos_test.xml"  --xml-file=$(VAL_INFO)/qos_val.xml
${VAL_CMD_OPTS} copp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/copp_test.xml"  --xml-file=$(VAL_INFO)/copp_val.xml
${VAL_CMD_OPTS} nh_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nh_test.xml"  --xml-file=$(VAL_INFO)/nh_val.xml
${VAL_CMD_OPTS} route_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/route_test.xml"  --xml-file=$(VAL_INFO)/route_val.xml
${VAL_CMD_OPTS} nat_pool_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nat_pool_test.xml"  --xml-file=$(VAL_INFO)/nat_pool_val.xml
${VAL_CMD_OPTS} nat_policy_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nat_policy_test.xml"  --xml-file=$(VAL_INFO)/nat_policy_val.xml
${VAL_CMD_OPTS} nwsec_prof_upgrade_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_prof_upgrade_test.xml"  --xml-file=$(VAL_INFO)/nwsec_prof_upgrade_val.xml
${VAL_CMD_OPTS} vrf_upgrade_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/vrf_upgrade_test.xml"  --xml-file=$(VAL_INFO)/vrf_upgrade_val.xml
${VAL_CMD_OPTS} filter_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/filter_test.xml"  --xml-file=$(VAL_INFO)/filter_val.xml
${VAL_CMD_OPTS} mgmt_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/mgmt_test.xml"  --xml-file=$(VAL_INFO)/mgmt_val.xml

#${VAL_CMD_OPTS} fmiss_learn_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/fmiss_learn_test.xml"  --xml-file=$(VAL_INFO)/fmiss_learn_val.xml
#${VAL_CMD_OPTS} arp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/arp_test.xml"  --xml-file=$(VAL_INFO)/arp_val.xml
#${VAL_CMD_OPTS} dhcp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/dhcp_test.xml"  --xml-file=$(VAL_INFO)/dhcp_val.xml
#${VAL_CMD_OPTS} learn_mix_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/learn_mix_test.xml"  --xml-file=$(VAL_INFO)/learn_mix_val.xml
${VAL_CMD_OPTS} alg_utils_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_utils_test.xml"  --xml-file=$(VAL_INFO)/alg_utils_val.xml
${VAL_CMD_OPTS} alg_rpc_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_rpc_test.xml"  --xml-file=$(VAL_INFO)/alg_rpc_val.xml
${VAL_CMD_OPTS} alg_tftp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_tftp_test.xml"  --xml-file=$(VAL_INFO)/alg_tftp_val.xml
${VAL_CMD_OPTS} alg_ftp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_ftp_test.xml"  --xml-file=$(VAL_INFO)/alg_ftp_val.xml
${VAL_CMD_OPTS} alg_rtsp_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_rtsp_test.xml"  --xml-file=$(VAL_INFO)/alg_rtsp_val.xml
${VAL_CMD_OPTS} alg_dns_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/alg_dns_test.xml"  --xml-file=$(VAL_INFO)/alg_dns_val.xml
#${VAL_CMD_OPTS} nwsec_policy_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/nwsec_policy_test.xml"  --xml-file=$(VAL_INFO)/nwsec_policy_val.xml
${VAL_CMD_OPTS} network_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/network_test.xml"  --xml-file=$(VAL_INFO)/network_val.xml
${VAL_CMD_OPTS} fte_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/fte_test.xml"  --xml-file=$(VAL_INFO)/fte_val.xml
${VAL_CMD_OPTS} agent_api_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/agent_api_test.xml"  --xml-file=$(VAL_INFO)/agent_api_val.xml
${VAL_CMD_OPTS} events_queue_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/events_queue_test.xml"  --xml-file=$(VAL_INFO)/events_queue_val.xml
${VAL_CMD_OPTS} events_recorder_test --gtest_output="xml:${GEN_TEST_RESULTS_DIR}/events_recorder_test.xml"  --xml-file=$(VAL_INFO)/events_recorder_val.xml
