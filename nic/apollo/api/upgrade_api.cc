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
    // return error if any operation is pending
    // TODO : allow repeal if any operation is pending
    if (api::g_upg_state->ev_in_progress()) {
        PDS_TRACE_ERR("Upgrade, operation in progress, retry later");
        return SDK_RET_RETRY;
    }

    // starting new, clear the status
    api::g_upg_state->clear_ev_status();
    // set ongoing spec
    api::g_upg_state->set_ev_params(params);

    api::upg_event_send(params);

    // send back the result if there are no pending reponses
    if (!api::g_upg_state->ev_in_progress()) {
        return api::g_upg_state->ev_status();
    } else {
        // reply in progress. waiting for processing threads to complete
        return SDK_RET_IN_PROGRESS;
    }
}

