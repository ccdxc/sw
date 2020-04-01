//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements pds upgrade
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/core.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/api/upgrade.hpp"


sdk_ret_t
pds_upgrade (sdk::upg::upg_ev_params_t *params)
{
    if (!params) {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

