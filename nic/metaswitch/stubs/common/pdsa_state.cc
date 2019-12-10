//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include "nic/metaswitch/stubs/common/pdsa_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_if_store.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pdsa_stub {

state_t* state_t::g_state_ = nullptr;
std::mutex state_t::g_mtx_;

template<> sdk::lib::slab* slab_obj_t<cookie_t>::slab_ = nullptr;

state_t::state_t(void)
{    
    tep_slab_init(slabs_, PDSA_TEP_SLAB_ID);
    if_slab_init(slabs_, PDSA_IF_SLAB_ID);
    host_lif_slab_init(slabs_, PDSA_HOST_LIF_SLAB_ID);
    bd_slab_init(slabs_, PDSA_BD_SLAB_ID);
    vpc_slab_init (slabs_, PDSA_VPC_SLAB_ID);

    slabs_[PDSA_COOKIE_SLAB_ID].
        reset(sdk::lib::slab::factory("PDSA-COOKIE", 
                                      PDSA_COOKIE_SLAB_ID, 
                                      sizeof(cookie_t), 
                                      100, 
                                      true, true, true));
    if (unlikely(!slabs_[PDSA_COOKIE_SLAB_ID])) {
        throw Error("SLAB creation failed for Cookie");
    }
    cookie_t::set_slab(slabs_[PDSA_COOKIE_SLAB_ID].get());
}

bool 
state_init (void)
{
    try { 
        state_t::create();
    } catch (Error& e) {
        SDK_TRACE_ERR("Initialization failed - %s", e.what());
        return false;
    }
    SDK_TRACE_INFO ("State Initialization successful");
    return true;
}

void 
state_destroy (void)
{
    state_t::destroy();
}

}
