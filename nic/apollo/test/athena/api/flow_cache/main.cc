
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
#include "nic/apollo/api/include/athena/pds_init.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "ftl_p4pd_mock.hpp"
#include "ftltest_utils.hpp"

extern "C" {
// Function prototypes
sdk_ret_t pds_flow_cache_create(void);
void pds_flow_cache_delete(void);
void pds_flow_cache_set_core_id(uint32_t core_id);
}

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
        pds_flow_cache_create();
        pds_flow_cache_set_core_id(2);
    }
    virtual void TearDown() {
        pds_flow_cache_delete();
        ftl_mock_cleanup();
        SDK_TRACE_INFO("============== TEARDOWN : %s.%s ===============",
                       ::testing::UnitTest::GetInstance()->current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->current_test_info()->name());
    }
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
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
    pds_flow_iter_cb_arg_t iter_cb_arg = { 0 };
    pds_flow_stats_t stats = { 0 };

    fill_key(1, &spec.key);
    fill_data(1, PDS_FLOW_SPEC_INDEX_SESSION, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_create(&spec) == PDS_RET_OK);

    fill_key(1, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_SESSION);
    SDK_ASSERT(info.spec.data.index == 1);

    SDK_ASSERT(pds_flow_cache_entry_iterate(dump_flow, &iter_cb_arg) ==
               PDS_RET_OK);
    SDK_ASSERT(memcmp(&iter_cb_arg.flow_key, &key,
               sizeof(pds_flow_key_t)) == 0);
    SDK_ASSERT(iter_cb_arg.flow_appdata.index_type == PDS_FLOW_SPEC_INDEX_SESSION);
    SDK_ASSERT(iter_cb_arg.flow_appdata.index == 1);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(2, &spec.key);
    fill_data(2, PDS_FLOW_SPEC_INDEX_CONNTRACK, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_create(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_CONNTRACK);
    SDK_ASSERT(info.spec.data.index == 2);

    SDK_ASSERT(pds_flow_cache_entry_iterate(dump_flow, &iter_cb_arg) ==
               PDS_RET_OK);

    memset(&spec, 0, sizeof(spec)); 
    fill_key(2, &spec.key);
    fill_data(2, PDS_FLOW_SPEC_INDEX_SESSION, &spec.data);
    SDK_ASSERT(pds_flow_cache_entry_update(&spec) == PDS_RET_OK);

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == PDS_RET_OK);
    SDK_ASSERT(info.spec.data.index_type == PDS_FLOW_SPEC_INDEX_SESSION);
    SDK_ASSERT(info.spec.data.index == 2);

    memset(&info, 0, sizeof(info));
    fill_key(2, &key);
    SDK_ASSERT(pds_flow_cache_entry_delete(&key) == PDS_RET_OK);
    SDK_ASSERT(pds_flow_cache_entry_read(&key, &info) == PDS_RET_ENTRY_NOT_FOUND);

    SDK_ASSERT(pds_flow_cache_entry_iterate(dump_flow, &iter_cb_arg) ==
               PDS_RET_OK);

    SDK_ASSERT(pds_flow_cache_stats_get(2, &stats) == PDS_RET_OK);
    dump_stats(&stats);
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
