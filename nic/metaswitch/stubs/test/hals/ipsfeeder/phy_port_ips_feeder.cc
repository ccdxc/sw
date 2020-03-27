//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/hals/ipsfeeder/phy_port_ips_feeder.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/apollo/api/utils.hpp"

namespace pds_ms_test {

void load_phy_port_test_input ()
{
    static phy_port_ips_feeder_t g_phy_port_ips_feeder;
    test_params()->test_input = &g_phy_port_ips_feeder;
}

void phy_port_ips_feeder_t::trigger_create(void) {
    // Form spec based on input
    pds_if_spec_t spec = {0};
    spec.key = pds_ms::msidx2pdsobjkey(l3_if);
    spec.type = PDS_IF_TYPE_L3;
    spec.admin_state = admin_state ? PDS_IF_STATE_UP:PDS_IF_STATE_DOWN;
    pds_ifindex_t eth_ifindex = ETH_IFINDEX(ETH_IF_DEFAULT_SLOT, phy_port,
                              ETH_IF_DEFAULT_CHILD_PORT);
    spec.l3_if_info.port = api::uuid_from_objid(eth_ifindex);
    spec.l3_if_info.encap.type = PDS_ENCAP_TYPE_NONE;
    spec.l3_if_info.encap.val.vnid = 0;

    auto state_ctxt = pds_ms::state_t::thread_context();
    auto new_if_obj = new pds_ms::if_obj_t(ms_ifindex, spec);
    state_ctxt.state()->if_store().add_upd(ms_ifindex, new_if_obj);

    ms_iflist.push_back(ms_ifindex);

    auto add_upd = generate_add_upd_ips();
    // Need to enter MS context since this creates MS FRL woker
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(li_proc_id);
    NBS_GET_SHARED_DATA();
    pds_ms::li_is()->port_add_update(&add_upd);
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT    
}

void phy_port_ips_feeder_t::cleanup(void) {

    auto state_ctxt = pds_ms::state_t::thread_context();
    for (auto ms_ifindex: ms_iflist) {
        state_ctxt.state()->if_store().erase(ms_ifindex);
    }
}

} // End namespace
