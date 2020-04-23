//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all vpc test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/workflow.hpp"
#include "nic/apollo/test/api/utils/utils.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"

namespace test {
namespace api {

// globals
static constexpr uint32_t k_max_vpc = PDS_MAX_VPC + 1;

//----------------------------------------------------------------------------
// VPC test class
//----------------------------------------------------------------------------

class vpc : public ::pds_test_base {
protected:
    vpc() {}
    virtual ~vpc() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VPC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VPC_TEST VPC Tests
/// @{

/// \brief VPC WF_B1
/// \ref WF_B1
TEST_F(vpc, vpc_workflow_b1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16");
    workflow_b1<vpc_feeder>(feeder);
}

/// \brief VPC WF_B2
/// \ref WF_B2
TEST_F(vpc, vpc_workflow_b2) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  1, "00:02:0A:00:00:01");
    workflow_b2<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_1
/// \ref WF_1
TEST_F(vpc, vpc_workflow_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_2
/// \ref WF_2
TEST_F(vpc, vpc_workflow_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_3
/// \ref WF_3
TEST_F(vpc, vpc_workflow_3) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "30.0.0.0/16",
                 20, "00:02:0A:00:00:01");
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "60.0.0.0/16",
                 20, "00:02:0A:0B:00:01");
    workflow_3<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_4
/// \ref WF_4
TEST_F(vpc, vpc_workflow_4) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", k_max_vpc);
    workflow_4<vpc_feeder>(feeder);
}

/// \brief VPC WF_5
/// \ref WF_5
TEST_F(vpc, vpc_workflow_5) {
    pds_obj_key_t key1 = int2pdsobjkey(10);
    pds_obj_key_t key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70);
    vpc_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16",
                 20, "00:02:0A:00:00:01");
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "70.0.0.0/16",
                 20, "00:02:0A:0B:00:01");
    workflow_5<vpc_feeder>(feeder1, feeder2, feeder3);
}

/// \brief VPC WF_6
/// \ref WF_6
TEST_F(vpc, vpc_workflow_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc, "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc, "00:02:0A:00:00:01");
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  k_max_vpc, "00:02:0A:0B:00:01");
    workflow_6<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_7
/// \ref WF_7
TEST_F(vpc, vpc_workflow_7) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc, "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc, "00:02:0A:00:00:01");
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  k_max_vpc, "00:02:0A:0B:00:01");
    workflow_7<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_8
/// \ref WF_8
TEST_F(vpc, vpc_workflow_8) {
    if (!apulu()) return;

    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A, feeder1B;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc, "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc, "00:02:0A:00:00:01");
    feeder1B.init(key, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  k_max_vpc, "00:02:0A:0B:00:01");
    workflow_8<vpc_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief VPC WF_9
/// \ref WF_9
TEST_F(vpc, vpc_workflow_9) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc, "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc, "00:02:0A:00:00:01");
    workflow_9<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_10
/// \ref WF_10
TEST_F(vpc, vpc_workflow_10) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    pds_obj_key_t key3 = int2pdsobjkey(70), key4 = int2pdsobjkey(100);
    vpc_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                 20, "00:02:0A:00:00:01");
    feeder2A.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                  20, "00:02:0A:0B:00:01");
    feeder3.init(key3, PDS_VPC_TYPE_TENANT, "13.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder3A.init(key3, PDS_VPC_TYPE_TENANT, "14.0.0.0/16",
                  20, "00:02:0A:00:00:01");
    feeder4.init(key4, PDS_VPC_TYPE_TENANT, "15.0.0.0/16",
                 20, "00:02:0A:0B:00:01");
    workflow_10<vpc_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief VPC WF_N_1
/// \ref WF_N_1
TEST_F(vpc, vpc_workflow_neg_1) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                k_max_vpc, "00:02:01:00:00:01");
    workflow_neg_1<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_2
/// \ref WF_N_2
TEST_F(vpc, DISABLED_vpc_workflow_neg_2) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                k_max_vpc + 1, "00:02:01:00:00:01");
    workflow_neg_2<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_3
/// \ref WF_N_3
TEST_F(vpc, vpc_workflow_neg_3) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder;

    feeder.init(key, PDS_VPC_TYPE_TENANT, "0.0.0.0/0",
                k_max_vpc, "00:02:01:00:00:01");
    workflow_neg_3<vpc_feeder>(feeder);
}

/// \brief VPC WF_N_4
/// \ref WF_N_4
TEST_F(vpc, vpc_workflow_neg_4) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "40.0.0.0/16",
                 20, "00:02:0A:00:00:01");
    workflow_neg_4<vpc_feeder>(feeder1, feeder2);
}

/// \brief VPC WF_N_5
/// \ref WF_N_5
TEST_F(vpc, vpc_workflow_neg_5) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc,  "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc, "00:02:0A:00:00:01");
    workflow_neg_5<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_6
/// \ref WF_N_6
TEST_F(vpc, vpc_workflow_neg_6) {
    pds_obj_key_t key = int2pdsobjkey(1);
    vpc_feeder feeder1, feeder1A;

    feeder1.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 k_max_vpc, "00:02:01:00:00:01");
    feeder1A.init(key, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  k_max_vpc + 1, "00:02:0A:00:00:01");
    workflow_neg_6<vpc_feeder>(feeder1, feeder1A);
}

/// \brief VPC WF_N_7
/// \ref WF_N_7
TEST_F(vpc, vpc_workflow_neg_7) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    vpc_feeder feeder1, feeder1A, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder1A.init(key1, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                  20, "00:02:0A:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "12.0.0.0/16",
                 20, "00:02:0A:0B:00:01");
    workflow_neg_7<vpc_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief VPC WF_N_8
/// \ref WF_N_8
TEST_F(vpc, vpc_workflow_neg_8) {
    pds_obj_key_t key1 = int2pdsobjkey(10), key2 = int2pdsobjkey(40);
    vpc_feeder feeder1, feeder2;

    feeder1.init(key1, PDS_VPC_TYPE_TENANT, "10.0.0.0/16",
                 20, "00:02:01:00:00:01");
    feeder2.init(key2, PDS_VPC_TYPE_TENANT, "11.0.0.0/16",
                 20, "00:02:0A:00:00:01");
    workflow_neg_8<vpc_feeder>(feeder1, feeder2);
}

//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------

/// \brief update type
TEST_F(vpc, vpc_update_type) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_UNDERLAY, "10.0.0.0/16");
    vpc_create(feeder);
    spec.type = PDS_VPC_TYPE_TENANT;
    // updating vpc type is not allowed
    vpc_update(feeder, &spec, VPC_ATTR_TYPE, SDK_RET_ERR);
    feeder.init(key, PDS_VPC_TYPE_UNDERLAY, "10.0.0.0/16");
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update v4 prefix
TEST_F(vpc, vpc_update_v4_prefix) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16");
    vpc_create(feeder);
    str2ipv4pfx((char *)"20.0.0.0/16", &spec.v4_prefix);
    vpc_update(feeder, &spec, VPC_ATTR_V4_PREFIX);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update v6 prefix
TEST_F(vpc, vpc_update_v6_prefix) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 1,
                "00:02:01:00:00:01", "2001::1/64");
    vpc_create(feeder);
    str2ipv6pfx((char *) "3001::1/64", &spec.v6_prefix);
    vpc_update(feeder, &spec, VPC_ATTR_V6_PREFIX);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update vr mac
TEST_F(vpc, vpc_update_vr_mac) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16");
    vpc_create(feeder);
    mac_str_to_addr((char *)"ba:de:de:ad:be:ef", spec.vr_mac);
    vpc_update(feeder, &spec, VPC_ATTR_VR_MAC);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update fabric encap
TEST_F(vpc, vpc_update_fabric_encap) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 1,
                "00:02:01:00:00:01", "", "VxLAN 100");
    vpc_create(feeder);
    pds_str2encap("VxLAN 200", &spec.fabric_encap);  // update encap value
    vpc_update(feeder, &spec, VPC_ATTR_FAB_ENCAP);
    vpc_read(feeder);

    // updating encap type is not allowed
    pds_str2encap("MPLSoUDP 100", &spec.fabric_encap);
    vpc_update(feeder, &spec, VPC_ATTR_FAB_ENCAP, SDK_RET_ERR);
    pds_str2encap("VxLAN 200", &feeder.spec.fabric_encap);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief upate route tables
TEST_F(vpc, vpc_update_rttbl) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 1,
                "ba:de:de:ad:be:ef", "1000::1/64", "VxLAN 9999",
                int2pdsobjkey(4000), int2pdsobjkey(6000));
    vpc_create(feeder);
    spec.v4_route_table = int2pdsobjkey(5000); // update v4 route table
    vpc_update(feeder, &spec, VPC_ATTR_V4_RTTBL);
    vpc_read(feeder);
    spec.v6_route_table = int2pdsobjkey(7000); // update v6 route table
    vpc_update(feeder, &spec, VPC_ATTR_V6_RTTBL);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief upate tos
TEST_F(vpc, vpc_update_tos) {
    if (!apulu()) return;

    vpc_feeder feeder;
    pds_vpc_spec_t spec;
    pds_obj_key_t key = int2pdsobjkey(1);

    feeder.init(key, PDS_VPC_TYPE_TENANT, "10.0.0.0/16", 1,
                "ba:de:de:ad:be:ef", "1000::1/64", "VxLAN 9999",
                int2pdsobjkey(4000), int2pdsobjkey(6000), 1);
    vpc_create(feeder);
    spec.tos = 2; // update tos
    vpc_update(feeder, &spec, VPC_ATTR_TOS);
    vpc_read(feeder);
    vpc_delete(feeder);
    vpc_read(feeder, SDK_RET_ENTRY_NOT_FOUND);
}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
#ifdef AGENT_MODE
    if (agent_mode()) {
        test_app_init();
    }
#endif
    return api_test_program_run(argc, argv);
}
