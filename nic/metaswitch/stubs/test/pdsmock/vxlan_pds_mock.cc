//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/vxlan_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"

namespace pds_ms_test {

void load_vxlan_test_output ()
{
    static vxlan_pds_mock_t g_vxlan_pds_mock;
    test_params()->test_output = &g_vxlan_pds_mock;
}

void vxlan_pds_mock_t::generate_addupd_specs(const vxlan_input_params_t& input,
                                             batch_spec_t& pds_batch) 
{
    auto op = (op_create_)?API_OP_CREATE : API_OP_UPDATE;
    test::api::tep_feeder tep_feeder;
    tep_feeder.init (input.tnl_ifindex,
                     0x0,  // No DMAC for now
                     ipaddr2str (&input.dest_ip),
                     PDS_MAX_TEP, 
                     PDS_NH_TYPE_UNDERLAY_ECMP,
                     {0},   // NH Index 
                     {pds_ms::msidx2pdsobjkey(input.unh_dp_idx)});  // NH Group index
    tep_feeder.spec.type = PDS_TEP_TYPE_WORKLOAD;
    tep_feeder.spec.ip_addr = input.source_ip;
    pds_batch.emplace_back (OBJ_ID_TEP, op);
    auto& tep_spec = pds_batch.back().tep;
    tep_spec = tep_feeder.spec; 

    if (op_create_) {
        test::api::nexthop_group_feeder nhgroup_feeder;
        nhgroup_feeder.init(PDS_NHGROUP_TYPE_OVERLAY_ECMP,
                            1, //Num Nexthops
                            pds_ms::msidx2pdsobjkey(hal_oecmp_idx_),    // ID
                            PDS_MAX_NEXTHOP_GROUP);
        nhgroup_feeder.spec.nexthops[0].tep = pds_ms::msidx2pdsobjkey(input.tnl_ifindex);
        pds_batch.emplace_back (OBJ_ID_NEXTHOP_GROUP, op);
        auto& nhgroup_spec = pds_batch.back().nhgroup;
        nhgroup_spec = nhgroup_feeder.spec;
    }
}

void vxlan_pds_mock_t::generate_del_specs(const vxlan_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    pds_batch.emplace_back (OBJ_ID_NEXTHOP_GROUP, API_OP_DELETE);
    pds_batch.back().nhgroup.key = pds_ms::msidx2pdsobjkey(hal_oecmp_idx_);
    pds_batch.emplace_back (OBJ_ID_TEP, API_OP_DELETE);
    pds_batch.back().tep.key = pds_ms::msidx2pdsobjkey(input.tnl_ifindex);
}

void vxlan_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == (num_tep_objs_));
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == (num_if_objs_));
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) == 0);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        if (op_delete_) {
            // Object is removed from store synchronously for deletes
            ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == (num_if_objs_-1));
            ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == (num_tep_objs_-1));
        } else {
            ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == (num_tep_objs_+1));
            if (!op_create_ && !op_delete_) {
                // Update - No change in Tunnel interface object
                ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == (num_if_objs_));
            } else {
                ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == (num_if_objs_+1));
            }
        }
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) == 1);
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
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == num_tep_objs_);
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == num_if_objs_);
        ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) == 0);
        return;
    }

    if (op_create_) {++num_tep_objs_; ++num_if_objs_;}
    if (op_delete_) {--num_tep_objs_; --num_if_objs_;}
    auto state_ctxt = pds_ms::state_t::thread_context();
    auto state = state_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_TEP_SLAB_ID) == num_tep_objs_);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID) == num_if_objs_);
    ASSERT_TRUE (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID) == 0);
}

} // End namespace pds_ms_test
