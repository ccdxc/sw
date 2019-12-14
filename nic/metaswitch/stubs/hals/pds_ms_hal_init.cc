//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/event.hpp"
#include <li_fte.hpp>

namespace pdsa_stub {

void
hal_callback (sdk_ret_t status, const void *cookie)
{
    std::unique_ptr<cookie_t> cookie_ptr ((cookie_t*) cookie);

    if (status != SDK_RET_OK) {
        SDK_TRACE_ERR("Async PDS Batch failure");
        cookie_ptr->print_debug_str(); 
        return;
    }

    SDK_TRACE_DEBUG("Async PDS Batch success"); 
    cookie_ptr->print_debug_str(); 

    auto state_ctxt = pdsa_stub::state_t::thread_context();
    for (auto& obj_uptr: cookie_ptr->objs) {
        obj_uptr->update_store (state_ctxt.state(), false); 
        // For create/update operations the underlying obj is saved in store.
        // Release the obj ownership from cookie
        obj_uptr.release();
    }

    cookie_ptr->send_ips_reply(true);
}

static void
handle_port_event (core::event_t &event)
{
    FRL_FAULT_STATE fault_state;
    std::string ifname;
    void *worker = NULL;

    if (!TGD) {
        return;
    }

    // Get the FRL pointer
    ntl::Frl &frl = li::Fte::get().get_frl();
    frl.init_fault_state(&fault_state);
    uint32_t ifidx = pds_ms::pds_to_ms_ifindex(event.port.ifindex, IF_TYPE_ETH);

    // Init the fault state
    // Set the fault state based on current link state
    if (event.port.event == port_event_t::PORT_EVENT_LINK_UP) {
        fault_state.hw_link_faults = ATG_FRI_FAULT_NONE;
    } else if (event.port.event == port_event_t::PORT_EVENT_LINK_DOWN) {
        fault_state.hw_link_faults = ATG_FRI_FAULT_PRESENT;
    }
    {
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto obj = state_ctxt.state()->if_store().get(ifidx);
        if (obj != nullptr) {
            worker = obj->phy_port_properties().fri_worker;
        }
    }
    frl.send_fault_ind(worker, &fault_state);
    
    return;
}

// TODO: Placeholder function for testing. Need to change to the correct
// event callback function once its ready
void
hal_event_callback (core::event_t &event)
{
    switch (event.event_id) {
        case EVENT_ID_PORT_STATUS:
            handle_port_event(event);
            break;
        case EVENT_ID_LIF_STATUS:
            // TODO: Need to propagate LIF events to the software-IF
            break;
        default:
            break;
    }
    
    return;
}

bool hal_init(void)
{
    // TODO: Register callback with PDS
    return true;
}

void hal_deinit(void)
{
}

}
