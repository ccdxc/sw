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
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/subnet.hpp"

const char *g_cfg_file = "hal.json";
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;
constexpr uint32_t k_max_vnic = PDS_MAX_VNIC;
constexpr uint64_t k_seed_mac = 0xa010101000000000;

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
        test_case_params_t params;
        params.cfg_file = g_cfg_file;
        params.enable_fte = false;
        pds_test_base::SetUpTestCase(params);

        pds_batch_params_t batch_params = {0};
        vpc_util vpc_obj(1, "10.0.0.0/8");
        subnet_util subnet_obj(1, 1, "10.1.0.0/16");


        BATCH_START();
        ASSERT_TRUE(vpc_obj.create() == sdk::SDK_RET_OK);
        ASSERT_TRUE(subnet_obj.create() == sdk::SDK_RET_OK);
        BATCH_COMMIT();
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC
/// @{

// update encap value to seed base
static inline void
vnic_stepper_update_seed_encap (uint32_t seed_base, pds_encap_type_t encap_type,
                                pds_encap_t *encap)
{
    encap->type = encap_type;
    switch (encap_type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        encap->val.vlan_tag = seed_base;
        break;
    case PDS_ENCAP_TYPE_QINQ:
        encap->val.qinq_tag.c_tag = seed_base;
        encap->val.qinq_tag.s_tag = seed_base + 4096;
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap->val.mpls_tag = seed_base;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        encap->val.vnid = seed_base;
        break;
    default:
        encap->val.value = seed_base;
        break;
    }
}

static inline void
vnic_stepper_seed_change (vnic_stepper_seed_t *seed,
                          pds_encap_type_t vnic_type=PDS_ENCAP_TYPE_DOT1Q,
                          pds_encap_type_t fabric_type=PDS_ENCAP_TYPE_MPLSoUDP,
                          bool src_dst_check=true)
{
    vnic_stepper_update_seed_encap(seed->id, vnic_type, &seed->vnic_encap);
    vnic_stepper_update_seed_encap(seed->id, fabric_type, &seed->fabric_encap);
    seed->mac_u64 = 0xb010101010101010;
    seed->src_dst_check = src_dst_check;
}

/// \brief Create and delete max VNICs in the same batch
/// The operation should be de-duped by framework and is
/// a NO-OP from hardware perspective
/// [ Create SetMax - Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_1) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete and create max VNICs in the same batch
/// Create and delete should be de-deduped by framework and subsequent create
/// should result in successful creation
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_2) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    VNIC_MANY_DELETE(&seed);
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();
}

/// \brief Create, delete some and create another set of TEPs in the same batch
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
TEST_F(vnic_test, vnic_workflow_3) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    vnic_stepper_seed_t seed3 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);
    VNIC_SEED_INIT(70, num_vnics, k_seed_mac, &seed3);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    VNIC_MANY_CREATE(&seed2);
    VNIC_MANY_DELETE(&seed1);
    VNIC_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed2);
    VNIC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete VNIC in two batches
/// The hardware should create and delete VNIC correctly. Validate using reads
/// at each batch end
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_4) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and delete mix and match of VNIC in two batches
/// [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read
TEST_F(vnic_test, vnic_workflow_5) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    vnic_stepper_seed_t seed3 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);
    VNIC_SEED_INIT(70, num_vnics, k_seed_mac, &seed3);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    VNIC_MANY_CREATE(&seed2);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    VNIC_MANY_CREATE(&seed3);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed2);
    VNIC_MANY_DELETE(&seed3);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete maximum VNICs in the same batch
/// The operation should be de-duped by framework and is a NO-OP
/// from hardware perspective
/// [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_6) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    // update vnic encap to qinq
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_QINQ);
    VNIC_MANY_UPDATE(&seed);
    // update vnic encap to dot1q & fabric to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, delete, create, update, update max VNICs in the same batch
/// Create and delete should be de-deduped by framework and subsequent
/// create and double update should succeed
/// [ Create Max - Delete Max - Create Max - Update Max - Update Max]
TEST_F(vnic_test, vnic_workflow_7) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    VNIC_MANY_DELETE(&seed);
    VNIC_MANY_CREATE(&seed);
    // update vnic encap to qinq
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_QINQ);
    VNIC_MANY_UPDATE(&seed);
    // update vnic encap to dot1q & fabric to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();
}

/// \brief Create and update max VNICs in same batch
/// [ Create Max - Update Max ] - Read - [ Update Max ] - Read - [ Delete Max ]
TEST_F(vnic_test, DISABLED_vnic_workflow_8) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    // update fabric encap back to mplsoudp
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_MPLSoUDP);
    VNIC_MANY_UPDATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update and delete max VNICs in same batch
/// [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_9) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create, update and delete mix and match of VNICs in different batches
/// [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read
/// - [ Update Set3 - Delete Set2 - Create Set4] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_10) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    vnic_stepper_seed_t seed3 = {};
    vnic_stepper_seed_t seed4 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);
    VNIC_SEED_INIT(70, num_vnics, k_seed_mac, &seed3);
    VNIC_SEED_INIT(100, num_vnics, k_seed_mac, &seed4);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    VNIC_MANY_CREATE(&seed2);
    VNIC_MANY_CREATE(&seed3);
    VNIC_MANY_DELETE(&seed1);
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed2, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed2);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_OK);

    BATCH_START();
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed3, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed3);
    VNIC_MANY_DELETE(&seed2);
    VNIC_MANY_CREATE(&seed4);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed3, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed4, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed3);
    VNIC_MANY_DELETE(&seed4);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
    VNIC_MANY_READ(&seed4, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create maximum number of VNICs in two batches
/// [ Create SetMax ] - [ Create SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_neg_1) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create more than maximum number of VNICs supported.
/// [ Create SetMax+1 ] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_2) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic+1, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Read of a non-existing VNIC should return entry not found.
/// Read NonEx
TEST_F(vnic_test, vnic_workflow_neg_3a) {
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Deletion of a non-existing VNICs should fail.
/// [ Delete NonEx ]
TEST_F(vnic_test, vnic_workflow_neg_3b) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();
}

/// \brief Invalid batch shouldn't affect entries of previous batch
/// [ Create Set1 ] - [ Delete Set1, Set2 ] - Read
TEST_F(vnic_test, vnic_workflow_neg_4) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    VNIC_MANY_DELETE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create a VNICs with an id which is not within the range.
TEST_F(vnic_test, vnic_workflow_corner_case_4) {}

/// \brief Update the deleted max VNICs in same batch
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
TEST_F(vnic_test, DISABLED_vnic_workflow_neg_5) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();
    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// \brief Update and read non existing VNICs
/// [ Update SetMax ] - Read
TEST_F(vnic_test, vnic_workflow_neg_6) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);

    // trigger
    BATCH_START();
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Create and then update max+1 VNICs in different batch
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
TEST_F(vnic_test, vnic_workflow_neg_7) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed = {}, new_seed = {};

    VNIC_SEED_INIT(1, k_max_vnic, k_seed_mac, &seed);
    VNIC_SEED_INIT(1, k_max_vnic+1, k_seed_mac, &new_seed);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    BATCH_START();
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&new_seed, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&new_seed);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed, sdk::SDK_RET_OK);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed);
    BATCH_COMMIT();
    VNIC_MANY_READ(&seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Update existing and non-existing VNICs in same batch
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
TEST_F(vnic_test, vnic_workflow_neg_8) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {}, new_seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &new_seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&new_seed1, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&new_seed1);
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed2, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief Delete existing and Update non-existing VNICs in same batch
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
TEST_F(vnic_test, vnic_workflow_neg_9) {
    pds_batch_params_t batch_params = {0};
    vnic_stepper_seed_t seed1 = {};
    vnic_stepper_seed_t seed2 = {};
    uint32_t num_vnics = 20;

    VNIC_SEED_INIT(10, num_vnics, k_seed_mac, &seed1);
    VNIC_SEED_INIT(40, num_vnics, k_seed_mac, &seed2);

    // trigger
    BATCH_START();
    VNIC_MANY_CREATE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);

    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    // update fabric encap to vxlan
    vnic_stepper_seed_change(&seed2, PDS_ENCAP_TYPE_DOT1Q, PDS_ENCAP_TYPE_VXLAN);
    VNIC_MANY_UPDATE(&seed2);
    BATCH_COMMIT_FAIL();
    BATCH_ABORT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_OK);
    VNIC_MANY_READ(&seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    BATCH_START();
    VNIC_MANY_DELETE(&seed1);
    BATCH_COMMIT();

    VNIC_MANY_READ(&seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
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
