//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/underlay_ecmp_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include <hals_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_hals_l3.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include <cstdlib>
#include <cstdio>

namespace pdsa_test {
void load_underlay_ecmp_test_output ()
{
    static underlay_ecmp_pds_mock_t g_underlay_ecmp_pds_mock;
    test_params()->test_output = &g_underlay_ecmp_pds_mock;
}

void underlay_ecmp_pds_mock_t::generate_addupd_specs(const underlay_ecmp_input_params_t& input,
                                                     batch_spec_t& pds_batch) 
{
    auto op = (op_create_) ? API_OP_CREATE : API_OP_UPDATE;

    test::api::nexthop_group_feeder nhgroup_feeder;
    nhgroup_feeder.init(PDS_NHGROUP_TYPE_UNDERLAY_ECMP,
                        input.nexthops.size(), // Num Nexthops
                        input.pathset_id,      // ID
                        PDS_MAX_NEXTHOP_GROUP);
    int i = 0;
    for (auto& nh: input.nexthops) {
        nhgroup_feeder.spec.nexthops[i].l3_if.id  = pds_ms::ms_to_pds_ifindex(nh.l3_ifindex);
        memcpy (nhgroup_feeder.spec.nexthops[i].underlay_mac, nh.l3_dest_mac.m_mac, ETH_ADDR_LEN);
        ++i;
    }
    if (!op_create_) {
        for (auto& nh: input.nexthops) {
            nhgroup_feeder.spec.nexthops[i].l3_if.id = pds_ms::ms_to_pds_ifindex(nh.l3_ifindex);
            memcpy (nhgroup_feeder.spec.nexthops[i].underlay_mac, nh.l3_dest_mac.m_mac, ETH_ADDR_LEN);
            ++i;
        }
    }
    pds_batch.emplace_back (OBJ_ID_NEXTHOP_GROUP, op);
    auto& nhgroup_spec = pds_batch.back().nhgroup;
    nhgroup_spec = nhgroup_feeder.spec; 
}

void underlay_ecmp_pds_mock_t::generate_del_specs(const underlay_ecmp_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    pds_batch.emplace_back (OBJ_ID_NEXTHOP_GROUP, API_OP_DELETE);
    pds_batch.back().intf.key.id = input.pathset_id;
}

void underlay_ecmp_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 1);
    }

    ASSERT_TRUE (pds_ret_status != mock_pds_batch_async_fail_);
    
    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pdsa_stub::cookie_t*) pds_mock->cookie;
    pdsa_stub::hal_callback((mock_pds_batch_async_fail_) ? SDK_RET_ERR : SDK_RET_OK,
                            cookie);

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        return;
    }

    auto state_ctxt = pdsa_stub::state_t::thread_context();
    auto state = state_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
}

} // End namespace pdsa_test
