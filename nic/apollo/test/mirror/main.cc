//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all mirror session test cases
///
//----------------------------------------------------------------------------
#include <stdio.h>
#include <getopt.h>
#include <gtest/gtest.h>
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/workflow.hpp"
#include "nic/apollo/test/utils/mirror.hpp"

const char *g_cfg_file = "hal.json";
std::string g_pipeline("apollo");
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr int k_max_mirror_sessions = PDS_MAX_MIRROR_SESSION;
constexpr int k_base_ms = 1;
const std::string g_device_ip("91.0.0.1");
const std::string g_gateway_ip("90.0.0.2");
const std::string g_device_macaddr("00:00:01:02:0a:0b");

namespace api_test {

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
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.pipeline = g_pipeline;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);

        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(PDS_VPC_TYPE_SUBSTRATE, 1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_util tep_obj("10.1.1.1", PDS_TEP_TYPE_WORKLOAD, encap, true);
        tep_util tep_obj_update("10.1.2.1", PDS_TEP_TYPE_WORKLOAD,
                                encap, true);
        tep_util tep_obj_update_2("10.1.3.1", PDS_TEP_TYPE_WORKLOAD,
                                encap, true);
        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);

        BATCH_START();
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.create() == sdk::SDK_RET_OK);
        TEP_CREATE(tep_obj);
        TEP_CREATE(tep_obj_update);
        TEP_CREATE(tep_obj_update_2);
        ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
        BATCH_COMMIT();
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(PDS_VPC_TYPE_SUBSTRATE, 1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_util tep_obj("10.1.1.1", PDS_TEP_TYPE_WORKLOAD, encap, true);
        tep_util tep_obj_update("10.1.2.1", PDS_TEP_TYPE_WORKLOAD,
                                encap, true);
        tep_util tep_obj_update_2("10.1.3.1", PDS_TEP_TYPE_WORKLOAD,
                                encap, true);
        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);

        BATCH_START();
        ASSERT_TRUE(vpc_obj.del() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.del() == sdk::SDK_RET_OK);
        TEP_DELETE(tep_obj);
        TEP_DELETE(tep_obj_update);
        TEP_DELETE(tep_obj_update_2);
        ASSERT_TRUE(device_obj.del() == sdk::SDK_RET_OK);
        BATCH_COMMIT();

        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// mirror session test cases implementation
//----------------------------------------------------------------------------

/// \defgroup MIRROR_TEST
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
        std::string src_ip;
        if (count == 1) {
            dst_ip = "10.1.2.1";
            src_ip = "20.1.2.1";
        } else {
            dst_ip = "10.1.3.1";
            src_ip = "20.1.3.1";
        }
        extract_ip_addr((char *)dst_ip.c_str(), &seed->dst_ip);
        extract_ip_addr((char *)src_ip.c_str(), &seed->src_ip);
        seed->span_id = PDS_MAX_MIRROR_SESSION * count;
        seed->dscp = 2;
    } else {
        seed->interface = 0x11010001;   // eth 1/1
        seed->encap.type = PDS_ENCAP_TYPE_DOT1Q;
        seed->encap.val.vlan_tag = seed_base;
        seed->vpc_id = 1;
        std::string dst_ip = "10.1.1.1";
        std::string src_ip = "20.1.1.1";
        extract_ip_addr((char *)dst_ip.c_str(), &seed->dst_ip);
        extract_ip_addr((char *)src_ip.c_str(), &seed->src_ip);
        seed->span_id = 1;
        seed->dscp = 1;
    }
}

/// \brief Create and delete max mirror sessions in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_1) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_1<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Create, delete some and create another set of nodes in the same batch
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
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

/// \brief Create and delete mirror sessions in two batches
/// The hardware should create and delete mirror sessions correctly.
/// Validate using reads at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_4) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_4<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Create and delete mix and match of mirror sessions in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
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

/// \brief Create update, update and delete max mirror sessions in the same batch
/// a NO-OP kind of result from hardware perspective
/// [ Create SetMax, Update SetMax, Update SetMax Delete SetMax ] - Read
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

/// \brief Create delete create update and update
/// max mirror sessions in the same batch
/// Last update should be retained
/// [ Create SetMax, Delete SetMax, Create SetMax, Update SetMax,
/// Update SetMax ] - Read
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

/// \brief Create update in a batch, update in a batch
/// delete in a batch max mirror sessions
/// checking multiple updates, each in different batch
/// [ Create SetMax, Update SetMax ] - Read
/// [ Update SetMax ] - Read  [ Delete SetMax ] - Read
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

/// \brief Create in a batch, update and delete in a batch
/// Delete checking after Update
/// [ Create SetMax ] - Read [ Update SetMax , Delete SetMax] - Read
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

/// \brief Create and delete mix and match of mirror sessions in two batches
/// [ Create Set1, Set2, Set3 Delete Set1, Update Set2  ] - Read
/// [ Update Set3 - Delete  Set2, Create Set4 ] - Read
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

/// \brief Create maximum number of mirror sessions in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_neg_1<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Create more than maximum number of mirror sessions supported.
/// [ Create SetMax+1] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms, k_max_mirror_sessions+1,
                                     &seed, false, 0);
    workflow_neg_2<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Read of a non-existing mirror session should return entry not found.
/// Read NonEx
TEST_F(mirror_session_test, mirror_session_workflow_neg_3) {
    mirror_session_stepper_seed_t seed = {};

    mirror_session_stepper_seed_init(k_base_ms+8, k_max_mirror_sessions,
                                     &seed, false, 0);
    workflow_neg_3<mirror_session_util, mirror_session_stepper_seed_t>(&seed);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_4) {
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

/// \brief Create max mirror sessions in a batch,
/// delete and update in a batch which fails resulting same old state as create.
/// [ Create SetMax ] - Read - [Delete SetMax Update SetMax] - Read
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

/// \brief Updation of more than max mirror sessions
/// should fail leaving old state unchanged
/// [Create SetMax] -  Read - [Update SetMax+1] - Read
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

/// \brief Create set1 in a batch, update set1 and set2 in next batch
/// fails leaving set1 unchanged
/// [ Create Set1 ] - Read - [ Update Set1 Update Set2 ] - Read
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

/// \brief Create set1 in a batch, delete set1 and update set2 in next batch
/// fails leaving set1 unchanged
/// [ Create Set1 ] - Read - [ Delete Set1 Update Set2 ] - Read
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

static inline void
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c <hal.json> -f <apollo|artemis>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {{"config", required_argument, NULL, 'c'},
                                {"feature", required_argument, NULL, 'f'},
                                {"help", no_argument, NULL, 'h'},
                                {0, 0, 0, 0}};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, "hc:f:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            g_cfg_file = optarg;
            if (!g_cfg_file) {
                fprintf(stderr, "HAL config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'f':
            g_pipeline = std::string(optarg);
            if (g_pipeline != "apollo" &&
                g_pipeline != "artemis") {
                fprintf(stderr, "Pipeline specified is invalid\n");
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
