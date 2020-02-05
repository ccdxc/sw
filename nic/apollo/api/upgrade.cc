//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"

namespace upg {

sdk_ret_t
upg_init (pds_init_params_t *params)
{
    pds_upg_spec_t spec;
    sdk_ret_t ret;
    // TODO:pass shm_create(true/false) based on bootup flags
    // prefer this to be created by upgrade manager and every process
    // opens it and add segments to it.
    bool shm_create = true;
    upg_state *ustate;

    // initialize upgrade state and call the upgade compatibitly checks
    if ((ustate = upg_state::factory(shm_create)) == NULL) {
        PDS_TRACE_ERR("Upgrade state creation failed");
        return SDK_RET_ERR;
    }
    // this will be filled only during upgrade boot/compat-check tests
    api::g_pds_state.set_upg_event_cb(params->upg_event_cb);
    if (params->upg_event_cb) {
        spec.stage = UPG_STAGE_NONE;
        ret = params->upg_event_cb(&spec);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    if (shm_create) {
        ustate->set_scale_profile(params->scale_profile);
    } else if (ustate->scale_profile() != params->scale_profile) {
        PDS_TRACE_ERR("Upgrade scale profile mismatch");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

}    // namespace upg
