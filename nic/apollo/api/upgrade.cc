//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"

namespace api {

static sdk_ret_t
upg_ev_compat_check (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_backup (upg_event_ctxt_t *upg_ev_ctxt)
{
    return impl_base::pipeline_impl()->upgrade_backup();
}

static sdk_ret_t
upg_ev_init (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_sw_quiesce (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_restore (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_switchover (upg_event_ctxt_t *upg_ev_ctxt)
{
    return impl_base::pipeline_impl()->upgrade_switchover();
}

static sdk_ret_t
upg_ev_abort (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

static sdk_ret_t
upg_ev_exit (upg_event_ctxt_t *upg_ev_ctxt)
{
    return SDK_RET_OK;
}

sdk_ret_t
upg_init (pds_init_params_t *params)
{
    sdk_ret_t ret;
    pds_event_t event;
    upg::upg_event_t upg_event;

    // fill upgrade events
    upg_event.thread_id = core::PDS_THREAD_ID_API;
    upg_event.compat_check_cb = upg_ev_compat_check;
    upg_event.backup_cb = upg_ev_backup;
    upg_event.init_cb  = upg_ev_init;
    upg_event.restore_cb = upg_ev_restore;
    upg_event.sw_quiesce_cb = upg_ev_sw_quiesce;
    upg_event.switchover_cb = upg_ev_switchover;
    upg_event.abort_cb = upg_ev_abort;
    upg_event.exit_cb  = upg_ev_exit;

    // TODO:pass shm_create(true/false) based on bootup flags
    // prefer this to be created by upgrade manager and every process
    // opens it and add segments to it.
    bool shm_create = true;

    // initialize upgrade state and call the upgade compatibitly checks
    if ((g_upg_state = upg_state::factory(shm_create)) == NULL) {
        PDS_TRACE_ERR("Upgrade state creation failed");
        return SDK_RET_ERR;
    }
    // this will be processed int upgrade boot/compat-check tests only
    if (params->event_cb) {
        event.upg_spec.stage = UPG_STAGE_NONE;
        event.event_id = PDS_EVENT_ID_UPG;
        ret = params->event_cb(&event);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    if (shm_create) {
        g_upg_state->set_scale_profile(params->scale_profile);
    } else if (g_upg_state->scale_profile() != params->scale_profile) {
        PDS_TRACE_ERR("Upgrade scale profile mismatch");
        return SDK_RET_ERR;
    }

    // register for upgrade events
    upg_event_cb_register(upg_event);

    return SDK_RET_OK;
}

}    // namespace api
