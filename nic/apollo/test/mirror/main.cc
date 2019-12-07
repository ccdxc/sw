//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all mirror session test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/mirror.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/nexthop.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

namespace api_test {
/// \cond
// Globals

static constexpr uint32_t k_max_mirror_sessions = PDS_MAX_MIRROR_SESSION;
static constexpr uint32_t k_base_ms = 1;
static const uint32_t k_tep_id1 = 2;
static const char * const k_tep_ip1 = "10.1.1.1";
static const uint32_t k_tep_id2 = 100;
static const char * const k_tep_ip2 = "10.1.2.1";
static const uint32_t k_tep_id3 = 200;
static const char * const k_tep_ip3 = "10.1.3.1";
static const pds_ifindex_t k_ifindex_id1 = 0x11010001;
static const pds_ifindex_t k_ifindex_id2 = 0x11020001;
static const char* const k_src_ip1 = "20.1.1.1";
static const char* const k_src_ip2 = "20.1.2.1";
static const char* const k_src_ip3 = "20.1.3.1";


//----------------------------------------------------------------------------
// Mirror session test class
//----------------------------------------------------------------------------

class mirror_session_test : public pds_test_base {
protected:
    mirror_session_test() {}
    virtual ~mirror_session_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        pds_batch_ctxt_t bctxt = batch_start();
        sample_device_setup(bctxt);
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_UNDERLAY);
        sample_subnet_setup(bctxt);
        if (apulu()) {
            sample_if_setup(bctxt);
            sample_nexthop_setup(bctxt);
        }
        sample_tep_setup(bctxt, k_tep_id1, k_tep_ip1, 1);
        sample_tep_setup(bctxt, k_tep_id2, k_tep_ip2, 1);
        sample_tep_setup(bctxt, k_tep_id3, k_tep_ip3, 1);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_tep_teardown(bctxt, k_tep_id1, k_tep_ip1, 1);
        sample_tep_teardown(bctxt, k_tep_id2, k_tep_ip2, 1);
        sample_tep_teardown(bctxt, k_tep_id3, k_tep_ip3, 1);
        if (apulu()) {
            sample_nexthop_teardown(bctxt);
            sample_if_teardown(bctxt);
        }
        sample_subnet_teardown(bctxt);
        sample_vpc_teardown(bctxt, PDS_VPC_TYPE_UNDERLAY);
        sample_device_teardown(bctxt);
        batch_commit(bctxt);
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};
/// \endcond
//----------------------------------------------------------------------------
// mirror session test cases implementation
//----------------------------------------------------------------------------

/// \defgroup MIRROR_TEST Mirror tests
/// @{

/// \brief Mirror session WF_1
/// \ref WF_1
TEST_F(mirror_session_test, mirror_session_workflow_1) {
    pds_mirror_session_key_t key = {.id = k_base_ms};
    mirror_session_feeder feeder;

    feeder.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_1<mirror_session_feeder>(feeder);
}
/// \brief Mirror session WF_2
/// \ref WF_2
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_2) {
    pds_mirror_session_key_t key = {.id = k_base_ms};
    mirror_session_feeder feeder;
    feeder.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_2<mirror_session_feeder>(feeder);
}

/// \brief Mirror session WF_3
/// \ref WF_3
TEST_F(mirror_session_test, mirror_session_workflow_3) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 3}, key3 = {.id = 5};
    mirror_session_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder2.init(key2, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder3.init(key3, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_3<mirror_session_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Mirror session WF_4
/// \ref WF_4
TEST_F(mirror_session_test, mirror_session_workflow_4) {
    pds_mirror_session_key_t key = {.id = k_base_ms};
    mirror_session_feeder feeder;

    feeder.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_4<mirror_session_feeder>(feeder);
}

/// \brief Mirror session WF_5
/// \ref WF_5
TEST_F(mirror_session_test, mirror_session_workflow_5) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 3}, key3 = {.id = 5};
    mirror_session_feeder feeder1, feeder2, feeder3;

    feeder1.init(key1, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder2.init(key2, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder3.init(key3, 2, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_5<mirror_session_feeder>(feeder1, feeder2, feeder3);
}

/// \brief Mirror session WF_6
/// \ref WF_6
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_6) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A, feeder1B;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint16_t vlan_tag_1b = key.id + 2 * PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1b = 2 * PDS_MAX_MIRROR_SESSION;


    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, 1,
                 k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    feeder1B.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1b,
                  k_src_ip3, k_tep_id3, span_id_1b, 2);
    workflow_6<mirror_session_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Mirror session WF_7
/// \ref WF_7
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_7) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A, feeder1B;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint16_t vlan_tag_1b = key.id + 2 * PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1b = 2 * PDS_MAX_MIRROR_SESSION;


    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, 1,
                 k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    feeder1B.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1b,
                  k_src_ip3, k_tep_id3, span_id_1b, 2);
    workflow_7<mirror_session_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Mirror session WF_8
/// \ref WF_8
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_8) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A, feeder1B;
    uint16_t vlan_tag_1 = key.id;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint16_t vlan_tag_1b = key.id + 2 * PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1b = 2 * PDS_MAX_MIRROR_SESSION;


    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, vlan_tag_1,
                 k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    feeder1B.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1b,
                  k_src_ip3, k_tep_id3, span_id_1b, 2);
    workflow_8<mirror_session_feeder>(feeder1, feeder1A, feeder1B);
}

/// \brief Mirror session WF_9
/// \ref WF_9
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_9) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A, feeder1B;
    uint16_t vlan_tag_1 = key.id;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;


    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, vlan_tag_1,
                 k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    workflow_9<mirror_session_feeder>(feeder1, feeder1A);
}

/// \brief Mirror session WF_10
/// \ref WF_10
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_10) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 3},
                             key3 = {.id = 5}, key4 = {.id = 7};
    uint32_t num_mirror_sessions = 2;
    mirror_session_feeder feeder1, feeder2, feeder3, feeder4, feeder2A, feeder3A;

    uint16_t vlan_tag_1 = key1.id;
    uint16_t vlan_tag_2 = key2.id;
    uint16_t vlan_tag_3 = key3.id;
    uint16_t vlan_tag_4 = key4.id;
    uint16_t vlan_tag_2a = 4 * num_mirror_sessions + 1;
    uint16_t vlan_tag_3a = 5 * num_mirror_sessions + 1;

    uint32_t span_id_2a = 4 * num_mirror_sessions;
    uint32_t span_id_3a = 5 * num_mirror_sessions;

    feeder1.init(key1, num_mirror_sessions, k_ifindex_id1, vlan_tag_1, k_src_ip1, k_tep_id1);
    feeder2.init(key2, num_mirror_sessions, k_ifindex_id1, vlan_tag_2, k_src_ip1, k_tep_id1);
    feeder3.init(key3, num_mirror_sessions, k_ifindex_id1, vlan_tag_3, k_src_ip1, k_tep_id1);
    feeder4.init(key4, num_mirror_sessions, k_ifindex_id1, vlan_tag_4, k_src_ip1, k_tep_id1);


    feeder2A.init(key2, num_mirror_sessions, k_ifindex_id2, vlan_tag_2a,
                  k_src_ip2, k_tep_id2, span_id_2a, 2);
    feeder3A.init(key3, num_mirror_sessions, k_ifindex_id2, vlan_tag_3a,
                  k_src_ip2, k_tep_id2, span_id_3a, 2);
    workflow_10<mirror_session_feeder>(
        feeder1, feeder2, feeder2A, feeder3, feeder3A, feeder4);
}

/// \brief Mirror session WF_N_1
/// \ref WF_N_1
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_1) {
    pds_mirror_session_key_t key = {.id = k_base_ms};
    mirror_session_feeder feeder;

    feeder.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_neg_1<mirror_session_feeder>(feeder);
}

/// \brief Mirror session WF_N_2
/// \ref WF_N_2
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_2) {
    pds_mirror_session_key_t key = {.id = k_base_ms};
    mirror_session_feeder feeder;

    feeder.init(key, k_max_mirror_sessions + 1, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_neg_2<mirror_session_feeder>(feeder);
}

/// \brief Mirror session WF_N_3
/// \ref WF_N_3
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_3) {
    pds_mirror_session_key_t key = {.id = k_base_ms + 8};
    mirror_session_feeder feeder;

    feeder.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    workflow_neg_3<mirror_session_feeder>(feeder);
}

/// \brief Mirror session WF_N_4
/// \ref WF_N_4
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_4) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 9};
    mirror_session_feeder feeder1, feeder2;
    uint32_t num_mirror_sessions = 2;

    feeder1.init(key1, num_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder2.init(key2, num_mirror_sessions, k_ifindex_id1, 9, k_src_ip1, k_tep_id1);
    workflow_neg_4<mirror_session_feeder>(feeder1, feeder2);
}

/// \brief Mirror session WF_N_5
/// \ref WF_N_5
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_5) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;

    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    workflow_neg_5<mirror_session_feeder>(feeder1, feeder1A);
}

/// \brief Mirror session WF_N_6
/// \ref WF_N_6
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_6) {
    pds_mirror_session_key_t key = {.id = 1};
    mirror_session_feeder feeder1, feeder1A;
    uint16_t vlan_tag_1a = key.id + PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;

    feeder1.init(key, k_max_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder1A.init(key, k_max_mirror_sessions + 1, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    workflow_neg_6<mirror_session_feeder>(feeder1, feeder1A);
}

/// \brief Mirror session WF_N_7
/// \ref WF_N_7
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_7) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 3};
    mirror_session_feeder feeder1, feeder1A, feeder2;
    uint16_t vlan_tag_1a = key1.id + PDS_MAX_MIRROR_SESSION;
    uint16_t vlan_tag_2 = 2 * PDS_MAX_MIRROR_SESSION + 1;
    uint32_t span_id_1a = PDS_MAX_MIRROR_SESSION;
    uint32_t span_id_2 = 2 * PDS_MAX_MIRROR_SESSION;
    uint32_t num_mirror_sessions = 2;

    feeder1.init(key1, num_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder1A.init(key1, num_mirror_sessions, k_ifindex_id2, vlan_tag_1a,
                  k_src_ip2, k_tep_id2, span_id_1a, 2);
    feeder2.init(key2, num_mirror_sessions, k_ifindex_id2, vlan_tag_2,
                  k_src_ip2, k_tep_id2, span_id_2, 2);
    workflow_neg_7<mirror_session_feeder>(feeder1, feeder1A, feeder2);
}

/// \brief Mirror session WF_N_8
/// \ref WF_N_8
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_8) {
    pds_mirror_session_key_t key1 = {.id = 1}, key2 = {.id = 3};
    mirror_session_feeder feeder1, feeder2;
    uint16_t vlan_tag_2 = PDS_MAX_MIRROR_SESSION + 1;
    uint32_t span_id_2 = PDS_MAX_MIRROR_SESSION;
    uint32_t num_mirror_sessions = 2;

    feeder1.init(key1, num_mirror_sessions, k_ifindex_id1, 1, k_src_ip1, k_tep_id1);
    feeder2.init(key2, num_mirror_sessions, k_ifindex_id2, vlan_tag_2,
                  k_src_ip2, k_tep_id2, span_id_2, 2);
    workflow_neg_8<mirror_session_feeder>(feeder1, feeder2);
}
/// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
