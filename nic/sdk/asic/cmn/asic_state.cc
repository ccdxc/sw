// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "include/sdk/base.hpp"
#include "asic/cmn/asic_state.hpp"


namespace sdk {
namespace asic {
class asic_state *g_asic_state = NULL;

//------------------------------------------------------------------------------
// (private) constructor method
//------------------------------------------------------------------------------
asic_state::asic_state(void) {
    cfg_.mempartition = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
asic_state::~asic_state(void) {
}

bool
asic_state::init (asic_cfg_t *cfg) {
    cfg_.mempartition = cfg->mempartition;
    cfg_.cfg_path     = cfg->cfg_path;
    return true;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
asic_state *
asic_state::factory(asic_cfg_t *cfg) {
    asic_state  *state;

    state = new asic_state();
    SDK_ASSERT_RETURN((state != NULL), NULL);
    if (state->init(cfg) == false) {
        delete state;
        return NULL;
    }
    return state;
}

sdk_ret_t
asic_state_init(asic_cfg_t *cfg) {
    if (g_asic_state) {
        return SDK_RET_OK;
    }

    g_asic_state = sdk::asic::asic_state::factory(cfg);
    SDK_ASSERT_TRACE_RETURN((g_asic_state != NULL), SDK_RET_INVALID_ARG,
                            "Failed to instantiate ASIC state PD");
    return SDK_RET_OK;
}


}    // namespace asic 
}    // namespace sdk 
