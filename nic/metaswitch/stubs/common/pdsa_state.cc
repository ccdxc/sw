//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Stub state implementation
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state_init.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include "nic/metaswitch/stubs/common/pdsa_tep_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_bd_store.hpp"
#include "nic/metaswitch/stubs/common/pdsa_if_store.hpp"
#include "nic/sdk/lib/logger/logger.hpp"

namespace pdsa_stub {

state_t* state_t::g_state = nullptr;

state_t::state_t(void)
{     
    slabs_[PDSA_TEP_SLAB_ID].reset(sdk::lib::slab::factory("TEP", PDSA_TEP_SLAB_ID, 
                                   sizeof(tep_obj_t), 1000, true, true, true));
    if (!slabs_[PDSA_TEP_SLAB_ID]) {
        throw Error("SLAB creation failed for TEP");
    }
    tep_obj_t::set_slab(slabs_[PDSA_TEP_SLAB_ID].get());

    slabs_[PDSA_IF_SLAB_ID].reset(sdk::lib::slab::factory("IF", PDSA_IF_SLAB_ID, 
                                  sizeof(if_obj_t), 1000, true, true, true));
    if (!slabs_[PDSA_IF_SLAB_ID]) {
        throw Error("SLAB creation failed for IF");
    }
    if_obj_t::set_slab(slabs_[PDSA_IF_SLAB_ID].get());

    slabs_[PDSA_BD_SLAB_ID].reset(sdk::lib::slab::factory("BD", PDSA_BD_SLAB_ID, 
                                  sizeof(bd_obj_t), 1000, true, true, true));
    if (!slabs_ [PDSA_BD_SLAB_ID]) {
        throw Error("SLAB creation failed for BD");
    }
    bd_obj_t::set_slab(slabs_[PDSA_BD_SLAB_ID].get());
}

bool state_init(void)
{
    try { 
        state_t::create();
    } catch (Error& e) {
#if 0 // TODO: Requires logger library linkage
        SDK_TRACE_ERR("Initialization failed - %s", e.what());
#endif
        return false;
    }
    return true;
}

void state_destroy(void)
{
    state_t::destroy();
}

}
