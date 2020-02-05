
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all vnic test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"

namespace test {
namespace api {

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
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// VNIC test cases implementation
//----------------------------------------------------------------------------

/// \defgroup VNIC Vnic Tests
/// @{

/// \brief VLAN TO VNIC tests
TEST_F(vnic_test, vlan_to_vnic_map_crud) {
    pds_vlan_to_vnic_map_spec_t spec = { 0 };
    pds_vlan_to_vnic_map_key_t key = { 0 };
    pds_vlan_to_vnic_map_info_t info = { 0 };

    spec.key.vlan_id = 1;
    spec.data.vnic_id = 1;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_vlan_to_vnic_map_create(&spec) == SDK_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.vlan_id = 2;
    spec.data.vnic_id = 2;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_vlan_to_vnic_map_create(&spec) == SDK_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.vlan_id = 5;
    spec.data.vnic_id = 20;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_vlan_to_vnic_map_create(&spec) == SDK_RET_OK);

    key.vlan_id = 2;
    SDK_ASSERT(pds_vlan_to_vnic_map_delete(&key) == SDK_RET_OK);

    memset(&key, 0 , sizeof(key));
    key.vlan_id = 2;
    SDK_ASSERT(pds_vlan_to_vnic_map_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 0);
    SDK_ASSERT(info.spec.data.vnic_type == 0);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.vlan_id = PDS_VLAN_ID_MAX;
    spec.data.vnic_id = 10;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_vlan_to_vnic_map_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&key, 0 , sizeof(key));
    key.vlan_id = 5;
    SDK_ASSERT(pds_vlan_to_vnic_map_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 20);
    SDK_ASSERT(info.spec.data.vnic_type == VNIC_TYPE_L2);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.vlan_id = 5;
    spec.data.vnic_id = 30;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_vlan_to_vnic_map_update(&spec) == SDK_RET_OK);

    memset(&key, 0 , sizeof(key));
    key.vlan_id = 5;
    SDK_ASSERT(pds_vlan_to_vnic_map_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 30);
    SDK_ASSERT(info.spec.data.vnic_type == VNIC_TYPE_L3);
}

/// \brief MPLS LABEL TO VNIC tests
TEST_F(vnic_test, mpls_label_to_vnic_map_crud) {
    pds_mpls_label_to_vnic_map_spec_t spec = { 0 };
    pds_mpls_label_to_vnic_map_key_t key = { 0 };
    pds_mpls_label_to_vnic_map_info_t info = { 0 };

    spec.key.mpls_label = 1;
    spec.data.vnic_id = 1;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == SDK_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 2;
    spec.data.vnic_id = 2;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == SDK_RET_OK);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 5;
    spec.data.vnic_id = 20;
    spec.data.vnic_type = VNIC_TYPE_L2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == SDK_RET_OK);

    key.mpls_label = 2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_delete(&key) == SDK_RET_OK);

    memset(&key, 0 , sizeof(key));
    key.mpls_label = 2;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 0);
    SDK_ASSERT(info.spec.data.vnic_type == 0);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = PDS_MPLS_LABEL_MAX;
    spec.data.vnic_id = 10;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&key, 0 , sizeof(key));
    key.mpls_label = 5;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.vnic_id == 20);
    SDK_ASSERT(info.spec.data.vnic_type == VNIC_TYPE_L2);

    memset(&spec, 0 , sizeof(spec)); 
    spec.key.mpls_label = 5;
    spec.data.vnic_id = 30;
    spec.data.vnic_type = VNIC_TYPE_L3;
    SDK_ASSERT(pds_mpls_label_to_vnic_map_update(&spec) == SDK_RET_OK);
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
    return api_test_program_run(argc, argv);
}
