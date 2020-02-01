
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all conntrack test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_conntrack.h"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// CONNTRACK test class
//----------------------------------------------------------------------------

class conntrack_test : public pds_test_base {
protected:
    conntrack_test() {}
    virtual ~conntrack_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        //pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Conntrack test cases implementation
//----------------------------------------------------------------------------

/// \defgroup CONNTRACK Conntrack Tests
/// @{

/// \brief Conntrack tests
TEST_F(conntrack_test, conntrack_crud) {
    pds_conntrack_spec_t spec = { 0 };
    pds_conntrack_key_t key = { 0 };
    pds_conntrack_info_t info = { 0 };

    spec.key.conntrack_id = 1;
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_ICMP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == SDK_RET_OK);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.conntrack_id = 2;
    spec.data.flow_state = PDS_FLOW_STATE_SYN_SENT;
    spec.data.flow_type = PDS_FLOW_TYPE_TCP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == SDK_RET_OK);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.conntrack_id = 5;
    spec.data.flow_state = ESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_UDP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == SDK_RET_OK);

    key.conntrack_id = 2;
    SDK_ASSERT(pds_conntrack_state_delete(&key) == SDK_RET_OK);

    memset(&key, 0, sizeof(key));
    key.conntrack_id = 2;
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.flow_state == 0);
    SDK_ASSERT(info.spec.data.flow_type == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.conntrack_id = PDS_CONNTRACK_ID_MAX;
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_UDP;
    SDK_ASSERT(pds_conntrack_state_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&key, 0, sizeof(key));
    key.conntrack_id = 5;
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.flow_state == ESTABLISHED);
    SDK_ASSERT(info.spec.data.flow_type == PDS_FLOW_TYPE_UDP);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.conntrack_id = 5;
    spec.data.flow_state = UNESTABLISHED;
    spec.data.flow_type = PDS_FLOW_TYPE_OTHERS;
    SDK_ASSERT(pds_conntrack_state_update(&spec) == SDK_RET_OK);

    memset(&key, 0 , sizeof(key));
    key.conntrack_id = 5;
    SDK_ASSERT(pds_conntrack_state_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.flow_state == UNESTABLISHED);
    SDK_ASSERT(info.spec.data.flow_type == PDS_FLOW_TYPE_OTHERS);
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
