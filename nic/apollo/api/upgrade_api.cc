//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements pds upgrade
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"


#define INVOKE_EV_THREAD_CB(cb, ret) {                                  \
    std::list<upg::upg_event_t> ev_threads;                             \
                                                                        \
    ev_threads = api::g_upg_state->ev_threads();                        \
    for (std::list<upg::upg_event_t>::iterator ev=ev_threads.begin();   \
         ev != ev_threads.end(); ++ev) {                                \
        ret = ev->cb(NULL);                                             \
        if (ret != SDK_RET_OK) {                                        \
            break;                                                      \
        }                                                               \
    }                                                                   \
}

sdk_ret_t
pds_upgrade (pds_upg_spec_t *spec)
{
    sdk_ret_t ret;

    if (!spec) {
        return SDK_RET_INVALID_ARG;
    }
    PDS_TRACE_DEBUG("Upgrade cmd received stage %s", upg_stage2str(spec->stage));
    if (!api::g_upg_state) {
        PDS_TRACE_ERR("Upgrade, state not allocated %s", upg_stage2str(spec->stage));
        return SDK_RET_ERR;
    }

    // call the registerd functions. event invocation
    //   on pds threads can be in any order.
    //   impl will be called once all the other returns OK
    switch(spec->stage) {
    case UPG_STAGE_START:
        // return error if last stage is not none
        if (api::g_upg_state->last_stage() != UPG_STAGE_NONE) {
            PDS_TRACE_ERR("Upgrade, invalid stage, last stage %s",
                          upg_stage2str(api::g_upg_state->last_stage()));
            return SDK_RET_ERR;
        }
        INVOKE_EV_THREAD_CB(compat_check_cb, ret)
        break;
    case UPG_STAGE_SWITCHOVER:
        // return error if last stage is not rollback
        if (api::g_upg_state->last_stage() != UPG_STAGE_ROLLBACK) {
            PDS_TRACE_ERR("Upgrade, invalid stage, last stage %s",
                          upg_stage2str(api::g_upg_state->last_stage()));
            return SDK_RET_ERR;
        }
        INVOKE_EV_THREAD_CB(switchover_cb, ret)
        break;
    default:
        // nothing to do. no need to update last stage.
        return SDK_RET_OK;
    }

    // if there is a failure, call abort
    if (ret != SDK_RET_OK) {
        INVOKE_EV_THREAD_CB(abort_cb, ret)
        if (ret != SDK_RET_OK) {
            // could not reset to previous state, it is critical
            ret = sdk_ret_t::SDK_RET_UPG_CRITICAL;
        }
    } else {
        // update the last successfully completed stage
        api::g_upg_state->set_spec(spec);
    }

    return ret;
}


namespace upg {

void
upg_event_cb_register (upg_event_t &upg_ev)
{
    PDS_TRACE_INFO("Upgrade thread event register thread_id %u", upg_ev.thread_id);
    api::g_upg_state->register_ev_thread(upg_ev);
}

}    // namespace upg
