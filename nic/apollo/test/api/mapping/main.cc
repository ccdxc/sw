//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test cases
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/device.hpp"
#include "nic/apollo/test/api/utils/local_mapping.hpp"
#include "nic/apollo/test/api/utils/remote_mapping.hpp"
#include "nic/apollo/test/api/utils/subnet.hpp"
#include "nic/apollo/test/api/utils/tep.hpp"
#include "nic/apollo/test/api/utils/vnic.hpp"
#include "nic/apollo/test/api/utils/vpc.hpp"
#include "nic/apollo/test/api/utils/nexthop.hpp"
#include "nic/apollo/test/api/utils/if.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
pds_encap_type_t g_encap_type = PDS_ENCAP_TYPE_MPLSoUDP;
uint32_t g_encap_val = 1;


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
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_INFO;
        if (apulu()) {
            g_encap_type = PDS_ENCAP_TYPE_VXLAN;
            g_encap_val = pdsobjkey2int(k_subnet_key) + 512;
        }

        pds_batch_ctxt_t bctxt = batch_start();

        // device setup
        sample_device_setup(bctxt);

        // create VPC "10.0.0.0/8"
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_TENANT);

        // create 1 subnet per VPC
        sample1_subnet_setup(bctxt);

        // create TEPs
        if (!apulu()) {
            sample_tep_setup(bctxt, 2, "1.0.0.3", k_max_tep);
        } else {
            // temp until fixed
            sample_if_setup(bctxt);
            sample_nexthop_setup(bctxt);

            tep_feeder tep_feeder;
            tep_feeder.init(2, k_tep_mac, "1.0.0.3");
            many_create(bctxt, tep_feeder);
        }

        // create VNICs
        vnic_feeder vnic_feeder;
        vnic_feeder.init(int2pdsobjkey(1), k_max_vnic, 0x000000030b020a01,
                         PDS_ENCAP_TYPE_DOT1Q,
                         g_encap_type, true, false);
        many_create(bctxt, vnic_feeder);

        batch_commit(bctxt);
    }

    static void TearDownTestCase() {

        pds_batch_ctxt_t bctxt = batch_start();

        // delete VNICs
        vnic_feeder vnic_feeder;
        vnic_feeder.init(int2pdsobjkey(1), k_max_vnic, 0x000000030b020a01,
                         PDS_ENCAP_TYPE_DOT1Q,
                         g_encap_type, true, false);
        many_delete(bctxt, vnic_feeder);

        // delete TEPs
        sample_tep_teardown(bctxt, 2, "1.0.0.1", k_max_tep);

        // delete subnets
        sample1_subnet_teardown(bctxt);

        // delete VPC
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_TENANT);

        // delete device
        sample_device_teardown(bctxt);

        batch_commit(bctxt);

        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

//#define SCALE_DOWN_FOR_DEBUG

/// helper functions to create feeders
static void create_local_mapping_feeders(local_mapping_feeder feeders[],
                                         int num_feeders)
{
    int i;

    if (num_feeders < 1 || num_feeders > 4)
        return;

#ifndef SCALE_DOWN_FOR_DEBUG
     int num_vnics = k_max_vnic/num_feeders;
    int num_ips = PDS_MAX_VNIC_IP;
#else
    // debug
    int num_vnics = 2;
    int num_ips = 2;
#endif
    // first set
    feeders[0].init(k_vpc_key, k_subnet_key, "10.0.0.2/8",
                0x000000030b020a01, g_encap_type, g_encap_val, int2pdsobjkey(1),
                true, "12.0.0.0/16", num_vnics, num_ips, PDS_MAPPING_TYPE_L3);

    // subsequent sets can be copied from first set and iterated to
    // required position.
    for (i = 1; i < num_feeders; i++) {
        feeders[i] = feeders[i - 1];
        feeders[i].iter_next(num_vnics * PDS_MAX_VNIC_IP);
    }
}

static void create_remote_mapping_feeders(remote_mapping_feeder feeders[],
                                         int num_feeders)
{
    int i;

    if (num_feeders < 1 || num_feeders > 4)
        return;

#ifndef SCALE_DOWN_FOR_DEBUG
    int num_teps = PDS_MAX_TEP/num_feeders;
    int num_vnics = k_max_vnic;
#else
    int num_teps = 2;
    int num_vnics = 2;
#endif

    // first set
    feeders[0].init(k_vpc_key, k_subnet_key, "10.80.0.2/8",
                0x000000140b020a01, g_encap_type, g_encap_val,
                PDS_NH_TYPE_OVERLAY, 2, num_teps, num_vnics,
                PDS_MAPPING_TYPE_L3);

    // subsequent sets can be copied from first set and iterated to
    // required position.
    for (i = 1; i < num_feeders; i++) {
        feeders[i] = feeders[i - 1];
        feeders[i].iter_next(num_teps * PDS_MAX_TEP_VNIC);
    }
}


/// \endcond
//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup MAPPING_TEST Mapping tests
/// @{

// --------------------------- LOCAL MAPPINGS -----------------------

/// \brief Local mappings WF_1
/// \ref WF_1
TEST_F(mapping_test, local_mapping_workflow_1) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1);
    workflow_1<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, local_mapping_workflow_2) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1);
    workflow_2<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, local_mapping_workflow_3) {
    local_mapping_feeder feeders[3];

    create_local_mapping_feeders(feeders, 3);
    workflow_3<local_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Local mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, local_mapping_workflow_4) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1);
    workflow_4<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, local_mapping_workflow_5) {
    local_mapping_feeder feeders[3];

    create_local_mapping_feeders(feeders, 3);
    workflow_5<local_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Local mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, local_mapping_workflow_6) {
    local_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_local_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_6<local_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Local mappings WF_7
/// \ref WF_7
TEST_F(mapping_test, local_mapping_workflow_7) {
    local_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_local_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_7<local_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Local mappings WF_8
/// \ref WF_8
TEST_F(mapping_test, local_mapping_workflow_8) {
    local_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_local_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_8<local_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Local mappings WF_9
/// \ref WF_9
TEST_F(mapping_test, local_mapping_workflow_9) {
    local_mapping_feeder feeders[1], feeder1A;

    create_local_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_9<local_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Local mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_local_mapping_workflow_10) {
    local_mapping_feeder feeders[4], feeder2A, feeder3A;

    create_local_mapping_feeders(feeders, 4);
    feeder2A = feeders[1];
    feeder2A.update_spec(1);
    feeder3A = feeders[2];
    feeder3A.update_spec(1);

    workflow_10<local_mapping_feeder>(feeders[0], feeders[1], feeder2A,
                           feeders[2], feeder3A, feeders[3]);
}

/// \brief Local mappings WF_N_1
/// \ref WF_N_1
TEST_F(mapping_test, local_mapping_workflow_neg_1) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1);
    workflow_neg_1<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, local_mapping_workflow_neg_3) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1);
    workflow_neg_3<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, local_mapping_workflow_neg_4) {
    local_mapping_feeder feeders[2];

    create_local_mapping_feeders(feeders, 2);
    workflow_neg_4<local_mapping_feeder>(feeders[0], feeders[1]);
}

/// \brief Local mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, local_mapping_workflow_neg_5) {
    local_mapping_feeder feeders[1], feeder1A;

    create_local_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_5<local_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Local mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, local_mapping_workflow_neg_7) {
    local_mapping_feeder feeders[2], feeder1A;

    create_local_mapping_feeders(feeders, 2);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_7<local_mapping_feeder>(feeders[0], feeder1A, feeders[1]);
}

/// \brief Local mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, local_mapping_workflow_neg_8) {
    local_mapping_feeder feeders[2];

    create_local_mapping_feeders(feeders, 2);
    workflow_neg_8<local_mapping_feeder>(feeders[0], feeders[1]);
}

// --------------------------- END LOCAL MAPPINGS --------------------

// --------------------------- REMOTE MAPPINGS -----------------------

/// \brief Remote mappings WF_1
/// \ref WF_1
TEST_F(mapping_test, remote_mapping_workflow_1) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1);
    workflow_1<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, remote_mapping_workflow_2) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1);
    workflow_2<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, remote_mapping_workflow_3) {
    remote_mapping_feeder feeders[3];

    create_remote_mapping_feeders(feeders, 3);
    workflow_3<remote_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Remote mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, remote_mapping_workflow_4) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1);
    workflow_4<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, remote_mapping_workflow_5) {
    remote_mapping_feeder feeders[3];

    create_remote_mapping_feeders(feeders, 3);
    workflow_5<remote_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Remote mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, remote_mapping_workflow_6) {
    remote_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_remote_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_6<remote_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Remote mappings WF_7
/// \ref WF_7
TEST_F(mapping_test, remote_mapping_workflow_7) {
    remote_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_remote_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_7<remote_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Remote mappings WF_8
/// \ref WF_8
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_8) {
    remote_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_remote_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);
    feeder1B = feeders[0];
    feeder1B.update_spec(2);

    workflow_8<remote_mapping_feeder>(feeders[0], feeder1A, feeder1B);
}

/// \brief Remote mappings WF_9
/// \ref WF_9
TEST_F(mapping_test, remote_mapping_workflow_9) {
    remote_mapping_feeder feeders[1], feeder1A;

    create_remote_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_9<remote_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Remote mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_10) {
    remote_mapping_feeder feeders[4], feeder2A, feeder3A;

    create_remote_mapping_feeders(feeders, 4);
    feeder2A = feeders[1];
    feeder2A.update_spec(1);
    feeder3A = feeders[2];
    feeder3A.update_spec(1);

    workflow_10<remote_mapping_feeder>(feeders[0], feeders[1], feeder2A,
                           feeders[2], feeder3A, feeders[3]);
}

/// \brief Remote mappings WF_N_1
/// \ref WF_N_1
TEST_F(mapping_test, remote_mapping_workflow_neg_1) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1);
    workflow_neg_1<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, remote_mapping_workflow_neg_3) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1);
    workflow_neg_3<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, remote_mapping_workflow_neg_4) {
    remote_mapping_feeder feeders[2];

    create_remote_mapping_feeders(feeders, 2);
    workflow_neg_4<remote_mapping_feeder>(feeders[0], feeders[1]);
}

/// \brief Remote mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, remote_mapping_workflow_neg_5) {
    remote_mapping_feeder feeders[1], feeder1A;

    create_remote_mapping_feeders(feeders, 1);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_5<remote_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Remote mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, remote_mapping_workflow_neg_7) {
    remote_mapping_feeder feeders[2], feeder1A;

    create_remote_mapping_feeders(feeders, 2);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_7<remote_mapping_feeder>(feeders[0], feeder1A, feeders[1]);
}

/// \brief Remote mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, remote_mapping_workflow_neg_8) {
    remote_mapping_feeder feeders[2];

    create_remote_mapping_feeders(feeders, 2);
    workflow_neg_8<remote_mapping_feeder>(feeders[0], feeders[1]);
}

/// --------------------------- END REMOTE MAPPINGS --------------------
// @}


}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
