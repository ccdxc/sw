//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/event.hpp"
#include <li_fte.hpp>

extern NBB_ULONG li_proc_id;

namespace pdsa_stub {

void
hal_callback (sdk_ret_t status, const void *cookie)
{
    std::unique_ptr<cookie_t> cookie_ptr ((cookie_t*) cookie);

    SDK_TRACE_DEBUG("Async PDS HAL callback, status %d, cookie 0x%lx",
                     status, cookie);
    if (status != SDK_RET_OK) {
        SDK_TRACE_ERR("Async PDS HAL callback failure err %d", status);
        cookie_ptr->print_debug_str(); 
    } else {
        SDK_TRACE_DEBUG("Async PDS Batch success"); 
        cookie_ptr->print_debug_str(); 

        auto state_ctxt = pdsa_stub::state_t::thread_context();
        for (auto& obj_uptr: cookie_ptr->objs) {
            obj_uptr->update_store (state_ctxt.state(), false); 
            // For create/update operations the underlying obj is saved in store.
            // Release the obj ownership from cookie
            obj_uptr.release();
        }
    }
    cookie_ptr->send_ips_reply((status == SDK_RET_OK), cookie_ptr->ips_mock);
}

static void
handle_port_event (core::port_event_info_t &portev)
{
    FRL_FAULT_STATE fault_state;
    std::string ifname;
    void *worker = NULL;

    {
        auto ctx = pds_ms::mgmt_state_t::thread_context();
        if (!(ctx.state()->nbase_thread()->ready())) {
            // If event comes before nbase thread is ready then
            // ignore that event. This can happen since the event
            // subscribe is called before nbase is ready
            return;
        }
    }

    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(li_proc_id);
    NBS_GET_SHARED_DATA();

    // Get the FRL pointer
    ntl::Frl &frl = li::Fte::get().get_frl();
    frl.init_fault_state(&fault_state);
    uint32_t ifidx = pds_ms::pds_to_ms_ifindex(portev.ifindex, IF_TYPE_ETH);

    // Init the fault state
    // Set the fault state based on current link state
    if (portev.event == port_event_t::PORT_EVENT_LINK_UP) {
        fault_state.hw_link_faults = ATG_FRI_FAULT_NONE;
    } else if (portev.event == port_event_t::PORT_EVENT_LINK_DOWN) {
        fault_state.hw_link_faults = ATG_FRI_FAULT_PRESENT;
    }
    {
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto obj = state_ctxt.state()->if_store().get(ifidx);
        if (obj != nullptr) {
            worker = obj->phy_port_properties().fri_worker;
        }
    }
    if (worker != nullptr) {
        SDK_TRACE_DEBUG("Sending intf fault indication, event %u",
                         portev.event);
        frl.send_fault_ind(worker, &fault_state);
    } else {
        SDK_TRACE_DEBUG("No intf FRL worker, event %u", portev.event);
    }
            
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT    
    
    return;
}

void
hal_event_callback (sdk::ipc::ipc_msg_ptr msg, const void *ctx)
{
    core::event_t *event = (core::event_t *) msg->data();
    if (!event) {
        return;
    }
    SDK_TRACE_DEBUG("Got event id %u", event->event_id);
    switch (event->event_id) {
        case EVENT_ID_PORT_STATUS:
            handle_port_event(event->port);
            break;
        case EVENT_ID_LIF_STATUS:
            // TODO: Need to propagate LIF events to the software-IF
            break;
        default:
            break;
    }
    
    return;
}

void
ipc_init_cb (int fd, sdk::ipc::handler_ms_cb cb, void *ctx)
{
    SDK_TRACE_DEBUG("ipc init callback, fd 0x%lx", fd);
    // Register SDK ipc infra fd with metaswitch. Metaswitch calls the callback
    // function in the context of the nbase thread when there is any event
    // pending on the fd
    NBS_REGISTER_WORK_SOURCE(fd, NBS_WORK_SOURCE_FD, NBS_WORK_MASK_READ,
                             cb, ctx);
    return;
}

bool
hal_init (void)
{
    sdk::ipc::ipc_init_metaswitch(32, &ipc_init_cb);
    sdk::ipc::subscribe(EVENT_ID_PORT_STATUS, &hal_event_callback, NULL);
    return true;
}

void
hal_deinit (void)
{
    return;
}

}
