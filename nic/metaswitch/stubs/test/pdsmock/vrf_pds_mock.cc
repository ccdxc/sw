//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/vrf_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include <cstdlib>
#include <cstdio>

namespace pds_ms_test {

void load_vrf_test_output ()
{
    static vrf_pds_mock_t g_vrf_pds_mock;
    test_params()->test_output = &g_vrf_pds_mock;
}

void vrf_pds_mock_t::generate_addupd_specs(const vrf_input_params_t& input,
                                           batch_spec_t& pds_batch) 
{
    auto op = (op_create_)?API_OP_CREATE : API_OP_UPDATE;
    pds_batch.emplace_back (OBJ_ID_VPC, op);
    auto& vpc_spec = pds_batch.back().vpc;
    vpc_spec = input.vpc_spec; 
    if (op != API_OP_UPDATE) {
        // Add route table also to expect
        pds_batch.emplace_back (OBJ_ID_ROUTE_TABLE, op);
        auto& route_table = pds_batch.back().route_table;
        route_table = input.route_table;
    }
}

void vrf_pds_mock_t::generate_del_specs(const vrf_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    pds_batch.emplace_back (OBJ_ID_VPC, API_OP_DELETE);
    pds_batch.back().vpc.key = input.vpc_spec.key;
    // Add route table also to expect
    pds_batch.emplace_back (OBJ_ID_ROUTE_TABLE, API_OP_DELETE);
    pds_batch.back().route_table.key = input.route_table.key;
}

void vrf_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_VPC_SLAB_ID) , (uint32_t)(num_vrf_objs_));
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)0);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_FALSE (vrf_obj->properties().hal_created);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        if (op_delete_) {
            // Object is removed from store synchronously for deletes
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_VPC_SLAB_ID) , uint32_t(num_vrf_objs_-1));
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)0);
        } else if (op_create_) {
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_VPC_SLAB_ID) , (uint32_t)(num_vrf_objs_));
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)0);
        }
    }

    ASSERT_TRUE (pds_ret_status != mock_pds_batch_async_fail_);
    
    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pds_ms::cookie_t*) pds_mock->cookie;
    if (pds_mock->async) {
        pds_ms::hal_callback((mock_pds_batch_async_fail_)
                                ? SDK_RET_ERR : SDK_RET_OK, cookie);
    }

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_VPC_SLAB_ID) , (uint32_t)num_vrf_objs_);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)0);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_FALSE (vrf_obj->properties().hal_created);
        return;
    }

    auto state_ctxt = pds_ms::state_t::thread_context();
    auto state = state_ctxt.state();
    if (op_delete_) { 
        --num_vrf_objs_;
    } else {
        std::cout << "Fetching VRF for " <<  ((vrf_input_params_t*)test_params()->test_input)->vrf_id << std::endl;
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_TRUE (vrf_obj->properties().hal_created);
    }
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_VPC_SLAB_ID) , (uint32_t)num_vrf_objs_);
    if (op_delete_) {
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)0);
    } else {
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)1);
    }
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
}

} // End namespace pds_ms_test

