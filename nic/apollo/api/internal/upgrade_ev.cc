//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/core.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/nicmgr/nicmgr.hpp"

namespace api {

#define INVOKE_EV_THREAD_HDLR(ev, hdlr, msg_id) {                           \
    api::upg_ev_params_t api_params;                                        \
                                                                            \
    memset(&api_params, 0, sizeof(api_params));                             \
    api_params.mode = api::g_upg_state->ev_params()->mode;                  \
    api_params.id = msg_id;                                                 \
    api::g_upg_state->set_ev_in_progress_id(msg_id);                        \
    for (; ev != ev_threads.end(); ++ev) {                                  \
        PDS_TRACE_DEBUG("Upgrade event req %s to thread %s",                \
                        api::upg_msgid2str(msg_id), ev->thread_name);       \
        api::g_upg_state->ev_incr_in_progress();                            \
        ret = ev->hdlr(&api_params);                                        \
        PDS_TRACE_DEBUG("Upgrade event rsp %s from thread %s ret %u",       \
                        api::upg_msgid2str(msg_id), ev->thread_name, ret);  \
        if (ret != SDK_RET_IN_PROGRESS) {                                   \
            api::g_upg_state->ev_decr_in_progress();                        \
        }                                                                   \
        if (ret == SDK_RET_OK || ret == SDK_RET_IN_PROGRESS) {              \
            continue;                                                       \
        } else {                                                            \
            break;                                                          \
        }                                                                   \
    }                                                                       \
}

static void
upg_graceful_additional_ev_send (sdk::upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;
    std::list<api::upg_ev_graceful_t> ev_threads;
    ev_threads = api::g_upg_state->ev_threads_hdlr_graceful();
    std::list<api::upg_ev_graceful_t>::iterator ev = ev_threads.begin();
    upg_ev_msg_id_t id = g_upg_state->ev_in_progress_id();

    // send additional requests if there are any
    if (id == UPG_MSG_ID_LINK_DOWN) {
        INVOKE_EV_THREAD_HDLR(ev, hostdev_reset_hdlr,
                              UPG_MSG_ID_HOSTDEV_RESET);
        if (ret != SDK_RET_OK && ret != SDK_RET_IN_PROGRESS) {
            PDS_TRACE_ERR("Upgrade event %s failed",
                          sdk::upg::upg_event2str(params->id));
        }
        api::g_upg_state->set_ev_status(ret);
        api::g_upg_state->set_ev_more(true);
    }
    if (id == UPG_MSG_ID_HOSTDEV_RESET) {
        INVOKE_EV_THREAD_HDLR(ev, quiesce_hdlr, UPG_MSG_ID_QUIESCE);
        if (ret != SDK_RET_OK && ret != SDK_RET_IN_PROGRESS) {
            PDS_TRACE_ERR("Upgrade event %s failed",
                          sdk::upg::upg_event2str(params->id));
        }
        api::g_upg_state->set_ev_status(ret);
        api::g_upg_state->set_ev_more(false);
    }

    if (id == UPG_MSG_ID_PRE_SWITCHOVER) {
        INVOKE_EV_THREAD_HDLR(ev, pipeline_quiesce_hdlr, UPG_MSG_ID_PIPELINE_QUIESCE);
        if (ret != SDK_RET_OK && ret != SDK_RET_IN_PROGRESS) {
            PDS_TRACE_ERR("Upgrade event %s failed",
                          sdk::upg::upg_event2str(params->id));
        }
        api::g_upg_state->set_ev_status(ret);
        api::g_upg_state->set_ev_more(false);
    }
}

static void
upg_hitless_additional_ev_send (sdk::upg::upg_ev_params_t *params)
{
    // TODO
    SDK_ASSERT(0);
}


static sdk_ret_t
upg_backup_shm_create (void)
{
    upg_shm *shm =  api::g_upg_state->backup_shm();

    if (!shm && ((shm = upg_shm::factory(true)) == NULL)) {
        PDS_TRACE_ERR("Upgrade shared memory instance creation failed");
        return SDK_RET_ERR;
    }
    api::g_upg_state->set_backup_shm(shm);
    return SDK_RET_OK;
}

static sdk_ret_t
upg_graceful_ev_send (sdk::upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;
    std::list<api::upg_ev_graceful_t> ev_threads;
    ev_threads = api::g_upg_state->ev_threads_hdlr_graceful();
    std::list<api::upg_ev_graceful_t>::iterator ev = ev_threads.begin();

    switch(params->id) {
    case UPG_EV_COMPAT_CHECK:
        INVOKE_EV_THREAD_HDLR(ev, compat_check_hdlr, UPG_MSG_ID_COMPAT_CHECK);
        break;
    case UPG_EV_START:
        ret = SDK_RET_OK;
        break;
    case UPG_EV_BACKUP:
        if ((ret = upg_backup_shm_create()) != SDK_RET_OK) {
            break;
        }
        INVOKE_EV_THREAD_HDLR(ev, backup_hdlr, UPG_MSG_ID_BACKUP);
        break;
    case UPG_EV_PREPARE:
        // this should be executed in hal first and then nicmgr in sim
        // environment. this is made sure by the thread_hdlr registration
        // function. hal is registerd first before nicmgr
        // TODO: split the events to remove the above restriction
        INVOKE_EV_THREAD_HDLR(ev, linkdown_hdlr, UPG_MSG_ID_LINK_DOWN);
        // have additional events to send when the first operation completes
        // on all threads
        api::g_upg_state->set_ev_more(true);
        break;
    case UPG_EV_PRE_SWITCHOVER:
        INVOKE_EV_THREAD_HDLR(ev, pre_switchover_hdlr, UPG_MSG_ID_PRE_SWITCHOVER);
        // have additional events to send when the first operation completes
        // on all threads
        api::g_upg_state->set_ev_more(true);
        break;
    case UPG_EV_PRE_RESPAWN:
        INVOKE_EV_THREAD_HDLR(ev, pre_respawn_hdlr, UPG_MSG_ID_PRE_RESPAWN);
        break;
    case UPG_EV_RESPAWN:
        INVOKE_EV_THREAD_HDLR(ev, respawn_hdlr, UPG_MSG_ID_RESPAWN);
        break;
    case UPG_EV_SWITCHOVER:
        ret = SDK_RET_OK;
        break;
    case UPG_EV_REPEAL:
        INVOKE_EV_THREAD_HDLR(ev, repeal_hdlr, UPG_MSG_ID_REPEAL);
        break;
    case UPG_EV_READY:
        ret = SDK_RET_OK;
        break;
    case UPG_EV_FINISH:
        INVOKE_EV_THREAD_HDLR(ev, finish_hdlr, UPG_MSG_ID_FINISH);
        break;
    default:
        //  should provide default handler. otherwise we may miss
        //  adding handlers for new events.
        PDS_TRACE_ERR("Upgrade handler for event %s not implemented",
                      sdk::upg::upg_event2str(params->id));
        SDK_ASSERT(0);
    }
    return ret;
}

static sdk_ret_t
upg_hitless_ev_send (sdk::upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;
    std::list<api::upg_ev_hitless_t> ev_threads;
    ev_threads = api::g_upg_state->ev_threads_hdlr_hitless();
    std::list<api::upg_ev_hitless_t>::iterator ev = ev_threads.begin();

    switch(params->id) {
    case UPG_EV_COMPAT_CHECK:
        INVOKE_EV_THREAD_HDLR(ev, compat_check_hdlr, UPG_MSG_ID_COMPAT_CHECK);
        break;
    case UPG_EV_START:
        ret = SDK_RET_OK;
        break;
    case UPG_EV_BACKUP:
        if ((ret = upg_backup_shm_create()) != SDK_RET_OK) {
            break;
        }
        INVOKE_EV_THREAD_HDLR(ev, backup_hdlr, UPG_MSG_ID_BACKUP);
        ret = SDK_RET_OK;
        break;
    case UPG_EV_CONFIG_REPLAY:
        ret = SDK_RET_OK;
        break;
    case UPG_EV_SYNC:
        ret = SDK_RET_OK;
        break;
    default:
        // TODO
        return SDK_RET_OK;
    }
    return ret;
}

sdk_ret_t
upg_event_send (sdk::upg::upg_ev_params_t *params)
{
    sdk_ret_t ret;

    // increment the inprogress count for to avoid duplicating the response
    // from upg_event_response_cb while we are here..
    api::g_upg_state->ev_incr_in_progress();

    if (upgrade_mode_graceful(params->mode)) {
        ret = upg_graceful_ev_send(params);
    } else if (upgrade_mode_hitless(params->mode)) {
        ret = upg_hitless_ev_send(params);
    } else {
        SDK_ASSERT(0);
    }
    if (ret != SDK_RET_OK && ret != SDK_RET_IN_PROGRESS) {
        PDS_TRACE_ERR("Upgrade event %s failed", upg_event2str(params->id));
    }
    api::g_upg_state->ev_decr_in_progress();
    api::g_upg_state->set_ev_status(ret);

    // send additional events if there are any
    while (!api::g_upg_state->ev_in_progress() &&
           api::g_upg_state->ev_more() &&
           (api::g_upg_state->ev_status() == SDK_RET_OK)) {
        api::g_upg_state->ev_incr_in_progress();
        if (upgrade_mode_graceful(params->mode)) {
            upg_graceful_additional_ev_send(params);
        } else {
            upg_hitless_additional_ev_send(params);
        }
        api::g_upg_state->ev_decr_in_progress();
    }
    ret = api::g_upg_state->ev_in_progress() ? SDK_RET_IN_PROGRESS :
        api::g_upg_state->ev_status();
    return ret;
}

void
upg_ev_process_response (sdk_ret_t ret, upg_ev_msg_id_t id)
{
    sdk::upg::upg_ev_params_t *params = api::g_upg_state->ev_params();

    PDS_TRACE_DEBUG("Upgrade event rsp-id %s inprogress-id %s ret %u",
                    api::upg_msgid2str(id),
                    api::upg_msgid2str(api::g_upg_state->ev_in_progress_id()),
                    ret);
    // ignore if ongoing not matching. can happen if there
    // is a abort request asynchronously
    if (id != api::g_upg_state->ev_in_progress_id()) {
        PDS_TRACE_INFO("Upgrade event not matching, ignoring");
        return;
    }

    // assert if no responses are pending
    SDK_ASSERT(api::g_upg_state->ev_in_progress() != 0);
    api::g_upg_state->ev_decr_in_progress();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Upgrade event %s failed", upg_event2str(params->id));
    }
    api::g_upg_state->set_ev_status(ret);

    // send additional events if there are any
    if (!api::g_upg_state->ev_in_progress() &&
        api::g_upg_state->ev_more() &&
        (api::g_upg_state->ev_status() == SDK_RET_OK)) {
        if (upgrade_mode_graceful(params->mode)) {
            upg_graceful_additional_ev_send(params);
        } else {
            upg_hitless_additional_ev_send(params);
        }
    }
    // wait for all the responses to come
    if (api::g_upg_state->ev_in_progress()) {
        return;
    }
    ret = api::g_upg_state->ev_status();
    PDS_TRACE_DEBUG("Upgrade event %s completed status %u",
                  upg_event2str(params->id), ret);
    params->response_cb(ret, params->response_cookie);
}

void
upg_ev_thread_hdlr_register (upg_ev_graceful_t &ev)
{
    PDS_TRACE_INFO("Upgrade graceful event register for thread %s",
                   ev.thread_name);
    api::g_upg_state->register_ev_thread_hdlr(ev);
}

void
upg_ev_thread_hdlr_register (upg_ev_hitless_t &ev)
{
    PDS_TRACE_INFO("Upgrade hitless event register for thread %s",
                   ev.thread_name);
    api::g_upg_state->register_ev_thread_hdlr(ev);
}

static sdk_ret_t
obj_restore_hitless (void)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::list<api::upg_ev_hitless_t> hitless_list;

    hitless_list = api::g_upg_state->ev_threads_hdlr_hitless();
    std::list<api::upg_ev_hitless_t>::iterator it = hitless_list.begin();
    for (; it != hitless_list.end(); ++it) {
        if (it->restore_hdlr) {
            ret = it->restore_hdlr(NULL);
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return ret;
}

static sdk_ret_t
obj_restore_graceful (void)
{
    sdk_ret_t ret = SDK_RET_OK;
    std::list<api::upg_ev_graceful_t> graceful_list;

    graceful_list = api::g_upg_state->ev_threads_hdlr_graceful();
    std::list<api::upg_ev_graceful_t>::iterator it = graceful_list.begin();
    for (; it != graceful_list.end(); ++it) {
        if (it->restore_hdlr) {
            ret = it->restore_hdlr(NULL);
            if (ret != SDK_RET_OK) {
                break;
            }
        }
    }
    return ret;
}

sdk_ret_t
upg_obj_restore (upg_mode_t mode)
{
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Upgrade object restore, mode %u", mode);
    if (upgrade_mode_hitless(mode)) {
        ret = obj_restore_hitless();
    } else if (upgrade_mode_graceful(mode)) {
        ret = obj_restore_graceful();
    } else {
        ret = SDK_RET_OK;
    }
    return ret;
}

}   // namespace api
