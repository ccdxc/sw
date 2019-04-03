//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test cases
///
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/mapping.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

const char *g_cfg_file = "hal.json";

namespace api_test {

uint8_t g_snd_pkt1[] = {
    0x00, 0x00, 0x14, 0x01, 0x01, 0x03, 0x00, 0x00, 0x0a, 0x01, 0x01,
    0x03, 0x81, 0x00, 0x00, 0x64, 0x08, 0x00, 0x45, 0x00, 0x00, 0x5c,
    0x00, 0x01, 0x00, 0x00, 0x40, 0x06, 0x5a, 0x94, 0x0a, 0x01, 0x01,
    0x03, 0x14, 0x01, 0x01, 0x03, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02, 0x20, 0x00, 0xe9,
    0xc3, 0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x6c, 0x6b, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
    0x74, 0x75, 0x76, 0x77, 0x7a, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6c, 0x6b, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7a, 0x78, 0x79};

uint8_t g_rcv_pkt1[] = {
    0x00, 0x02, 0x0B, 0x0A, 0x0D, 0x0E, 0x00, 0x00, 0x64, 0x01, 0x01, 0x01,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,
    0x00, 0x00, 0x64, 0x01, 0x01, 0x01, 0x64, 0x00, 0x00, 0x04, 0x15, 0xDE,
    0x19, 0xEB, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x0C, 0x80, 0x00, 0x00, 0x19,
    0x01, 0x00, 0x45, 0x00, 0x00, 0x5C, 0x00, 0x01, 0x00, 0x00, 0x40, 0x06,
    0x5A, 0x94, 0x0A, 0x01, 0x01, 0x03, 0x14, 0x01, 0x01, 0x03, 0x12, 0x34,
    0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02,
    0x20, 0x00, 0xE9, 0xC3, 0x00, 0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79};

//----------------------------------------------------------------------------
// Mapping test class
//----------------------------------------------------------------------------

class mapping_test : public pds_test_base {
protected:
    mapping_test() {}
    virtual ~mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);
    }
};

//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup Mapping
/// @{

/// \brief Mapping create test
/// Create a local and remote IPV4 & V6 mapping and test the below scenarios
/// with bidirectional traffic.
/// Packets having valid(configured) src and dst ipaddress from host to host,
/// host to switch and vice-versa
/// Packets having invalid(non-configured) src and dst ipaddresses from host to
/// host, host to switch and vice-versa
/// Get the stats and make sure the drop counts/with-reason is matching
TEST_F(mapping_test, mapping_create) {
    pds_batch_params_t batch_params = {0};
    struct in_addr ipaddr;
    pds_vcn_id_t vcn_id = 1;
    pds_subnet_id_t sub_id = 1;
    pds_route_table_id_t rt_id = 1;
    pds_vnic_id_t vnic_id = 1;
    uint16_t vlan_tag = 100;
    uint16_t mpls_slot = 200;
    uint16_t mpls_rem_slot = 400;
    const char *vnic_mac = "00:00:0a:01:01:03";
    std::string vcn_cidr = "10.0.0.0/8";
    std::string sub_cidr = "10.1.0.0/16";
    const char *sub_vr_ip = "10.1.1.1";
    const char *sub_vr_mac = "00:00:0a:01:01:01";
    const char *vnic_ip = "10.1.1.3";    // Part of the subnet
    const char *my_ip = "100.1.1.1";
    const char *my_mac = "00:00:64:01:01:01";
    const char *my_gw = "100.1.1.2";
    std::string sub_rem_cidr = "20.1.0.0/16";
    const char *vnic_rem_ip = "20.1.1.3";
    const char *vnic_rem_mac = "00:00:14:01:01:03";
    const char *sub_rem_gw = "100.0.0.4";    // Part of my-tep subnet
    pds_mapping_info_t map_info;
    pds_route_table_spec_t rt_tbl;
    pds_policy_spec_t policy;
    rule_t *rule;
    uint32_t policy_id = 1, ingress = 1, num_rules = 1;

    batch_params.epoch = 1;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    // Create device
    device_util device(my_ip, my_mac, my_gw);
    ASSERT_TRUE(device.create() == SDK_RET_OK);

    // Create VCN
    vcn_util vcn(PDS_VCN_TYPE_TENANT, vcn_id, vcn_cidr);
    ASSERT_TRUE(vcn.create() == SDK_RET_OK);

    // Create local tep
    pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
    tep_util mytep(my_ip, PDS_TEP_TYPE_WORKLOAD, encap);
    ASSERT_TRUE(mytep.create() == SDK_RET_OK);

    // Create remote tep
    tep_util rtep(sub_rem_gw, PDS_TEP_TYPE_WORKLOAD, encap);
    ASSERT_TRUE(rtep.create() == SDK_RET_OK);

    // Create route
    rt_tbl.key.id = rt_id;
    rt_tbl.af = IP_AF_IPV4;
    rt_tbl.num_routes = 1;
    rt_tbl.routes = (pds_route_t *)calloc(1, sizeof(pds_route_t));
    ASSERT_TRUE(str2ipv4pfx((char *)sub_rem_cidr.c_str(),
                            &rt_tbl.routes[0].prefix) == SDK_RET_OK);
    rt_tbl.routes[0].nh_ip.af = IP_AF_IPV4;
    inet_aton(sub_rem_gw, &ipaddr);
    rt_tbl.routes[0].nh_ip.addr.v4_addr = ntohl(ipaddr.s_addr);
    rt_tbl.routes[0].nh_type = PDS_NH_TYPE_TEP;
    ASSERT_TRUE(pds_route_table_create(&rt_tbl) == SDK_RET_OK);

    // Create Subnet on the VCN
    subnet_util sub(vcn_id, sub_id, sub_cidr);
    sub.vr_ip = sub_vr_ip;
    sub.vr_mac = sub_vr_mac;
    sub.v4_route_table.id = rt_id;
    ASSERT_TRUE(sub.create() == SDK_RET_OK);

    // Create vnic
    vnic_util vnic(vcn_id, vnic_id, sub_id, vnic_mac);
    vnic.vlan_tag = vlan_tag;
    vnic.mpls_slot = mpls_slot;
    vnic.rsc_pool_id = 0;
    ASSERT_TRUE(vnic.create() == SDK_RET_OK);

    // Create Local Mappings
    mapping_util lmap(vcn_id, vnic_ip, vnic_id, vnic_mac);
    lmap.sub_id = sub_id;
    lmap.mpls_slot = mpls_slot;
    lmap.tep_ip = my_ip;
    // TODO public IP
    ASSERT_TRUE(lmap.create() == SDK_RET_OK);

    // Create Remote mapping
    mapping_util rmap(vcn_id, vnic_rem_ip, vnic_rem_mac);
    rmap.sub_id = sub_id;
    rmap.mpls_slot = mpls_rem_slot;
    rmap.tep_ip = sub_rem_gw;
    ASSERT_TRUE(rmap.create() == SDK_RET_OK);

#if 0
    // Allow all tcp traffic
policy_config:
    policy.policy_type = POLICY_TYPE_FIREWALL;
    policy.af = IP_AF_IPV4;
    policy.direction = ingress ? RULE_DIR_INGRESS : RULE_DIR_EGRESS;
    policy.num_rules = num_rules;
    policy.rules = (rule_t *)malloc(num_rules * sizeof(rule_t));
    memset(policy.rules, 0, num_rules * sizeof(rule_t));
    policy.key.id = policy_id++;
    rule = &policy.rules[0];
    rule->stateful = false;
    rule->match.l3_match.ip_proto = 0x6;    // TCP
    ASSERT_TRUE(str2ipv4pfx((char *)vcn_cidr.c_str(),
                            &rule->match.l3_match.ip_pfx) == SDK_RET_OK);
    rule->match.l4_match.sport_range.port_lo = 0;
    rule->match.l4_match.sport_range.port_hi = 65535;
    rule->match.l4_match.dport_range.port_lo = 0;
    rule->match.l4_match.dport_range.port_hi = 65535;
    rule->action_data.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
    ASSERT_TRUE(pds_policy_create(&policy) == SDK_RET_OK);
    // Apply egress rule for the same configuration
    if (ingress == 1)  {
        ingress = 0;
        goto policy_config;
    }
#endif
    // Completed the configuration
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);

#if 0
    api_test::send_packet(g_snd_pkt1, sizeof(g_snd_pkt1), TM_PORT_UPLINK_0, g_rcv_pkt1,
                          sizeof(g_rcv_pkt1), TM_PORT_UPLINK_1);
    exit_simulation();
#endif
}

/// \brief Mapping many create
/// Test the HASH overflow table is woking fine by creating max number of
/// mappings.
/// Test the above with traffic and compare the result. Chose the
/// IPs which will go to the overflow table.
TEST_F(mapping_test, mapping_many_create) {}

/// \brief Mapping get
/// Configure the mapping and make-sure the parameters are configured properly
/// by getting the configured values back
// TODO : Enable this after fixing the read issue
TEST_F(mapping_test, DISABLED_mapping_get) {
    const char *vnic_rem_ip = "20.1.1.3";
    pds_vcn_id_t vcn_id = 1;
    pds_vnic_id_t vnic_id = 1;
    pds_mapping_info_t map_info;
    const char *vnic_ip = "10.1.1.3";

    mapping_util rmap(vcn_id, vnic_rem_ip);
    mapping_util lmap(vcn_id, vnic_ip, vnic_id);

    // Read remote mapping info and compare the configuration
    ASSERT_TRUE(rmap.read(&map_info) == SDK_RET_OK);

    // Read local mapping info and compare the configuration.
    ASSERT_TRUE(lmap.read(&map_info) == SDK_RET_OK);

}

// \brief Mapping Delete
// Delete a single ipv4/v6 local/remote mapping and verify the change with
// traffic.
// Get the stats and makesure the drop count/reason are as expected
// Deletion while the traffic is going on . This has to be done in the Naples.
// There should not be any traffic hit.
// TODO : Enable this after fixing the delete issue
TEST_F(mapping_test, DISABLED_mapping_delete)
{
    pds_batch_params_t batch_params = {0};
    const char *vnic_rem_ip = "20.1.1.3";
    pds_vcn_id_t vcn_id = 1;
    const char *vnic_ip = "10.1.1.3";

    mapping_util rmap(vcn_id, vnic_rem_ip);
    mapping_util lmap(vcn_id, vnic_ip);
    batch_params.epoch = 2;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(rmap.del() == SDK_RET_OK);
    ASSERT_TRUE(lmap.del() == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

// \brief Mapping memory utilization
// Test max number of create followed by delete N number of times.
// Measure the memory utilization in each iteration and make sure it is not
// growing
// Run traffic intermittendely and make sure the data path is OK even
// after many creation and deletion.
TEST_F(mapping_test, mapping_memutil) {}


/// \brief Mapping Public
/// Configure public IP for each private and test the NAT scenarios,
/// Geneate traffic for fwd/reverse NAT faliures.
/// Check the stats and makesure the reason/drop count are expected.
TEST_F(mapping_test, mapping_nat) {}

/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

static inline void
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> \n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "hc:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        default:
            // ignore all other options
            break;
        }
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
