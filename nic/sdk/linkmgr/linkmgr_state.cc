// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_state.hpp"

namespace sdk {
namespace linkmgr {

sdk_ret_t
linkmgr_state::init(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    port_slab_ = slab::factory("port", sdk::lib::SDK_SLAB_ID_PORT_PD,
                               sizeof(port), 8, false, true, true);

    if (NULL == port_slab_) {
        ret = SDK_RET_ERR;
    }

    return ret;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
linkmgr_state *
linkmgr_state::factory(void)
{
    linkmgr_state *state = NULL;

    // TODO: DONOT USE new operator
    state = new linkmgr_state();
    if (NULL == state) {
        return NULL;
    }

    if (state->init() != SDK_RET_OK) {
        delete state;
        return NULL;
    }

    return state;
}

}    // namespace linkmgr
}    // namespace sdk
