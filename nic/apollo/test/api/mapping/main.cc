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
#include "nic/apollo/test/api/utils/utils.hpp"
#include "nic/apollo/test/api/utils/workflow.hpp"

namespace test {
namespace api {

// globals
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
pds_encap_type_t g_encap_type = PDS_ENCAP_TYPE_MPLSoUDP;
uint32_t g_encap_val = 1;
//#define SCALE_DOWN_FOR_DEBUG

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
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_DEBUG;
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
        vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                         0x000000030b020a01, PDS_ENCAP_TYPE_DOT1Q,
                         g_encap_type, true, false, 0, 0, 5, 0);
        many_create(bctxt, vnic_feeder);

        batch_commit(bctxt);
    }

    static void TearDownTestCase() {

        pds_batch_ctxt_t bctxt = batch_start();

        // delete VNICs
        vnic_feeder vnic_feeder;
        vnic_feeder.init(int2pdsobjkey(1), int2pdsobjkey(1), k_max_vnic,
                         0x000000030b020a01, PDS_ENCAP_TYPE_DOT1Q,
                         g_encap_type, true, false, 0, 0, 5, 0);
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

/// helper functions to create feeders
static void create_local_mapping_feeders(local_mapping_feeder feeders[],
                                         int num_feeders, int nvnics,
                                         int nips, int ntags)
{
    int i;

    if (num_feeders < 1 || num_feeders > 4)
        return;

#ifndef SCALE_DOWN_FOR_DEBUG
     int num_vnics = nvnics/num_feeders;
    int num_ips = nips;
#else
    // debug
    int num_vnics = 2;
    int num_ips = 2;
#endif
    // first set
    feeders[0].init(k_vpc_key, k_subnet_key, "10.0.0.2/8",
                0x000000030b020a01, g_encap_type, g_encap_val, int2pdsobjkey(1),
                true, "12.0.0.0/16", num_vnics, num_ips, PDS_MAPPING_TYPE_L3,
                ntags);

    // subsequent sets can be copied from first set and iterated to
    // required position.
    for (i = 1; i < num_feeders; i++) {
        feeders[i] = feeders[i - 1];
        feeders[i].iter_next(num_vnics * PDS_MAX_VNIC_IP);
    }
}

static void create_remote_mapping_feeders(remote_mapping_feeder feeders[],
                                         int num_feeders, int nteps,
                                         int nvnics, int ntags)
{
    int i;

    if (num_feeders < 1 || num_feeders > 4)
        return;

#ifndef SCALE_DOWN_FOR_DEBUG
    int num_teps = nteps/num_feeders;
    int num_vnics = nvnics;
#else
    int num_teps = 2;
    int num_vnics = 2;
#endif

    // first set
    feeders[0].init(k_vpc_key, k_subnet_key, "10.80.0.2/8",
                0x000000140b020a01, g_encap_type, g_encap_val,
                PDS_NH_TYPE_OVERLAY, 2, num_teps, num_vnics,
                PDS_MAPPING_TYPE_L3, ntags);

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

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_1<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, local_mapping_workflow_2) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_2<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, local_mapping_workflow_3) {
    local_mapping_feeder feeders[3];

    create_local_mapping_feeders(feeders, 3, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_3<local_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Local mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, local_mapping_workflow_4) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_4<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, local_mapping_workflow_5) {
    local_mapping_feeder feeders[3];

    create_local_mapping_feeders(feeders, 3, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_5<local_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Local mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, local_mapping_workflow_6) {
    local_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
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

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
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

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
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

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_9<local_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Local mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_local_mapping_workflow_10) {
    local_mapping_feeder feeders[4], feeder2A, feeder3A;

    create_local_mapping_feeders(feeders, 4, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
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

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_1<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, local_mapping_workflow_neg_3) {
    local_mapping_feeder feeders[1];

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_3<local_mapping_feeder>(feeders[0]);
}

/// \brief Local mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, local_mapping_workflow_neg_4) {
    local_mapping_feeder feeders[2];

    create_local_mapping_feeders(feeders, 2, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_4<local_mapping_feeder>(feeders[0], feeders[1]);
}

/// \brief Local mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, local_mapping_workflow_neg_5) {
    local_mapping_feeder feeders[1], feeder1A;

    create_local_mapping_feeders(feeders, 1, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_5<local_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Local mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, local_mapping_workflow_neg_7) {
    local_mapping_feeder feeders[2], feeder1A;

    create_local_mapping_feeders(feeders, 2, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_7<local_mapping_feeder>(feeders[0], feeder1A, feeders[1]);
}

/// \brief Local mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, local_mapping_workflow_neg_8) {
    local_mapping_feeder feeders[2];

    create_local_mapping_feeders(feeders, 2, k_max_vnic, PDS_MAX_VNIC_IP,
                                 PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_8<local_mapping_feeder>(feeders[0], feeders[1]);
}

//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------
/// \brief update mapping vnic
TEST_F(mapping_test, local_mapping_update_vnic) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.vnic = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping subnet
TEST_F(mapping_test, DISABLED_local_mapping_update_subnet) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.subnet = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_SUBNET);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping fabric encap type
TEST_F(mapping_test, local_mapping_update_fab_encap_type) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.fabric_encap = feeders[0].spec.fabric_encap;
    utils_encap_type_update(&spec.fabric_encap);
    lmap_update(feeders[0], &spec, LMAP_ATTR_FAB_ENCAP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping fabric encap val
TEST_F(mapping_test, local_mapping_update_fab_encap_val) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.fabric_encap = feeders[0].spec.fabric_encap;
    utils_encap_val_update(&spec.fabric_encap, feeders[0].num_obj * 3);
    lmap_update(feeders[0], &spec, LMAP_ATTR_FAB_ENCAP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping mapping M1 public IP P1 to no public IP.
TEST_F(mapping_test, DISABLED_local_mapping_update_publicip1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip_valid = false;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping mapping M1 public IP P1 to P2.
TEST_F(mapping_test, DISABLED_local_mapping_update_publicip2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip = feeders[0].spec.public_ip;
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping mapping M1 no public IP  to valid public ip.
TEST_F(mapping_test, DISABLED_local_mapping_update_publicip3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
     ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping mapping M1 public IP P1 to P2 and vnic V1 to V2.
TEST_F(mapping_test, local_mapping_update_publicip4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip = feeders[0].spec.public_ip;
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    spec.vnic = int2pdsobjkey(2);
    chg_bmap = (LMAP_ATTR_PUBLIC_IP | LMAP_ATTR_VNIC);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping vnic_mac.
TEST_F(mapping_test, local_mapping_update_vnic_mac) {
    if (!apulu()) return;

    uint64_t mac;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    mac = MAC_TO_UINT64(feeders[0].spec.vnic_mac);
    mac++;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, mac);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC_MAC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with no tags to Tag T1.
TEST_F(mapping_test, local_mapping_update_tag1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with tags set T1 to T2.
TEST_F(mapping_test, local_mapping_update_tag2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + feeders[0].spec.num_tags;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with tags set T1 to no tags.
TEST_F(mapping_test, local_mapping_update_tag3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP no Tag,
/// \update mapping M1 with no public IP and no Tag.(effectively no change)
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_1_1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    // update vnic to have no change on pip and tag
    spec.vnic = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and with  Tag,
/// \update mapping M1 with no public IP and with Tag. (effectively no change)
TEST_F(mapping_test, local_mapping_update_pip_tag_1_2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    // update vnic to have no change on pip and tag
    spec.vnic = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and no Tag,
/// \update mapping M1 with no public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_1_3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and with Tag,
/// \update mapping M1 with no public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_1_4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and with Tag,
/// \update mapping M1 with no public IP and with new Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_1_5) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + spec.num_tags;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP no Tag,
/// \update mapping M1 with public IP and no Tag.(effectively no change)
TEST_F(mapping_test, local_mapping_update_pip_tag_2_1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    // update vnic to have no change on pip and tag
    spec.vnic = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and with  Tag,
/// \update mapping M1 with with public IP and with Tag. (effectively no change)
TEST_F(mapping_test, local_mapping_update_pip_tag_2_2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    // update vnic to have no change on pip and tag
    spec.vnic = int2pdsobjkey(2);
    lmap_update(feeders[0], &spec, LMAP_ATTR_VNIC);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and no Tag,
/// \update mapping M1 with with public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_2_3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and with Tag,
/// \update mapping M1 with with public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_2_4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

#if 0
/// \brief initial state with public IP and with Tag,
/// \update mapping M1 with with public IP and with new Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_2_5) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + spec.num_tags;
    }
    lmap_update(feeders[0], &spec, LMAP_ATTR_TAGS);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}
#endif

/// \brief initial state with no public IP no Tag,
/// \update mapping M1 with public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_3_1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and with  Tag,
/// \update mapping M1  with public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_3_2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and no Tag,
/// \update mapping M1 with public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_3_3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap = 0;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with  no public IP and with Tag,
/// \update mapping M1 with with public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_3_4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state no public IP and with Tag,
/// \update mapping M1 with with public IP and with new Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_3_5) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    feeders[0].spec.public_ip_valid = false;
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + spec.num_tags;
    }
    test::extract_ip_pfx("12.0.0.1", &public_ip_pfx);
    spec.public_ip = public_ip_pfx.addr;
    spec.public_ip_valid = true;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

#if 0
/// \brief initial state  with public IP no Tag,
/// \update mapping M1 no public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_4_1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip_valid = false;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}
#endif

/// \brief initial state with public IP and with  Tag,
/// \update mapping M1  no public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_4_2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip_valid = false;
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

#if 0
/// \brief initial state with public IP and no Tag,
/// \update mapping M1 no public IP and with Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_4_3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap = 0;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    spec.public_ip_valid = false;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}
#endif

/// \brief initial state with  with public IP and with Tag,
/// \update mapping M1 no public IP and no Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_4_4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    spec.public_ip_valid = false;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and with Tag,
/// \update mapping M1 with no public IP and with new Tag.
TEST_F(mapping_test, local_mapping_update_pip_tag_4_5) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + spec.num_tags;
    }
    spec.public_ip_valid = false;
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state  with public IP no Tag,
/// \update mapping M1 new public IP and no Tag.
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_5_1) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and with  Tag,
/// \update mapping M1  new public IP and with Tag.
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_5_2) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    lmap_update(feeders[0], &spec, LMAP_ATTR_PUBLIC_IP);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and no Tag,
/// \update mapping M1 new public IP and with Tag.
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_5_3) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap = 0;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 0);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with  with public IP and with Tag,
/// \update mapping M1 new public IP and no Tag.
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_5_4) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 1);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief initial state with public IP and with Tag,
/// \update mapping M1 with new public IP and with new Tag.
TEST_F(mapping_test, DISABLED_local_mapping_update_pip_tag_5_5) {
    if (!apulu()) return;

    pds_local_mapping_spec_t spec = {0};
    local_mapping_feeder feeders[1];
    ip_prefix_t public_ip_pfx;
    uint64_t chg_bmap;

    // init
    create_local_mapping_feeders(feeders, 1, 1, 1, 2);
    lmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + spec.num_tags;
    }
    spec.public_ip_valid = feeders[0].spec.public_ip_valid;
    increment_ip_addr(&spec.public_ip, 1);
    chg_bmap = (LMAP_ATTR_TAGS | LMAP_ATTR_PUBLIC_IP);
    lmap_update(feeders[0], &spec, chg_bmap);

    // validate
    lmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    lmap_delete(feeders[0]);
    lmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

// --------------------------- END LOCAL MAPPINGS --------------------

// --------------------------- REMOTE MAPPINGS -----------------------

/// \brief Remote mappings WF_1
/// \ref WF_1
TEST_F(mapping_test, remote_mapping_workflow_1) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_1<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, remote_mapping_workflow_2) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_2<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, remote_mapping_workflow_3) {
    remote_mapping_feeder feeders[3];

    create_remote_mapping_feeders(feeders, 3, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_3<remote_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Remote mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, remote_mapping_workflow_4) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_4<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, remote_mapping_workflow_5) {
    remote_mapping_feeder feeders[3];

    create_remote_mapping_feeders(feeders, 3, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_5<remote_mapping_feeder>(feeders[0], feeders[1], feeders[2]);
}

/// \brief Remote mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, remote_mapping_workflow_6) {
    remote_mapping_feeder feeders[1], feeder1A, feeder1B;

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
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

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
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

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
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

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_9<remote_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Remote mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_10) {
    remote_mapping_feeder feeders[4], feeder2A, feeder3A;

    create_remote_mapping_feeders(feeders, 4, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
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

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_1<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, remote_mapping_workflow_neg_3) {
    remote_mapping_feeder feeders[1];

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_3<remote_mapping_feeder>(feeders[0]);
}

/// \brief Remote mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, remote_mapping_workflow_neg_4) {
    remote_mapping_feeder feeders[2];

    create_remote_mapping_feeders(feeders, 2, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_4<remote_mapping_feeder>(feeders[0], feeders[1]);
}

/// \brief Remote mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, remote_mapping_workflow_neg_5) {
    remote_mapping_feeder feeders[1], feeder1A;

    create_remote_mapping_feeders(feeders, 1, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_5<remote_mapping_feeder>(feeders[0], feeder1A);
}

/// \brief Remote mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, remote_mapping_workflow_neg_7) {
    remote_mapping_feeder feeders[2], feeder1A;

    create_remote_mapping_feeders(feeders, 2, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    feeder1A = feeders[0];
    feeder1A.update_spec(1);

    workflow_neg_7<remote_mapping_feeder>(feeders[0], feeder1A, feeders[1]);
}

/// \brief Remote mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, remote_mapping_workflow_neg_8) {
    remote_mapping_feeder feeders[2];

    create_remote_mapping_feeders(feeders, 2, PDS_MAX_TEP, k_max_vnic,
                                  PDS_MAX_TAGS_PER_MAPPING);
    workflow_neg_8<remote_mapping_feeder>(feeders[0], feeders[1]);
}
//---------------------------------------------------------------------
// Non templatized test cases
//---------------------------------------------------------------------
/// \brief update mapping subnet
TEST_F(mapping_test, DISABLED_remote_mapping_update_subnet) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    spec.subnet = int2pdsobjkey(2);
    rmap_update(feeders[0], &spec, RMAP_ATTR_SUBNET);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping fabric encap type
TEST_F(mapping_test, remote_mapping_update_fab_encap_type) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    spec.fabric_encap = feeders[0].spec.fabric_encap;
    utils_encap_type_update(&spec.fabric_encap);
    rmap_update(feeders[0], &spec, RMAP_ATTR_FAB_ENCAP);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping fabric encap val
TEST_F(mapping_test, remote_mapping_update_fab_encap_val) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    spec.fabric_encap = feeders[0].spec.fabric_encap;
    utils_encap_val_update(&spec.fabric_encap, feeders[0].num_obj * 3);
    rmap_update(feeders[0], &spec, RMAP_ATTR_FAB_ENCAP);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 public Tunnel T1 to T2.
TEST_F(mapping_test, remote_mapping_update_nhtype1) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    spec.tep = feeders[0].spec.tep;
    spec.tep = int2pdsobjkey(pdsobjkey2int(spec.tep) + 1);
    spec.nh_type = PDS_NH_TYPE_OVERLAY;
    rmap_update(feeders[0], &spec, RMAP_ATTR_NH_TYPE);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 Tunnel T1 to NH group N1.
TEST_F(mapping_test, DISABLED_remote_mapping_update_nhtype2) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    spec.tep = feeders[0].spec.tep;
    spec.nh_group = int2pdsobjkey(pdsobjkey2int(spec.tep) + 1);
    spec.nh_type = PDS_NH_TYPE_UNDERLAY;
    rmap_update(feeders[0], &spec, RMAP_ATTR_NH_TYPE);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 NH group N1 to N2.
TEST_F(mapping_test, DISABLED_remote_mapping_update_nhtype3) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.nh_type = PDS_NH_TYPE_UNDERLAY;
    rmap_create(feeders[0]);

    // trigger
    spec.nh_group = feeders[0].spec.nh_group;
    spec.nh_group = int2pdsobjkey(pdsobjkey2int(spec.nh_group) + 1);
    spec.nh_type = PDS_NH_TYPE_UNDERLAY;
    rmap_update(feeders[0], &spec, RMAP_ATTR_NH_TYPE);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 NH group N1 to tep T1.
TEST_F(mapping_test, DISABLED_remote_mapping_update_nhtype4) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    feeders[0].spec.nh_type = PDS_NH_TYPE_UNDERLAY;
    rmap_create(feeders[0]);

    // trigger
    spec.nh_group = feeders[0].spec.nh_group;
    spec.tep = int2pdsobjkey(pdsobjkey2int(spec.nh_group) + 1);
    spec.nh_type = PDS_NH_TYPE_OVERLAY;
    rmap_update(feeders[0], &spec, RMAP_ATTR_NH_TYPE);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping vnic_mac.
TEST_F(mapping_test, remote_mapping_update_vnic_mac) {
    if (!apulu()) return;

    uint64_t mac;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 1);
    rmap_create(feeders[0]);

    // trigger
    mac = MAC_TO_UINT64(feeders[0].spec.vnic_mac);
    mac++;
    MAC_UINT64_TO_ADDR(spec.vnic_mac, mac);
    rmap_update(feeders[0], &spec, RMAP_ATTR_VNIC_MAC);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with no tags to Tag T1.
TEST_F(mapping_test, DISABLED_remote_mapping_update_tag1) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 0);
    rmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = i+1;
    }
    rmap_update(feeders[0], &spec, RMAP_ATTR_TAGS);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with tags set T1 to T2.
TEST_F(mapping_test, DISABLED_remote_mapping_update_tag2) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 2);
    rmap_create(feeders[0]);

    // trigger
    spec.num_tags = 2;
    for (int i = 0; i<spec.num_tags; i++) {
        spec.tags[i] = feeders[0].spec.tags[i] + feeders[0].spec.num_tags;
    }
    rmap_update(feeders[0], &spec, RMAP_ATTR_TAGS);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief update mapping M1 with tags set T1 to no tags.
TEST_F(mapping_test, DISABLED_remote_mapping_update_tag3) {
    if (!apulu()) return;

    pds_remote_mapping_spec_t spec = {0};
    remote_mapping_feeder feeders[1];

    // init
    create_remote_mapping_feeders(feeders, 1, 1, 1, 2);
    rmap_create(feeders[0]);

    // trigger
    spec.num_tags = 0;
    rmap_update(feeders[0], &spec, RMAP_ATTR_TAGS);

    // validate
    rmap_read(feeders[0], SDK_RET_OK);

    // cleanup
    rmap_delete(feeders[0]);
    rmap_read(feeders[0], SDK_RET_ENTRY_NOT_FOUND);
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
