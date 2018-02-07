// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/gft.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

//------------------------------------------------------------------------------
// process a GFT exact match profile create request
//------------------------------------------------------------------------------
hal_ret_t
gft_exact_match_profile_create (GftExactMatchProfileSpec& spec,
                                GftExactMatchProfileResponse *rsp)
{
    return HAL_RET_OK;
}

}    // namespace hal
