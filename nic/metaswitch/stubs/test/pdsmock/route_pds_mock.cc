//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/route_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include <cstdlib>
#include <cstdio>

namespace pds_ms_test {

void load_route_test_output ()
{
    static route_pds_mock_t g_route_pds_mock;
    test_params()->test_output = &g_route_pds_mock;
}

void route_pds_mock_t::generate_addupd_specs(const route_input_params_t& input,
                                             batch_spec_t& pds_batch) 
{
    // For route table, its always update op
    auto op = API_OP_UPDATE;
    test::api::route_table_feeder route_table_feeder;
    route_table_feeder.init("10.1.1.1/24", IP_AF_IPV4, 1,
                            1, 1);
    route_table_feeder.spec_build(&route_table_feeder.spec);
    pds_batch.emplace_back(OBJ_ID_ROUTE_TABLE, op);
    auto& route_table = pds_batch.back().route_table;
    route_table = route_table_feeder.spec; 
}

void route_pds_mock_t::generate_del_specs(const route_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    auto op = API_OP_UPDATE;
    test::api::route_table_feeder route_table_feeder;
    route_table_feeder.init("10.1.1.1/24", IP_AF_IPV4, 0,
                            1, 1);
    route_table_feeder.spec_build(&route_table_feeder.spec);
    pds_batch.emplace_back(OBJ_ID_ROUTE_TABLE, op);
    auto& route_table = pds_batch.back().route_table;
    route_table = route_table_feeder.spec; 
}

void route_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)2);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        auto rttbl = state->route_table_store().get(1);
        if (op_delete_update_) {
            // Object is removed from store synchronously for deletes
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)2);
            ASSERT_EQ (rttbl->num_routes() , 0);
        } else {
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)2);
            ASSERT_EQ (rttbl->num_routes() , 1);
        }
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)1);
    }

    ASSERT_TRUE (pds_ret_status != mock_pds_batch_async_fail_);
    
    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pds_ms::cookie_t*) pds_mock->cookie;
    pds_ms::hal_callback((mock_pds_batch_async_fail_) ? SDK_RET_ERR : SDK_RET_OK,
                            cookie);

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)2);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
        return;
    }

    auto state_ctxt = pds_ms::state_t::thread_context();
    auto state = state_ctxt.state();
    auto rttbl = state->route_table_store().get(1);
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_RTTABLE_SLAB_ID) , (uint32_t)2);
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) , (uint32_t)0);
    if (op_delete_update_) {
        ASSERT_EQ (rttbl->num_routes() , 0);
    } else {
        ASSERT_EQ (rttbl->num_routes() , 1);
    }
}

} // End namespace pds_ms_test
