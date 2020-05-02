//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// RTSP ALG
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "rtsp_parse.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "lib/list/list.hpp"

namespace hal {
namespace plugins {
namespace alg_rtsp {

namespace sfw = hal::plugins::sfw;
namespace alg_utils = hal::plugins::alg_utils;
namespace lib = sdk::lib;

static inline void incr_parse_error(rtsp_session_t *info) {
    SDK_ATOMIC_INC_UINT32(&info->parse_errors, 1);
}

static inline bool session_state_is_reset(session_t *session) {
    return(session &&
           ((session->iflow->state == session::FLOW_TCP_STATE_RESET) ||
            (session->rflow && session->rflow->state == session::FLOW_TCP_STATE_RESET)));
}

static inline uint32_t get_num_data_sessions(alg_utils::app_session_t *app_sess) {
    dllist_ctxt_t   *lentry=NULL, *next=NULL;
    uint32_t num_data_sess = 0;

    if (!dllist_empty(&app_sess->l4_sess_lhead))
        num_data_sess += dllist_count(&app_sess->l4_sess_lhead);
    dllist_for_each_safe(lentry, next, &app_sess->app_sess_lentry)
    {
        alg_utils::app_session_t *rtsp_sess = dllist_entry(lentry,
                          alg_utils::app_session_t, app_sess_lentry);
        num_data_sess += dllist_count(&rtsp_sess->l4_sess_lhead);
    }

    return num_data_sess;
}

static inline uint32_t get_num_exp_flows(alg_utils::app_session_t *app_sess) {
    dllist_ctxt_t   *lentry = NULL, *next = NULL;
    uint32_t num_exp_flow = 0;

    if (!dllist_empty(&app_sess->exp_flow_lhead))
        num_exp_flow += dllist_count(&app_sess->exp_flow_lhead);
    dllist_for_each_safe(lentry, next, &app_sess->app_sess_lentry)
    {
        alg_utils::app_session_t *rtsp_sess = dllist_entry(lentry,
                          alg_utils::app_session_t, app_sess_lentry);
        num_exp_flow += dllist_count(&rtsp_sess->exp_flow_lhead);
    }

    return num_exp_flow;
}

/*
 * APP Session get handler
 */
fte::pipeline_action_t alg_rtsp_session_get_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    SessionGetResponse            *sess_resp = ctx.sess_get_resp();
    alg_utils::l4_alg_status_t    *l4_sess = NULL;

    if (!ctx.sess_get_resp() || ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (alg_utils::l4_alg_status_t *)alg_utils::alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_RTSP)
        return fte::PIPELINE_CONTINUE;

    sess_resp->mutable_status()->set_alg(nwsec::APP_SVC_RTSP);

    rtsp_session_t   *ctrl_sess = (rtsp_session_t *)l4_sess->info;
    if (l4_sess->isCtrl == true) {
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                                set_iscontrol(true);
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                  set_parse_errors((ctrl_sess)?ctrl_sess->parse_errors:0);
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                 set_num_data_sess(get_num_data_sessions(l4_sess->app_session));
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                 set_num_exp_flows(get_num_exp_flows(l4_sess->app_session));
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                 set_num_rtsp_sessions(dllist_count(&l4_sess->app_session->app_sess_lentry));
    } else {
        sess_resp->mutable_status()->mutable_rtsp_info()->\
                               set_iscontrol(false);
    }

    return fte::PIPELINE_CONTINUE;
}

/*
 * APP session delete handler
 */
fte::pipeline_action_t alg_rtsp_session_delete_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    alg_utils::l4_alg_status_t    *l4_sess =  NULL;
    alg_utils::app_session_t      *app_sess = NULL;

    if (ctx.role() != hal::FLOW_ROLE_INITIATOR) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (alg_utils::l4_alg_status_t *)alg_utils::alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_RTSP)
        return fte::PIPELINE_CONTINUE;

    HAL_TRACE_DEBUG("Cleaning up RTSP session for session: {}", ctx.session()->hal_handle);

    ctx.flow_log()->alg = l4_sess->alg;

    app_sess = l4_sess->app_session;
    if (l4_sess->isCtrl == true) {
        if (ctx.force_delete() || session_state_is_reset(ctx.session()) ||
            (get_num_data_sessions(l4_sess->app_session) == 1)) {
            /*
             * Clean up app session if (a) its a force delete or
             * (b) if there are no rtsp session hanging off this app session
             */
            HAL_TRACE_DEBUG("Cleanup app session");
            g_rtsp_state->cleanup_app_session(l4_sess->app_session);
            return fte::PIPELINE_CONTINUE;
        } else if ((ctx.session()->iflow->state >= session::FLOW_TCP_STATE_FIN_RCVD) ||
                   (ctx.session()->rflow &&
                    (ctx.session()->rflow->state >= session::FLOW_TCP_STATE_FIN_RCVD))) {
            alg_utils::l4_alg_status_t   *ctrl_l4_sess = g_rtsp_state->get_ctrl_l4sess(app_sess);

            /*
             * We have received FIN on control session and RTSP sessions are active
             */
            HAL_TRACE_DEBUG("Received FIN while RTSP sessions are active");
            ctx.set_feature_status(HAL_RET_INVALID_CTRL_SESSION_OP);
            // Mark this entry for deletion so we cleanup
            // when we clean up the data sessions
            if (ctrl_l4_sess)
                ctrl_l4_sess->entry.deleting = true;
            return fte::PIPELINE_END;
        } else {
            /*
             * Dont cleanup if control session is timed out
             * we need to keep it around until the data session
             * goes away
             */
             HAL_TRACE_DEBUG("Data session is alive. Bailing session ageout on control");
             ctx.set_feature_status(HAL_RET_INVALID_CTRL_SESSION_OP);
             return fte::PIPELINE_END;
        }
    }

    HAL_TRACE_DEBUG("Cleaning up L4 session");
    /*
     * Cleanup the data session that is getting timed out
     */
    g_rtsp_state->cleanup_l4_sess(l4_sess);
    if (app_sess->ctrl_app_sess != NULL &&
        (get_num_data_sessions(app_sess->ctrl_app_sess) == 1)) {
        alg_utils::l4_alg_status_t   *ctrl_l4_sess = (alg_utils::l4_alg_status_t *)dllist_entry(\
                     app_sess->ctrl_app_sess->l4_sess_lhead.next, alg_utils::l4_alg_status_t, l4_sess_lentry);
        /*
         * There are cases when the FIN is received back to back and control
         * session ends up processing it first. In those cases, we reject control
         * session cleanup. To clean up stale sessions, we check if this is the
         * single hanging session and attempt clean it up if the state is BIDIR_FIN_RCVD.
         */
        if (ctrl_l4_sess != NULL && ctrl_l4_sess->isCtrl == true &&
            ctrl_l4_sess->entry.deleting == true) {
            hal::session_t *session = hal::find_session_by_handle(ctrl_l4_sess->sess_hdl);

            if (session != NULL &&
                session->iflow->state == session::FLOW_TCP_STATE_BIDIR_FIN_RCVD) {
                if (session->fte_id == fte::fte_id()) {
                    session_delete_in_fte(session->hal_handle);
                } else {
                    fte::session_delete_async(session);
                }
            }
        }

    }

    return fte::PIPELINE_CONTINUE;
}

void rtsp_app_sess_cleanup_hdlr(alg_utils::app_session_t *app_sess) {
    if (app_sess->isCtrl == true) {
        dllist_ctxt_t   *lentry, *next;

        dllist_for_each_safe(lentry, next, &app_sess->app_sess_lentry)
        {
            alg_utils::app_session_t *rtsp_sess = dllist_entry(lentry,
                            alg_utils::app_session_t, app_sess_lentry);
            g_rtsp_state->cleanup_app_session(rtsp_sess);
        }
    } else {
        dllist_del(&app_sess->app_sess_lentry);
    }

    if (app_sess->oper != NULL) {
        HAL_TRACE_VERBOSE("Freeing Slab pointer {:p}", (void *)app_sess->oper);
        g_rtsp_state->alg_info_slab()->free((rtsp_session_t *)app_sess->oper);
    }
}

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
static hal_ret_t
expected_flow_handler(fte::ctx_t &ctx, alg_utils::expected_flow_t *entry)
{
    hal_ret_t ret = HAL_RET_OK;

    sfw::sfw_info_t *sfw_info = sfw::sfw_feature_state(ctx);
    alg_utils::l4_alg_status_t *exp_flow = container_of(entry, alg_utils::l4_alg_status_t, entry);
    rtsp_session_t *rtsp_sess = (rtsp_session_t *)exp_flow->info;

    DEBUG("expected_flow_handler key={} sess={}", ctx.key(), rtsp_sess->sess_key);

    ctx.set_feature_name(FTE_FEATURE_ALG_RTSP.c_str());
    ctx.register_feature_session_state(&exp_flow->fte_feature_state);

    flow_update_t flowupd = {type: FLOWUPD_SFW_INFO};
    flowupd.sfw_info.skip_sfw_reval = 1;
    flowupd.sfw_info.sfw_rule_id = exp_flow->rule_id;
    flowupd.sfw_info.sfw_action = (uint8_t)nwsec::SECURITY_RULE_ACTION_ALLOW;
    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update sfw action");
    }

    ctx.flow_log()->sfw_action = nwsec::SECURITY_RULE_ACTION_ALLOW;
    ctx.flow_log()->rule_id = exp_flow->rule_id;
    ctx.flow_log()->alg = exp_flow->alg;
    ctx.flow_log()->parent_session_id = exp_flow->sess_hdl;

    // skip firewall check
    sfw_info->skip_sfw = true;
    sfw_info->idle_timeout = exp_flow->idle_timeout;

    return ret;
}

/*
 * Adds expected flows for all the src/dst port pairs in the transport spec
 */
static inline hal_ret_t
add_expected_flows(fte::ctx_t &ctx, alg_utils::app_session_t *app_sess,
                   alg_utils::l4_alg_status_t *l4_sess, rtsp_transport_t *spec)
{
    hal_ret_t ret;
    alg_utils::l4_alg_status_t *exp_flow = NULL;
    hal::flow_key_t  key = {};
    rtsp_session_t *rtsp_sess = (rtsp_session_t *)app_sess->oper;

    // TODO (goli) - revisit - may be we need to preserve the flow direction
    key.svrf_id = rtsp_sess->sess_key.vrf_id;
    key.dvrf_id = rtsp_sess->sess_key.vrf_id;
    key.flow_type = spec->client_ip.af == IP_AF_IPV6 ?
        hal::FLOW_TYPE_V6 : hal::FLOW_TYPE_V4;
    key.dip = spec->server_ip.addr;
    key.sip = spec->client_ip.addr;
    key.proto = (types::IPProtocol)spec->ip_proto;

    for (key.sport = spec->client_port_start, key.dport = spec->server_port_start;
         key.sport <= spec->client_port_end && key.dport <= spec->server_port_end;
         key.dport++, key.sport++) {
        INFO("adding rtsp expected flow {}", key);
        ret = g_rtsp_state->alloc_and_insert_exp_flow(app_sess, key, &exp_flow);
        if (ret != HAL_RET_OK) {
            ERR("failed to insert expected flow ret=", ret);
            return ret;
        }

        exp_flow->entry.handler = expected_flow_handler;
        exp_flow->isCtrl = false;
        exp_flow->alg =  nwsec::APP_SVC_RTSP;
        exp_flow->info = rtsp_sess;
        exp_flow->sess_hdl = HAL_HANDLE_INVALID;
        exp_flow->rule_id = (ctx.session())?ctx.session()->sfw_rule_id:0;
        exp_flow->idle_timeout = l4_sess->idle_timeout;
    }

    return HAL_RET_OK;
}

/*
 * Returns app_session of the specified key (creates if it doesn't exist)
 */
static inline alg_utils::app_session_t *
get_rtsp_session(const rtsp_session_key_t &key,
                alg_utils::app_session_t *ctrl_app_sess)
{
    dllist_ctxt_t   *lentry, *next;

    alg_utils::app_session_t *app_sess;
    rtsp_session_t *rtsp_sess;

    if (ctrl_app_sess == NULL) {
        HAL_TRACE_ERR("Control app session cannot be null");
        return NULL;
    }

    dllist_for_each_safe(lentry, next, &ctrl_app_sess->app_sess_lentry)
    {
        alg_utils::app_session_t *rtsp_app_sess = dllist_entry(lentry,
                       alg_utils::app_session_t, app_sess_lentry);

        if (!memcmp(rtsp_app_sess->oper, &key, sizeof(rtsp_session_key_t))) {
            return rtsp_app_sess;
        }
    }

    DEBUG("get_app_session - new rtsp session key={}", key);
    app_sess = (alg_utils::app_session_t *)g_rtsp_state->app_sess_slab()->alloc();
    SDK_ASSERT(app_sess != NULL);
    rtsp_sess = (rtsp_session_t *)g_rtsp_state->alg_info_slab()->alloc();
    rtsp_sess->sess_key = key;
    app_sess->oper = rtsp_sess;
    app_sess->isCtrl = false;

    g_rtsp_state->insert_app_sess(app_sess, ctrl_app_sess);

    return app_sess;
}

static inline hal_ret_t
process_req_message(fte::ctx_t& ctx, alg_utils::app_session_t *app_sess,
                    rtsp_msg_t *msg)
{
    switch (msg->req.method) {
    case RTSP_METHOD_TEARDOWN:
        // TODO(Pavithra) We need to support handling single URI
        // teardown from the client.
        g_rtsp_state->cleanup_app_session(app_sess);
        break;
    default:
        break;
    }

    return HAL_RET_OK;
}

/*
 * Process RTSP response
 *
 * Note: We dont react to RTSP session timeout as an ALG. Here is what
 * RFC 7826 states on timeout:
 * "A server may terminate the session after one
 *  session timeout period without any client activity beyond keep-alive.
 *  When a server terminates the session context, it does so by sending a
 *  TEARDOWN request indicating the reason."
 * "The session context is normally terminated by the client sending a
 *  TEARDOWN request to the server referencing the aggregated control URI."
 */
static inline hal_ret_t
process_resp_message(fte::ctx_t& ctx, alg_utils::app_session_t *app_sess,
                     alg_utils::l4_alg_status_t *l4_sess, rtsp_msg_t *msg)
{
    hal_ret_t ret = HAL_RET_OK;

    if (msg->rsp.status_code != RTSP_STATUS_OK) {
        DEBUG("error resp status={}", msg->rsp.status_code);
        return HAL_RET_OK;
    }


    if (!msg->hdrs.valid.transport) {
        DEBUG("no transport header");
        return HAL_RET_OK;
    }

    uint8_t af = (ctx.key().flow_type == hal::FLOW_TYPE_V6) ? IP_AF_IPV6 : IP_AF_IPV4;
    // add expected flows
    for (uint8_t i = 0; i < msg->hdrs.transport.nspecs; i++) {
        rtsp_transport_t *spec = &msg->hdrs.transport.specs[i];

        // If interleaved mode is on, we
        // dont have to add expected flows
        if (spec->interleaved == true)
            continue;

        HAL_TRACE_DEBUG("Key: {} Rkey: {}", ctx.key(), ctx.get_key(FLOW_ROLE_RESPONDER));
        // Set IPs to flow IPs if not specified in the spec
        if (ip_addr_is_zero(&spec->client_ip) ||
            ip_addr_is_multicast(&spec->client_ip)) {
            spec->client_ip.af = af;
            spec->client_ip.addr = ctx.key().sip;
        }

        if (ip_addr_is_zero(&spec->server_ip)) {
            spec->server_ip.af = af;
            spec->server_ip.addr = ctx.key().dip;
        }

        ret = add_expected_flows(ctx, app_sess, l4_sess, spec);
        if (ret != HAL_RET_OK) {
            ERR("failed to add expected flow ret={}", ret);
            return ret;
        }
    }

    return ret;
}

size_t
process_control_message(void *ctxt, uint8_t *payload, size_t pkt_len)
{
    fte::ctx_t                 *ctx = (fte::ctx_t *)ctxt;
    uint32_t                    offset = 0;
    alg_utils::l4_alg_status_t *l4_sess = (alg_utils::l4_alg_status_t *)\
                             alg_utils::alg_status(ctx->feature_session_state());
    rtsp_session_t             *ctrl_sess = (rtsp_session_t *)l4_sess->info;
    alg_utils::app_session_t   *app_sess;
    rtsp_session_key_t          sess_key = ctrl_sess->sess_key;
    hal_ret_t                   ret = HAL_RET_OK;

    while (offset < pkt_len) {
        rtsp_msg_t msg = {};

        if (!rtsp_parse_msg((const char*)payload, pkt_len, &offset, &msg)) {
            HAL_TRACE_VERBOSE("failed to parse message");
            incr_parse_error(ctrl_sess);
            return pkt_len;
        }

        // noop if session id is not known
        if (!msg.hdrs.valid.session) {
            return pkt_len;
        }

        // Lookup/create session
        memcpy(sess_key.id, msg.hdrs.session.id, sizeof(sess_key.id));
        app_sess = get_rtsp_session(sess_key, l4_sess->app_session);
        SDK_ASSERT_RETURN(app_sess, HAL_RET_OOM);

        // create expected flows if it is a rtsp resp with transport header
        switch (msg.type) {
        case RTSP_MSG_REQUEST:
            ret =  process_req_message(*ctx, app_sess, &msg);
            break;
        case RTSP_MSG_RESPONSE:
            ret =  process_resp_message(*ctx, app_sess, l4_sess, &msg);
            break;
        }
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("RTSP Processing failure");
        }
    }
    return pkt_len;
}

/*
 * RTSP ALG completion handler - invoked when the session creation is done.
 */
static void rtsp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    alg_utils::l4_alg_status_t *l4_sess = alg_utils::alg_status(ctx.feature_session_state());
    SDK_ASSERT(l4_sess);

    // cleanup the session if it is an abort
    if (status == false) {
        if (l4_sess->isCtrl == true) {
            g_rtsp_state->cleanup_app_session(l4_sess->app_session);
        }
    } else if (l4_sess != NULL) {
        l4_sess->sess_hdl = ctx.session()->hal_handle;
        HAL_TRACE_DEBUG("RTSP Completion handler iscontrol session: {}", l4_sess->isCtrl);
        if (l4_sess->isCtrl == true) {
            if (!l4_sess->tcpbuf[DIR_RFLOW] && ctx.is_flow_swapped() &&
                !ctx.payload_len()) {
                // Set up TCP buffer for RFLOW
                l4_sess->tcpbuf[DIR_RFLOW] = alg_utils::tcp_buffer_t::factory(
                                          htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1,
                                          NULL, process_control_message, g_rtsp_tcp_buffer_slabs);
            }
            /*
             * This will only be executed for control channel packets that
             * would lead to opening up pinholes for RTSP data sessions.
             */
            uint8_t buff = ctx.is_flow_swapped()?1:0;
            if (ctx.payload_len() &&
                (l4_sess->tcpbuf[0] && l4_sess->tcpbuf[1]))
                l4_sess->tcpbuf[buff]->insert_segment(ctx, process_control_message);
        } else {
            /*
             * Data session flow has been installed sucessfully
             * Cleanup expected flow from the exp flow table and app
             * session list and move it to l4 session list
             */
            g_rtsp_state->move_expflow_to_l4sess(l4_sess->app_session, l4_sess);
            l4_sess->alg = nwsec::APP_SVC_RTSP;
        }
    }
}

static hal_ret_t
rtsp_new_control_session(fte::ctx_t &ctx)
{
    hal_ret_t                   ret;
    alg_utils::app_session_t   *app_sess;
    alg_utils::l4_alg_status_t *l4_sess;
    fte::flow_update_t          flowupd = {};
    sfw::sfw_info_t            *sfw_info = NULL;
    rtsp_session_t             *rtsp_info = NULL;

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        ret = g_rtsp_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
        SDK_ASSERT(ret == HAL_RET_OK);
        SDK_ASSERT(app_sess != NULL);

        app_sess->isCtrl = true; // This is the control APP session
        ret = g_rtsp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
        if (ret != HAL_RET_OK) {
            return ret;
        }
        SDK_ASSERT(l4_sess != NULL);
        l4_sess->isCtrl = true;
        rtsp_info = (rtsp_session_t *)g_rtsp_state->alg_info_slab()->alloc();
        SDK_ASSERT(rtsp_info != NULL);

        app_sess->oper = rtsp_info;
        rtsp_info->sess_key.vrf_id = ctx.key().svrf_id;
        rtsp_info->sess_key.ip.af = (ctx.key().flow_type == hal::FLOW_TYPE_V6) ?
            IP_AF_IPV6 : IP_AF_IPV4;
        rtsp_info->sess_key.ip.addr = ctx.key().dip;
        rtsp_info->sess_key.port = ctx.key().dport;
        l4_sess->info = rtsp_info;
        l4_sess->alg = nwsec::APP_SVC_RTSP;
        sfw_info = sfw::sfw_feature_state(ctx);
        l4_sess->idle_timeout = sfw_info->idle_timeout;

        if ((ctx.cpu_rxhdr()->tcp_flags & (TCP_FLAG_SYN)) == TCP_FLAG_SYN) {
            //Setup TCP buffer for IFLOW
            l4_sess->tcpbuf[DIR_IFLOW] = alg_utils::tcp_buffer_t::factory(
                                      htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1,
                                      NULL, process_control_message, g_rtsp_tcp_buffer_slabs);
        }

        /*
         * Register Feature session state & completion handler
         */
        ctx.register_completion_handler(rtsp_completion_hdlr);
        ctx.register_feature_session_state(&l4_sess->fte_feature_state);
    }

    flowupd.type = fte::FLOWUPD_MCAST_COPY;
    flowupd.mcast_info.mcast_en = 1;
    flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
    flowupd.mcast_info.proxy_mcast_ptr = 0;

    return ctx.update_flow(flowupd);
}

/*
 * RTSP Exec - process only when RTSP ALG is invoked by firewall(sfw) or
 * we are processing an expected flow.
 */
fte::pipeline_action_t alg_rtsp_exec(fte::ctx_t &ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    sfw::sfw_info_t *sfw_info;

    if (hal::g_hal_state->is_flow_aware() || ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }

    sfw_info = sfw::sfw_feature_state(ctx);

    if (sfw_info->alg_proto == nwsec::APP_SVC_RTSP && !ctx.existing_session()) {
        ret = rtsp_new_control_session(ctx);
    } else if (ctx.feature_session_state() &&
               (!ctx.existing_session() || ctx.alg_cflow())) {
        // parse the packet in the completion handler
        // Register the completion handler only for new sessions
        // or for control session parsing
        ret = ctx.register_completion_handler(rtsp_completion_hdlr);
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace alg_rtsp
} // namespace plugins
} // namespace hal
