//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/sdk/lib/utils/time_profile.hpp"
#include "nic/apollo/api/include/athena/pds_dnat.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/test/athena/api/include/ftl_scale.hpp"
#include "ftl_p4pd_mock.hpp"
#include "ftltest_utils.hpp"

extern "C" {
// Function prototypes
pds_ret_t pds_dnat_map_create(void);
void pds_dnat_map_delete(void);
void pds_dnat_map_set_core_id(uint32_t core_id);
}

sdk_logger::trace_cb_t g_trace_cb;

namespace test {
namespace api {

//----------------------------------------------------------------------------
// DNAT MAP SCALE test class
//----------------------------------------------------------------------------

class dnat_map_scale: public ftl_scale_base {
    sdk_trace_level_e trace_level;

protected:
    sdk::utils::time_profile::time_profile_info t_info;

    dnat_map_scale() {
        trace_level = g_trace_level;
        g_trace_level = sdk::lib::SDK_TRACE_LEVEL_ERR;
        sdk::utils::time_profile::time_profile_enable = true;
    }

    ~dnat_map_scale() {
        g_trace_level = trace_level;
        sdk::utils::time_profile::time_profile_enable = false;
    }

    void SetUp() {
        ftl_scale_base::SetUp();
        ftl_mock_init();
        pds_dnat_map_create();
        pds_dnat_map_set_core_id(2);
        t_info.start();
    }

    void TearDown() {
        t_info.stop();
        SDK_TRACE_INFO("Time to %s.%s : %s",
                       ::testing::UnitTest::GetInstance()->
                       current_test_info()->test_case_name(),
                       ::testing::UnitTest::GetInstance()->
                       current_test_info()->name(),
                       t_info.print_diff().c_str());
        display_gtest_stats();
        pds_dnat_map_delete();
        ftl_mock_cleanup();
        ftl_scale_base::TearDown();
    }
public:
    void display_gtest_stats() {
        SDK_TRACE_INFO("GTest Table Stats: Entries:%d", GET_ATOMIC(table_count));
        SDK_TRACE_INFO("Test  API Stats: Insert=%d Update=%d Get=%d Remove:%d Reserve:%d Release:%d",
                       GET_ATOMIC(num_insert), GET_ATOMIC(num_update), GET_ATOMIC(num_get),
                       GET_ATOMIC(num_remove), GET_ATOMIC(num_reserve), GET_ATOMIC(num_release));
        return;
    }

    pds_ret_t create_helper(uint32_t index, pds_ret_t expret) {
        pds_dnat_mapping_spec_t spec = { 0 };

        fill_key(index, &spec.key);
        fill_data(index, &spec.data);
        auto rs = pds_dnat_map_entry_create(&spec);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        if (rs == PDS_RET_OK) {
            create_();
            table_count++;
        }
        return rs;
    }

    pds_ret_t delete_helper(uint32_t index, pds_ret_t expret) {
        pds_dnat_mapping_key_t key = { 0 };

        fill_key(index, &key);
        auto rs = pds_dnat_map_entry_delete(&key);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        if (rs == PDS_RET_OK) {
            delete_();
            table_count--;
        }
        return rs;
    }

    pds_ret_t update_helper(uint32_t index, pds_ret_t expret) {
        pds_dnat_mapping_spec_t spec = { 0 };

        fill_key(index, &spec.key);
        update_data(index, &spec.data);
        auto rs = pds_dnat_map_entry_update(&spec);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        if (rs == PDS_RET_OK) {
            update_();
        }
        return rs;
    }

    pds_ret_t read_helper(uint32_t index, pds_ret_t expret) {
        pds_dnat_mapping_key_t key = { 0 };
        pds_dnat_mapping_info_t info = { 0 };

        fill_key(index, &key);
        auto rs = pds_dnat_map_entry_read(&key, &info);;
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
        if (rs == PDS_RET_OK) {
            read_();
        }
        return rs;
    }
};

//----------------------------------------------------------------------------
// Dnat cache scale test cases implementation
//----------------------------------------------------------------------------

/// \defgroup DNAT MAP SCALE Dnat map scale tests
/// @{

/// \brief Dnat map scale tests
TEST_F(dnat_map_scale, read16) {
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, crud_16) {
    pds_ret_t rs;
    rs = create_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(16, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, crd_256) {
    pds_ret_t rs;
    rs = create_entries(256, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(256, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(256, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, delete1K) {
    pds_ret_t rs;
    rs = create_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, crd_1K) {
    pds_ret_t rs;
    rs = create_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = read_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, delete2K) {
    pds_ret_t rs;
    rs = create_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(2*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, create12K) {
    pds_ret_t rs;
    rs = create_entries(12*1024, PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

TEST_F(dnat_map_scale, DISABLED_cud_16K) {
    pds_ret_t rs;
    rs = create_entries(((16*1024)-1), PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = update_entries(((16*1024)-1), PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
    rs = delete_entries(((16*1024)-1), PDS_RET_OK);
    ASSERT_TRUE(rs == PDS_RET_OK);
}

/// @}

}    // namespace api
}    // namespace test

/// @private
int
main (int argc, char **argv)
{
    register_trace_cb(sdk_test_logger);
    return api_test_program_run(argc, argv);
}
