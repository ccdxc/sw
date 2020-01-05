//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pds_ms_state.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_state_init.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_cookie.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_error.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pds_ms {

state_t* state_t::g_state_ = nullptr;
std::mutex state_t::g_mtx_;

template<> sdk::lib::slab* slab_obj_t<cookie_t>::slab_ = nullptr;

state_t::state_t(void)
{    
    tep_slab_init(slabs_, PDS_MS_TEP_SLAB_ID);
    if_slab_init(slabs_, PDS_MS_IF_SLAB_ID);
    host_lif_slab_init(slabs_, PDS_MS_HOST_LIF_SLAB_ID);
    subnet_slab_init(slabs_, PDS_MS_SUBNET_SLAB_ID);
    bd_slab_init(slabs_, PDS_MS_BD_SLAB_ID);
    vpc_slab_init (slabs_, PDS_MS_VPC_SLAB_ID);
    mac_slab_init (slabs_, PDS_MS_MAC_SLAB_ID);
    route_table_slab_init (slabs_, PDS_MS_RTTABLE_SLAB_ID);

    slabs_[PDS_MS_COOKIE_SLAB_ID].
        reset(sdk::lib::slab::factory("PDS-MS-COOKIE", 
                                      PDS_MS_COOKIE_SLAB_ID, 
                                      sizeof(cookie_t), 
                                      100, 
                                      true, true, true));
    if (unlikely(!slabs_[PDS_MS_COOKIE_SLAB_ID])) {
        throw Error("SLAB creation failed for Cookie");
    }
    cookie_t::set_slab(slabs_[PDS_MS_COOKIE_SLAB_ID].get());
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
