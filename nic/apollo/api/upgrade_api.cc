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
    // only repeal is permitted if any operation is pending
    if (api::g_upg_state->ev_in_progress()) {
        sdk::upg::upg_ev_params_t *in_progress_params =
            api::g_upg_state->ev_params();
        if (params->id != UPG_EV_REPEAL) {
            PDS_TRACE_ERR("Upgrade, operation in progress, retry later");
            return SDK_RET_RETRY;
        }
        // stop the ongoing stage by responding error
        in_progress_params->response_cb(SDK_RET_ERR,
                                        in_progress_params->response_cookie);
        // clear of the inprogress
        api::g_upg_state->ev_clear_in_progress();
    }
    // starting new, clear the status
    api::g_upg_state->set_ev_status(SDK_RET_OK);
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

