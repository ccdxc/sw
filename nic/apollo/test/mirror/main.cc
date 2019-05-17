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
#include "nic/apollo/test/utils/mirror.hpp"

const char *g_cfg_file = "hal.json";
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr int k_max_mirror_sessions = PDS_MAX_MIRROR_SESSION/2;
constexpr int k_base_ms_rspan = 1;
constexpr int k_base_ms_erspan = 5;
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
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);

        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(PDS_VPC_TYPE_SUBSTRATE, 1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_util tep_obj("10.1.1.1", PDS_TEP_TYPE_WORKLOAD, encap, true);
        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);

        BATCH_START();
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.create() == sdk::SDK_RET_OK);
        TEP_CREATE(tep_obj);
        ASSERT_TRUE(device_obj.create() == sdk::SDK_RET_OK);
        BATCH_COMMIT();
    }
    static void TearDownTestCase() {
        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(PDS_VPC_TYPE_SUBSTRATE, 1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};
        tep_util tep_obj("10.1.1.1", PDS_TEP_TYPE_WORKLOAD, encap, true);
        device_util device_obj(g_device_ip, g_device_macaddr, g_gateway_ip);

        BATCH_START();
        ASSERT_TRUE(vpc_obj.del() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.del() == sdk::SDK_RET_OK);
        TEP_DELETE(tep_obj);
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
mirror_session_stepper_seed_init (int seed_base,
                                  pds_mirror_session_type_t type,
                                  mirror_session_stepper_seed_t *seed)
{
    seed->key.id = seed_base;
    seed->type = type;
    if (type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        seed->interface = 0x11010001;   // eth 1/1
        seed->encap.type = PDS_ENCAP_TYPE_DOT1Q;
        seed->encap.val.vlan_tag = seed_base;
    } else if (type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
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
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_rspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_erspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
       &seed_rspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
       &seed_erspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
}

/// \brief Create, delete some and create another set of nodes in the same batch
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_3) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    mirror_session_stepper_seed_t seed3 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, PDS_MIRROR_SESSION_TYPE_RSPAN, &seed1);
    mirror_session_stepper_seed_init(3, PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed2);
    mirror_session_stepper_seed_init(5, PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed3);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed1, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed2, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed1, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed3, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed1, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed2, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed3, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed2, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed3, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed2, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed3, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
}

/// \brief Create and delete mirror sessions in two batches
/// The hardware should create and delete mirror sessions correctly.
/// Validate using reads at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_4) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_rspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_erspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed_rspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed_erspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of mirror sessions in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_5) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    mirror_session_stepper_seed_t seed3 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed1);
    mirror_session_stepper_seed_init(3, PDS_MIRROR_SESSION_TYPE_RSPAN, &seed2);
    mirror_session_stepper_seed_init(5, PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed3);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed1, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed2, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed1, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed2, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed1, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed3, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed1, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed2, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed3, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed2, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(
        &seed3, num_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed2, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed3, num_mirror_sessions, sdk::SDK_RET_ENTRY_NOT_FOUND) ==
        sdk::SDK_RET_OK);
}

/// \brief Create maximum number of mirror sessions in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(&seed_rspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(&seed_erspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(&seed_rspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(&seed_erspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed_rspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed_erspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(&seed_rspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(&seed_erspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of mirror sessions supported.
/// [ Create SetMax+1] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);


    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_rspan, k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(
        &seed_erspan, k_max_mirror_sessions+1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_INVALID_ARG);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_rspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(
        &seed_erspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing mirror session should return entry not found.
/// Read NonEx
TEST_F(mirror_session_test, mirror_session_workflow_neg_3a) {
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);

    // trigger
    ASSERT_TRUE(mirror_session_util::many_read(&seed_rspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(&seed_erspan, k_max_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of invalid  mirror sessions should fail.
/// [Delete Invalid]
TEST_F(mirror_session_test, mirror_session_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed_rspan = {};
    mirror_session_stepper_seed_t seed_erspan = {};

    mirror_session_stepper_seed_init(k_base_ms_rspan+8,
                                     PDS_MIRROR_SESSION_TYPE_RSPAN, &seed_rspan);
    mirror_session_stepper_seed_init(k_base_ms_erspan+8,
                                     PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed_erspan);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed_rspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed_erspan,
        k_max_mirror_sessions) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(mirror_session_test, mirror_session_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    mirror_session_stepper_seed_t seed1 = {};
    mirror_session_stepper_seed_t seed2 = {};
    uint32_t num_mirror_sessions = 2;

    mirror_session_stepper_seed_init(1, PDS_MIRROR_SESSION_TYPE_RSPAN, &seed1);
    mirror_session_stepper_seed_init(9, PDS_MIRROR_SESSION_TYPE_ERSPAN, &seed2);

    // trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_create(&seed1, num_mirror_sessions) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed1, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed1, num_mirror_sessions) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed2, num_mirror_sessions) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_ENTRY_NOT_FOUND);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(
        &seed1, num_mirror_sessions, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_read(&seed2, num_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(mirror_session_util::many_delete(&seed1, num_mirror_sessions) ==
        sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(mirror_session_util::many_read(&seed1, num_mirror_sessions,
        sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

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
