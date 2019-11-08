//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_li.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_util.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"

namespace pdsa_stub {

void
hal_callback (uint64_t cookie)
{
    std::unique_ptr<cookie_t> cookie_ptr ((cookie_t*) cookie);

    auto state_ctxt = pdsa_stub::state_t::thread_context();
    for (auto& cookie_obj: cookie_ptr->objs) {
        cookie_obj->update_store (state_ctxt.state(), cookie_ptr->op_delete); 
    }

    if (cookie_ptr->ips == nullptr) {return;}

    // Send IPS
    NBB_CREATE_THREAD_CONTEXT
    NBS_ENTER_SHARED_CONTEXT(cookie_ptr->nbb_stub_pid);
    NBS_GET_SHARED_DATA();

    NBB_SWAP_IPS_REQ_RSP_HANDLES((NBB_IPS*)cookie_ptr->ips);
    NBB_SEND_IPS(cookie_ptr->nbb_send_pid, // bd_bdpi_cb->asc_ahsl_sj_cb->asc_ahl_sj_cb.partner_main_pid
                 cookie_ptr->nbb_qid,
                 (NBB_IPS *)cookie_ptr->ips);
    cookie_ptr->ips = nullptr;    

    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();
    NBB_DESTROY_THREAD_CONTEXT    
}

bool hal_init(void)
{
    static pdsa_stub::LiIntegSubcomponent g_pdsa_li;
    /* LI stub implementation instance needs to be registered with Metaswitch*/
    li_pen::is_initialize(&g_pdsa_li);
    // TODO: Register callback with PDS
    return true;
}

void hal_deinit(void)
{
}

}
