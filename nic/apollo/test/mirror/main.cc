//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all mirror session test cases
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/mirror.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

namespace api_test {
/// \cond
static constexpr uint32_t k_max_mirror_sessions = PDS_MAX_MIRROR_SESSION;
static constexpr uint32_t k_base_ms = 1;
static const uint32_t k_tep_id1 = 2;
static const char * const k_tep_ip1 = "10.1.1.1";
static const uint32_t k_tep_id2 = 100;
static const char * const k_tep_ip2 = "10.1.2.1";
static const uint32_t k_tep_id3 = 200;
static const char * const k_tep_ip3 = "10.1.3.1";

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
        sample_vpc_setup(bctxt, PDS_VPC_TYPE_UNDERLAY);
        sample_subnet_setup(bctxt);
        sample_tep_setup(bctxt, k_tep_id1, k_tep_ip1, 1);
        sample_tep_setup(bctxt, k_tep_id2, k_tep_ip2, 1);
        sample_tep_setup(bctxt, k_tep_id3, k_tep_ip3, 1);
        sample_device_setup(bctxt);
        batch_commit(bctxt);
    }
    static void TearDownTestCase() {
        pds_batch_ctxt_t bctxt = batch_start();
        sample_tep_teardown(bctxt, k_tep_id1, k_tep_ip1, 1);
        sample_tep_teardown(bctxt, k_tep_id2, k_tep_ip2, 1);
        sample_tep_teardown(bctxt, k_tep_id3, k_tep_ip3, 1);
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

static inline void
mirror_session_stepper_seed_init (int seed_base, uint8_t max_ms,
                                  mirror_session_stepper_seed_t *seed,
                                  bool update, uint8_t count)
{
    seed->key.id = seed_base;
    seed->num_ms = max_ms;
    // init with rspan type, then alternate b/w rspan and erspan in stepper
    seed->type = PDS_MIRROR_SESSION_TYPE_RSPAN;
    if (update) {
        seed->interface = 0x11020001;   // eth 1/2
        seed->encap.type = PDS_ENCAP_TYPE_DOT1Q;
        seed->encap.val.vlan_tag = seed_base +
            (count * PDS_MAX_MIRROR_SESSION);
        seed->vpc_id = 1;
        std::string dst_ip;
        uint32_t tep_id;
        std::string src_ip;
        if (count == 1) {
            tep_id = k_tep_id2;
            dst_ip = k_tep_ip2;
            src_ip = "20.1.2.1";
        } else {
            tep_id = k_tep_id3;
            dst_ip = k_tep_ip3;
            src_ip = "20.1.3.1";
        }
        seed->tep_id = tep_id;
        extract_ip_addr((char *)dst_ip.c_str(), &seed->dst_ip);
        extract_ip_addr((char *)src_ip.c_str(), &seed->src_ip);
        seed->span_id = PDS_MAX_MIRROR_SESSION * count;
        seed->dscp = 2;
    } else {
        seed->interface = 0x11010001;   // eth 1/1
        seed->encap.type = PDS_ENCAP_TYPE_DOT1Q;
        seed->encap.val.vlan_tag = seed_base;
        seed->vpc_id = 1;
        uint32_t tep_id = k_tep_id1;
        std::string dst_ip = k_tep_ip1;
        std::string src_ip = "20.1.1.1";
        memset(&seed->dst_ip, 0x0, sizeof(ip_addr_t));
        memset(&seed->src_ip, 0x0, sizeof(ip_addr_t));
        seed->tep_id = tep_id;
        extract_ip_addr((char *)dst_ip.c_str(), &seed->dst_ip);
        extract_ip_addr((char *)src_ip.c_str(), &seed->src_ip);
        seed->span_id = 1;
        seed->dscp = 1;
    }
}

/// \brief Mirror session WF_1
/// \ref WF_1
TEST_F(mirror_session_test, mirror_session_workflow_1) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_1<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Mirror session WF_3
/// \ref WF_3
TEST_F(mirror_session_test, mirror_session_workflow_3) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    mirror_session_stepper_seed_t seed3 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions, &seed1, false, 0);
    mirror_session_stepper_seed_init(3, num_mirror_sessions, &seed2, false, 0);
    mirror_session_stepper_seed_init(5, num_mirror_sessions, &seed3, false, 0);
    workflow_3<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed2, &seed3);
}

/// \brief Mirror session WF_4
/// \ref WF_4
TEST_F(mirror_session_test, mirror_session_workflow_4) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_4<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Mirror session WF_5
/// \ref WF_5
TEST_F(mirror_session_test, mirror_session_workflow_5) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    mirror_session_stepper_seed_t seed3 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(3, num_mirror_sessions,
                                     &seed2, false, 0);
    mirror_session_stepper_seed_init(5, num_mirror_sessions,
                                     &seed3, false, 0);
    workflow_5<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed2, &seed3);
}

/// \brief Mirror session WF_6
/// \ref WF_6
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_6) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};
    mirror_session_stepper_seed_t seed1b = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1a, true, 1);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1b, true, 2);
    workflow_6<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a, &seed1b);
}

/// \brief Mirror session WF_7
/// \ref WF_7
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_7) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};
    mirror_session_stepper_seed_t seed1b = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1a, true, 1);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1b, true, 2);
    workflow_7<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a, &seed1b);
}

/// \brief Mirror session WF_8
/// \ref WF_8
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_8) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};
    mirror_session_stepper_seed_t seed1b = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1a, true, 1);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1b, true, 2);
    workflow_8<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a, &seed1b);

}

/// \brief Mirror session WF_9
/// \ref WF_9
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_9) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1a, true, 1);
    workflow_9<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a);
}

/// \brief Mirror session WF_10
/// \ref WF_10
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_10) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    mirror_session_stepper_seed_t seed3 = {};
    mirror_session_stepper_seed_t seed4 = {};
    mirror_session_stepper_seed_t seed2a = {};
    mirror_session_stepper_seed_t seed3a = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(3, num_mirror_sessions,
                                     &seed2, false, 0);
    mirror_session_stepper_seed_init(5, num_mirror_sessions,
                                     &seed3, false, 0);
    mirror_session_stepper_seed_init(7, num_mirror_sessions,
                                     &seed4, false, 0);
    mirror_session_stepper_seed_init(3, num_mirror_sessions,
                                     &seed2a, true, 1);
    mirror_session_stepper_seed_init(5, num_mirror_sessions,
                                     &seed3a, true, 1);

    workflow_10<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed2, &seed2a, &seed3, &seed3a, &seed4);
}

/// \brief Mirror session WF_N_1
/// \ref WF_N_1
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_neg_1<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Mirror session WF_N_2
/// \ref WF_N_2
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions+1,
                                     &seed, false, 0);
    workflow_neg_2<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Mirror session WF_N_3
/// \ref WF_N_3
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_3) {
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms+8, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_neg_3<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Mirror session WF_N_4
/// \ref WF_N_4
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(9, num_mirror_sessions,
                                     &seed2, false, 0);
    workflow_neg_4<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed2);
}

/// \brief Mirror session WF_N_5
/// \ref WF_N_5
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_5) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1a, true, 1);
    workflow_neg_5<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a);
}

/// \brief Mirror session WF_N_6
/// \ref WF_N_6
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_6) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed1, false, 0);

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions+1,
                                     &seed1a, true, 1);
    workflow_neg_6<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a);
}

/// \brief Mirror session WF_N_7
/// \ref WF_N_7
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_7) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed1a = {};
    mirror_session_stepper_seed_t seed2 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1a, true, 1);
    mirror_session_stepper_seed_init(3, num_mirror_sessions,
                                     &seed2, true, 1);
    workflow_neg_7<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed1a, &seed2);
}

/// \brief Mirror session WF_N_8
/// \ref WF_N_8
TEST_F(mirror_session_test, DISABLED_mirror_session_workflow_neg_8) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, num_mirror_sessions,
                                     &seed1, false, 0);
    mirror_session_stepper_seed_init(3, num_mirror_sessions,
                                     &seed2, true, 1);
    workflow_neg_8<mirror_session_util, mirror_session_stepper_seed_t>(
        &seed1, &seed2);
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
