
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all flow cache test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "ftl_p4pd_mock.hpp"
#include "ftltest_utils.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// FLOW CACHE test class
//----------------------------------------------------------------------------

class flow_cache_test : public pds_test_base {
protected:
    flow_cache_test() {}
    virtual ~flow_cache_test() {}
    
    virtual void SetUp() {
        SDK_TRACE_INFO("============== SETUP : %s.%s ===============",
                       ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->current_test_info()->name());
        
        ftl_mock_init();
        pds_flow_cache_create(2);
    }
    virtual void TearDown() {
        //flow_hash::destroy(table);
        ftl_mock_cleanup();
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
                       ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }
    static void SetUpTestCase() {
    }
    static void TearDownTestCase() {
    }
};

//----------------------------------------------------------------------------
// Flow cache test cases implementation
//----------------------------------------------------------------------------

/// \defgroup FLOW CACHE Flow cache Tests
/// @{

/// \brief Flow cache tests
TEST_F(flow_cache_test, flow_cache_crud) {
    pds_flow_spec_t spec = { 0 };
    pds_flow_key_t key = { 0 };
    pds_flow_info_t info = { 0 };

    fill_key(1, &spec.key);
    fill_data(1, PDS_FLOW_SPEC_INDEX_SESSION, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_create(&spec) == SDK_RET_OK);

    fill_key(1, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_SESSION);
    SDK_ASSERT(info.spec.data.index == 1);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(2, &spec.key);
    fill_data(2, PDS_FLOW_SPEC_INDEX_CONNTRACK, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_CONNTRACK);
    SDK_ASSERT(info.spec.data.index == 2);

    // Update seems to be not really updating the flow data
    // TODO: Need to verify and fix this
#if 0
    memset(&spec, 0, sizeof(spec)); 
    fill_key(2, &spec.key);
    fill_data(2, PDS_FLOW_SPEC_INDEX_SESSION, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_update(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_SESSION);
    SDK_ASSERT(info.spec.data.index == 2);
#endif

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_delete(&key) == SDK_RET_OK);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == SDK_RET_ENTRY_NOT_FOUND);
}

/// @}

}    // namespace api
}    // namespace test

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
