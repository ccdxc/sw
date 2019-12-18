//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/vrf_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include <cstdlib>
#include <cstdio>

namespace pdsa_test {

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
}

void vrf_pds_mock_t::generate_del_specs(const vrf_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    pds_batch.emplace_back (OBJ_ID_VPC, API_OP_DELETE);
    pds_batch.back().vpc.key = input.vpc_spec.key;
}

void vrf_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_VPC_SLAB_ID) == (num_vrf_objs_));
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_FALSE (vrf_obj->properties().hal_created);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        if (op_delete_) {
            // Object is removed from store synchronously for deletes
            ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_VPC_SLAB_ID) == (num_vrf_objs_-1));
        } else if (op_create_) {
            ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_VPC_SLAB_ID) == (num_vrf_objs_));
        }
    }

    ASSERT_TRUE (pds_ret_status != mock_pds_batch_async_fail_);
    
    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pdsa_stub::cookie_t*) pds_mock->cookie;
    if (pds_mock->async) {
        pdsa_stub::hal_callback((mock_pds_batch_async_fail_)
                                ? SDK_RET_ERR : SDK_RET_OK, cookie);
    }

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_VPC_SLAB_ID) == num_vrf_objs_);
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_FALSE (vrf_obj->properties().hal_created);
        return;
    }

    auto state_ctxt = pdsa_stub::state_t::thread_context();
    auto state = state_ctxt.state();
    if (op_delete_) { 
        --num_vrf_objs_;
    } else {
        auto vrf_obj = state->vpc_store().get(((vrf_input_params_t*)test_params()->test_input)->vrf_id);
        ASSERT_FALSE (vrf_obj == nullptr);
        ASSERT_TRUE (vrf_obj->properties().hal_created);
    }
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_VPC_SLAB_ID) == num_vrf_objs_);
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
}

} // End namespace pdsa_test

