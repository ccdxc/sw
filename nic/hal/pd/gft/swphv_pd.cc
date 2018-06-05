//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// sw PHV injection
//-----------------------------------------------------------------------------

#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {


// pd_swphv_inject injects software PHV into a pipeline
hal_ret_t
pd_swphv_inject (pd_func_args_t *pd_func_args)
{
    // FIXME: to be implemented for GFT
    return HAL_RET_OK;
}

// pd_swphv_get_state
// get the current state of SW phv
hal_ret_t
pd_swphv_get_state (pd_func_args_t *pd_func_args)
{
    // FIXME: to be implemented for GFT
    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
