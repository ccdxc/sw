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
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/mapping.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vcn.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

const char *g_cfg_file = "hal.json";
int g_batch_epoch = 1;    // running batch epoch

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

        pds_batch_params_t batch_params = {0};
        vcn_util vcn_obj(1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");

        batch_params.epoch = ++g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vcn_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

static inline void
vnic_stepper_seed_init (int seed_base, vnic_stepper_seed_t *seed)
{
    seed->id = seed_base;
    seed->vlan_tag = seed_base;
    seed->mpls_slot = seed_base;
    seed->mac_u64 = 0xa010101000000000;
}

/// \brief Create and delete max VNICs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax, Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create, delete and create max VNICs in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_2) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create Set1, Set2, Delete Set1, Create Set3 in same batch
/// The set1 vnic should be de-duped and set2 and set3 should be programmed
/// in the hardware
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(vnic_test, vnic_workflow_3) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    vnic_stepper_seed_t seed3 = {};
    uint32_t num_vnics = 20;

    vnic_stepper_seed_init(10, &seed1);
    vnic_stepper_seed_init(40, &seed2);
    vnic_stepper_seed_init(70, &seed3);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed1, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed2, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed3, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create and delete VNIC in two batches
/// The hardware should create and delete VNIC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_4) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Create and delete mix and match of VNIC in two batches
/// [ Create Set1, Set2 ] - Read - [Delete Set1 - Create Set3 ] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_5) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    vnic_stepper_seed_t seed3 = {};
    uint32_t num_vnics = 20;

    vnic_stepper_seed_init(10, &seed1);
    vnic_stepper_seed_init(40, &seed2);
    vnic_stepper_seed_init(70, &seed3);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed1, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed2, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed1, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed2, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed3, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed3, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create maximum number of VCINs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
}

/// \brief Create more than maximum number of VNICs supported.
/// [ Create SetMax+1] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1025;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Read of a non-existing VNIC should return entry not found.
/// Read NonEx
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_3a) {
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    ASSERT_TRUE(vnic_util::many_read(
        &seed, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

/// \brief Deletion of a non-existing VNICs should fail.
/// [Delete NonEx]
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    uint32_t num_vnics = 1024;
    vnic_stepper_seed_t seed = {};

    vnic_stepper_seed_init(1, &seed);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [Delete Set1, Set2 ] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    uint32_t num_vnics = 20;

    vnic_stepper_seed_init(10, &seed1);
    vnic_stepper_seed_init(40, &seed2);

    // Trigger
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_create(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed1, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed2, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);

    ASSERT_TRUE(vnic_util::many_read(
        &seed1, num_vnics, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_read(
        &seed2, num_vnics, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);

    // Cleanup
    batch_params.epoch = ++g_batch_epoch;
    ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
    ASSERT_TRUE(vnic_util::many_delete(&seed1, num_vnics) == sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_commit() != sdk::SDK_RET_OK);
    ASSERT_TRUE(pds_batch_abort() == sdk::SDK_RET_OK);
}

/// \brief Create a VNICs with an id which is not within the range.
TEST_F(vnic_test, DISABLED_vnic_workflow_corner_case_4) {}

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
