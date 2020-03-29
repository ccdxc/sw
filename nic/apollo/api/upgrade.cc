//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/upgrade.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"

namespace api {

static inline upg_mode_t
get_upg_init_mode(void)
{
    const char *m = getenv("UPGRADE_MODE");
    upg_mode_t mode;

    if (!m) {
       return upg_mode_t::UPGRADE_MODE_NONE;
    }
    mode = (upg_mode_t)atoi(m);
    if (mode == upg_mode_t::UPGRADE_MODE_NONE ||
        mode == upg_mode_t::UPGRADE_MODE_GRACEFUL ||
        mode == upg_mode_t::UPGRADE_MODE_HITLESS) {
        return mode;
    } else {
        SDK_ASSERT(0);
    }
}

sdk_ret_t
upg_init (pds_init_params_t *params)
{
    sdk_ret_t ret;
    pds_event_t event;
    bool shm_create;
    upg_mode_t mode;

    mode = get_upg_init_mode();

    PDS_TRACE_DEBUG("Setting bootup upgrade mode to %s",
                    mode == upg_mode_t::UPGRADE_MODE_NONE ? "NONE" :
                    mode == upg_mode_t::UPGRADE_MODE_GRACEFUL ? "Graceful" :
                    "Hitless");
    shm_create = sdk::platform::upgrade_mode_none(mode);

    // initialize upgrade state and call the upgade compatibitly checks
    // TODO: prefer this to be created by upgrade manager and every process
    // opens it and add segments to it.
    if ((g_upg_state = upg_state::factory(shm_create)) == NULL) {
        PDS_TRACE_ERR("Upgrade state creation failed");
        return SDK_RET_ERR;
    }

    // this will be processed int upgrade boot/compat-check tests only
    if (params->event_cb) {
        event.upg_params.id = UPG_EV_NONE;
        event.event_id = PDS_EVENT_ID_UPG;
        ret = params->event_cb(&event);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }
    if (shm_create) {
        g_upg_state->set_memory_profile(params->memory_profile);
    } else if (g_upg_state->memory_profile() != params->memory_profile) {
        PDS_TRACE_ERR("Upgrade scale profile mismatch");
        return SDK_RET_ERR;
    }

    // pds hal registers for upgrade events
    ret = upg_graceful_init(params);
    if (ret == SDK_RET_OK) {
        ret = upg_hitless_init(params);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade graceful/hitless init failed");
        return ret;
    }

    // nicmgr registers for upgrade events
    ret = nicmgr::upg_graceful_init();
    if (ret == SDK_RET_OK) {
        ret = nicmgr::upg_hitless_init();
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade graceful/hitless init failed");
        return ret;
    }

    return SDK_RET_OK;
}

static sdk_ret_t
upg_graceful_event_send (upg::upg_ev_params_t *params)
{
    // TODO
    return SDK_RET_OK;
}

static sdk_ret_t
upg_hitless_event_send (upg::upg_ev_params_t *params)
{
    // TODO
    return SDK_RET_OK;
}

sdk_ret_t
upg_event_send (upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;

    if (upgrade_mode_graceful(params->mode)) {
        ret = upg_graceful_event_send(params);
    } else if (upgrade_mode_hitless(params->mode)) {
        ret = upg_hitless_event_send(params);
    } else {
        SDK_ASSERT(0);
    }
    if (ret != SDK_RET_OK && ret != SDK_RET_IN_PROGRESS) {
        PDS_TRACE_ERR("Upgrade event %s failed", upg_event2str(params->id));
    }
    return ret;
}

void
upg_event_response_cb (sdk_ret_t ret, event_id_t ev_id)
{


}

void
upg_ev_thread_hdlr_register (upg_ev_graceful_t &ev)
{
    PDS_TRACE_INFO("Upgrade graceful thread event register thread_id %u",
                   ev.thread_id);
    api::g_upg_state->register_ev_thread_hdlr(ev);
}

void
upg_ev_thread_hdlr_register (upg_ev_hitless_t &ev)
{
    PDS_TRACE_INFO("Upgrade hitless thread event register thread_id %u",
                   ev.thread_id);
    api::g_upg_state->register_ev_thread_hdlr(ev);
}

}   // namespace api
