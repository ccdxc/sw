// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_state_pd.hpp"

namespace sdk {
namespace linkmgr {
namespace pd {

sdk_ret_t
linkmgr_state_pd::init(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    port_slab_ = slab::factory("port_pd", sdk::lib::SDK_SLAB_ID_PORT_PD,
                               sizeof(port), 8, false, true, true);

    if (NULL == port_slab_) {
        ret = SDK_RET_ERR;
    }

    return ret;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
linkmgr_state_pd *
linkmgr_state_pd::factory(void)
{
    linkmgr_state_pd *state = NULL;

    // TODO: DONOT USE new operator
    state = new linkmgr_state_pd();
    if (NULL == state) {
        return NULL;
    }

    if (state->init() != SDK_RET_OK) {
        delete state;
        return NULL;
    }

    return state;
}

}    // namespace pd
}    // namespace linkmgr
}    // namespace sdk
