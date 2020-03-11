
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all flow session info test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/test/athena/api/flow_session_info/utils.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// SESSION INFO test class
//----------------------------------------------------------------------------

class flow_session_info_test : public pds_test_base {
protected:
    flow_session_info_test() {}
    virtual ~flow_session_info_test() {}
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
// Flow session test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW SESSION INFO Flow session info tests
/// @{

/// \brief Flow session info tests
TEST_F(flow_session_info_test, flow_session_info_crud) {
    pds_flow_session_spec_t spec = { 0 };
    pds_flow_session_key_t key = { 0 };
    pds_flow_session_info_t info = { 0 };

    fill_key(&spec.key, 2, HOST_TO_SWITCH);
    fill_data(&spec.data, 2, HOST_TO_SWITCH, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_OK);

    fill_key(&key, 2, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 5, SWITCH_TO_HOST);
    fill_data(&spec.data, 5, SWITCH_TO_HOST, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    fill_data(&spec.data, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST), CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 11, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    fill_key(&key, 5, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_delete(&key) == SDK_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 5, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) ==
               SDK_RET_ENTRY_NOT_FOUND);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, PDS_FLOW_SESSION_INFO_ID_MAX, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 0, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, PDS_FLOW_SESSION_INFO_ID_MAX, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_INVALID_ARG);

    memset(&info, 0, sizeof(info));
    fill_key(&key, 45, HOST_TO_SWITCH);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) ==
               SDK_RET_ENTRY_NOT_FOUND);

    memset(&info, 0, sizeof(info));
    memset(&spec, 0, sizeof(spec));
    fill_key(&spec.key, 40, SWITCH_TO_HOST);
    fill_data(&spec.data, 40, SWITCH_TO_HOST, CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_OK);
    fill_key(&key, 40, SWITCH_TO_HOST);
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
    memset(&spec, 0, sizeof(spec));
    memcpy(&spec.data, &info.spec.data, sizeof(pds_flow_session_data_t)); 
    fill_key(&spec.key, 40, HOST_TO_SWITCH);
    fill_data(&spec.data, 40, HOST_TO_SWITCH, UPDATE);
    SDK_ASSERT(pds_flow_session_info_update(&spec) == SDK_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 40, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(&spec.key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    fill_data(&spec.data, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST), CREATE);
    SDK_ASSERT(pds_flow_session_info_create(&spec) == SDK_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
    memset(&spec, 0, sizeof(spec));
    memcpy(&spec.data, &info.spec.data, sizeof(pds_flow_session_data_t)); 
    fill_key(&spec.key, 25, SWITCH_TO_HOST);
    fill_data(&spec.data, 25, SWITCH_TO_HOST, UPDATE);
    SDK_ASSERT(pds_flow_session_info_update(&spec) == SDK_RET_OK);
    memset(&info, 0, sizeof(info));
    fill_key(&key, 25, (HOST_TO_SWITCH | SWITCH_TO_HOST));
    SDK_ASSERT(pds_flow_session_info_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(memcmp(&info.spec.data, &spec.data,
               sizeof(pds_flow_session_data_t)) == 0);
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
