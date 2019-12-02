//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"

namespace pdsa_stub {

void
hal_callback (bool status, uint64_t cookie)
{
    std::unique_ptr<cookie_t> cookie_ptr ((cookie_t*) cookie);

    if (!status) {
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

    cookie_ptr->send_ips_reply(status);
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
