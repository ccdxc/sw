//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases
///
//----------------------------------------------------------------------------
#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/include/api/pds_batch.hpp"
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

//----------------------------------------------------------------------------
// VNIC test class
//----------------------------------------------------------------------------

class vnic_test : public pds_test_base {
protected:
    vnic_test() {}
    virtual ~vnic_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_cfg_file, false);
    }
};

//----------------------------------------------------------------------------
// VNIC test cases utility routines
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

/// \brief Create a VNIC
///
/// Create a VNIC and test the following
///
/// Configure both ipv4 and ipv6 addresses and run the packet test
///
/// Configure min to max of vlan ids and verify with packet
///
/// Configure varying range of mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_create) {
    pds_batch_params_t batch_params = {0};
    const char *my_ip = "100.1.1.1";
    const char *my_mac = "00:00:64:01:01:01";
    const char *my_gw = "100.1.1.2";
    std::string vcn_cidr = "aaaa:0001:0001::/48";
    std::string sub_cidr = "aaaa:0001:0001:0001::/64";
    pds_vnic_id_t vnic_id = 100;
    uint16_t vlan_tag = 100;
    uint16_t mpls_slot = 200;
    const char *vnic_mac = "00:00:0a:01:01:03";
    const char *vnic_ip = "aaaa:0001:0001:0001::0003";    // Part of the subnet
    const char *sub_vr_ip = "aaaa:0001:0001:0001::0001";
    const char *sub_vr_mac = "00:00:0a:01:01:01";
    std::string sub_rem_cidr = "aaaa:0001:0001:0002::/64";
    const char *sub_rem_gw = "100.0.0.4";    // Part of my-tep subnet
    pds_vcn_id_t vcn_id = 1;
    pds_subnet_id_t sub_id = 1;
    pds_route_table_id_t rt_id = 1;
    pds_route_table_spec_t rt_tbl;
    rule_t *rule;
    struct in_addr ipaddr;

    batch_params.epoch = 1;
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);

    // Create device
    device_util device(my_ip, my_mac, my_gw);
    ASSERT_TRUE(device.create() == SDK_RET_OK);

    // Create VCN
    vcn_util vcn(PDS_VCN_TYPE_TENANT, vcn_id, vcn_cidr);
    ASSERT_TRUE(vcn.create() == SDK_RET_OK);

   // Create remote tep
    tep_util rtep(sub_rem_gw, PDS_TEP_ENCAP_TYPE_VNIC);
    ASSERT_TRUE(rtep.create() == SDK_RET_OK);


    // Create ipv6 route for remote subnet
    rt_tbl.key.id = rt_id;
    rt_tbl.af = IP_AF_IPV6;
    rt_tbl.num_routes = 1;
    rt_tbl.routes = (pds_route_t *)calloc(1, sizeof(pds_route_t));
    ASSERT_TRUE(str2ipv6pfx((char *)sub_rem_cidr.c_str(),
                            &rt_tbl.routes[0].prefix) == SDK_RET_OK);
    rt_tbl.routes[0].nh_ip.af = IP_AF_IPV4;
    inet_aton(sub_rem_gw, &ipaddr);
    rt_tbl.routes[0].nh_ip.addr.v4_addr = ntohl(ipaddr.s_addr);
    rt_tbl.routes[0].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    rt_tbl.routes[0].vcn_id = PDS_VCN_ID_INVALID;
    ASSERT_TRUE(pds_route_table_create(&rt_tbl) == SDK_RET_OK);

    // Configure and ipv4 route.
    // TODO check with Sarat on the need for this
    rt_tbl.key.id = ++rt_id;
    rt_tbl.af = IP_AF_IPV4;
    rt_tbl.num_routes = 1;
    rt_tbl.routes = (pds_route_t *)calloc(1, sizeof(pds_route_t));
    ASSERT_TRUE(str2ipv4pfx((char *)"20.1.0.0/16",
                            &rt_tbl.routes[0].prefix) == SDK_RET_OK);
    rt_tbl.routes[0].nh_ip.af = IP_AF_IPV4;
    inet_aton(sub_rem_gw, &ipaddr);
    rt_tbl.routes[0].nh_ip.addr.v4_addr = ntohl(ipaddr.s_addr);
    rt_tbl.routes[0].nh_type = PDS_NH_TYPE_REMOTE_TEP;
    rt_tbl.routes[0].vcn_id = PDS_VCN_ID_INVALID;
    ASSERT_TRUE(pds_route_table_create(&rt_tbl) == SDK_RET_OK);

    // Create Subnet on the VCN
    subnet_util sub(vcn_id, sub_id, sub_cidr);
    sub.vr_ip = sub_vr_ip;
    sub.vr_mac = sub_vr_mac;
    sub.v6_route_table = 1;
    sub.v4_route_table = 2;
    ASSERT_TRUE(sub.create() == SDK_RET_OK);

    // Create vnic
    vnic_util vnic(vcn_id, vnic_id, sub_id, vnic_mac);
    vnic.vlan_tag = vlan_tag;
    vnic.mpls_slot = mpls_slot;
    vnic.rsc_pool_id = 0;
    ASSERT_TRUE(vnic.create() == SDK_RET_OK);

    // Completed the configuration
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// \brief Create many VNICs
///
/// Create max number of VNICs and test the following
///
/// Configure varying range of vlan tags & mpls slots and verify with packet
///
/// Get the vnic stats and compare the valid, drops/reason matches with the
/// expectation.
TEST_F(vnic_test, vnic_many_create) {}

/// \brief Get VNIC
///
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
// TODO : Enable this after fixing the read issue
TEST_F(vnic_test, DISABLED_vnic_get) {
    pds_vnic_info_t vnic_info;
    pds_vcn_id_t vcn_id = 1;
    pds_vnic_id_t vnic_id = 100;

    vnic_util vnic(vcn_id, vnic_id);
    // Read vnic info and compare the configuration packet count
    ASSERT_TRUE(vnic.read(&vnic_info) == SDK_RET_OK);
}

/// \brief VNIC source and destination check
///
/// Configure skip_src_dst_check to true/false and make sure the config by
/// sending both valid and invalid IP addresses.
///
/// Get the stats and makesure the drop count/reason are as expected
TEST_F(vnic_test, vnic_src_dst_check) {}


/// \brief Delete VNIC
///
/// Configure the vnic and make-sure the parameters are configured properly
/// by getting the configured values back
// TODO : Enable this after fixing the delete issue
TEST_F(vnic_test, DISABLED_vnic_delete) {

    pds_batch_params_t batch_params = {0};
    pds_vcn_id_t vcn_id = 1;
    pds_vnic_id_t vnic_id = 100;

    batch_params.epoch = 2;
    vnic_util vnic(vcn_id, vnic_id);
    ASSERT_TRUE(pds_batch_start(&batch_params) == SDK_RET_OK);
    ASSERT_TRUE(vnic.del() == SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == SDK_RET_OK);
}

/// @}
}    // namespace api_test

// print help message showing usage of HAL
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
