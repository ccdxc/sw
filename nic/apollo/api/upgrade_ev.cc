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

    if (api::g_upg_state->ev_in_progress() || !api::g_upg_state->ev_more() ||
        (api::g_upg_state->ev_status() != SDK_RET_OK)) {
        return;
    }

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
}

static void
upg_hitless_additional_ev_send (sdk::upg::upg_ev_params_t *params)
{
    // TODO
    SDK_ASSERT(0);
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
        INVOKE_EV_THREAD_HDLR(ev, backup_hdlr, UPG_MSG_ID_BACKUP);
        break;
    case UPG_EV_PREPARE:
        INVOKE_EV_THREAD_HDLR(ev, linkdown_hdlr, UPG_MSG_ID_LINK_DOWN);
        // have additional events to send when the first operation completes
        // on all threads
        api::g_upg_state->set_ev_more(true);
        break;
    case UPG_EV_PREP_SWITCHOVER:
        INVOKE_EV_THREAD_HDLR(ev, prep_switchover_hdlr, UPG_MSG_ID_PREP_SWITCHOVER);
        break;
    case UPG_EV_SWITCHOVER:
        INVOKE_EV_THREAD_HDLR(ev, switchover_hdlr, UPG_MSG_ID_SWITCHOVER);
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
    return SDK_RET_OK;
}

static sdk_ret_t
upg_hitless_ev_send (sdk::upg::upg_ev_params_t *params)
{
    // TODO
    SDK_ASSERT(0);
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
    if (upgrade_mode_graceful(params->mode)) {
        upg_graceful_additional_ev_send(params);
    } else {
        upg_hitless_additional_ev_send(params);
    }
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
    if (upgrade_mode_graceful(params->mode)) {
        upg_graceful_additional_ev_send(params);
    } else {
        upg_hitless_additional_ev_send(params);
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

}   // namespace api
