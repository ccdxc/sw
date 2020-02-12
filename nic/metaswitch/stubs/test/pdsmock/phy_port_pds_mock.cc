//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/phy_port_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include <cstdlib>
#include <cstdio>

namespace pds_ms_test {

void load_phy_port_test_output ()
{
    static phy_port_pds_mock_t g_phy_port_pds_mock;
    test_params()->test_output = &g_phy_port_pds_mock;
}

pds_obj_key_t phy_port_pds_mock_t::make_l3if_key_
                            (const phy_port_input_params_t& input)
{
    return pds_ms::msidx2pdsobjkey(input.l3_if);
}

void phy_port_pds_mock_t::generate_addupd_specs(const phy_port_input_params_t& input,
                                                batch_spec_t& pds_batch)
{
    auto op = (op_create_)?API_OP_CREATE : API_OP_UPDATE;
    test::api::if_feeder if_feeder;
    pds_ifindex_t eth_ifindex;
    pds_if_spec_t spec = {0};
    spec.key = make_l3if_key_(input);
    spec.type = PDS_IF_TYPE_L3;
    spec.admin_state = input.admin_state ? PDS_IF_STATE_UP:PDS_IF_STATE_DOWN;
    eth_ifindex = ETH_IFINDEX(ETH_IF_DEFAULT_SLOT, input.phy_port,
                              ETH_IF_DEFAULT_CHILD_PORT);
    spec.l3_if_info.port = api::uuid_from_objid(eth_ifindex);
    spec.l3_if_info.encap.type = PDS_ENCAP_TYPE_NONE;
    spec.l3_if_info.encap.val.vnid = 0;
    pds_batch.emplace_back (OBJ_ID_IF, op);
    auto& if_spec = pds_batch.back().intf;
    if_spec = spec;
}

void phy_port_pds_mock_t::generate_del_specs(const phy_port_input_params_t& input,
                                          batch_spec_t& pds_batch)
{
    pds_batch.emplace_back (OBJ_ID_IF, API_OP_DELETE);
    pds_batch.back().intf.key = make_l3if_key_(input);
}

void phy_port_pds_mock_t::validate_()
{
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID), (num_if_objs_));
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID), (uint32_t)0);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        if (op_delete_) {
            // Object is removed from store synchronously for deletes
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID), (num_if_objs_-1));
        } else {
            ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID), (num_if_objs_+1));
        }
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID), (uint32_t)1);
    }

    ASSERT_NE (pds_ret_status, mock_pds_batch_async_fail_);

    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pds_ms::cookie_t*) pds_mock->cookie;
    pds_ms::hal_callback((mock_pds_batch_async_fail_) ? SDK_RET_ERR : SDK_RET_OK,
                            cookie);

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pds_ms::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID), num_if_objs_);
        ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID), (uint32_t)0);
        return;
    }

//    if (op_create_) { ++num_if_objs_;}
    if (op_delete_) { --num_if_objs_;}
    auto state_ctxt = pds_ms::state_t::thread_context();
    auto state = state_ctxt.state();
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_IF_SLAB_ID), num_if_objs_);
    ASSERT_EQ (state->get_slab_in_use (pds_ms::PDS_MS_COOKIE_SLAB_ID), (uint32_t)0);
}

} // End namespace pds_ms_test
