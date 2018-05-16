//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// RTSP ALG
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "rtsp_parse.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/p4/iris/include/defines.h"

namespace hal {
namespace plugins {
namespace alg_rtsp {

namespace sfw = hal::plugins::sfw;
namespace alg_utils = hal::plugins::alg_utils;


/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
static hal_ret_t
expected_flow_handler(fte::ctx_t &ctx, alg_utils::expected_flow_t *entry)
{
    hal_ret_t ret;

    sfw::sfw_info_t *sfw_info = (sfw::sfw_info_t*)ctx.feature_state(sfw::FTE_FEATURE_SFW);
    alg_utils::l4_alg_status_t *exp_flow = container_of(entry, alg_utils::l4_alg_status_t, entry);
    rtsp_session_t *rtsp_sess = (rtsp_session_t *)exp_flow->info;
    alg_utils::l4_alg_status_t *l4_sess;

    DEBUG("expected_flow_handler key={} sess={}", ctx.key(), rtsp_sess->sess_key);

    // create l4_sess for data flow
    ret = g_rtsp_state->alloc_and_insert_l4_sess(exp_flow->app_session, &l4_sess);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
    l4_sess->isCtrl = FALSE;
    l4_sess->info = rtsp_sess;
    l4_sess->alg = nwsec::APP_SVC_RTSP;
    ctx.set_feature_name(FTE_FEATURE_ALG_RTSP.c_str());
    ctx.register_feature_session_state(&l4_sess->fte_feature_state);

    // skip firewall check
    sfw_info->skip_sfw = TRUE;

    return ret;
}

/*
 * Adds expected flows for all the src/dst port pairs in the transport spec
 */
static inline hal_ret_t
add_expected_flows(alg_utils::app_session_t *app_sess, rtsp_transport_t *spec)
{
    hal_ret_t ret;
    alg_utils::l4_alg_status_t *exp_flow = NULL;
    hal::flow_key_t  key = {};
    rtsp_session_t *rtsp_sess = (rtsp_session_t *)app_sess->oper;

    // TODO (goli) - revisit - may be we need to preserve the flow direction
    key.dir = 0;
    key.svrf_id = rtsp_sess->sess_key.vrf_id;
    key.dvrf_id = rtsp_sess->sess_key.vrf_id;
    key.flow_type = spec->client_ip.af == IP_AF_IPV6 ?
        hal::FLOW_TYPE_V6 : hal::FLOW_TYPE_V4;
    key.sip = spec->server_ip.addr;
    key.dip = spec->client_ip.addr;
    key.proto = (types::IPProtocol)spec->ip_proto;

    for (key.dport = spec->client_port_start, key.sport = spec->server_port_start;
         key.dport <= spec->client_port_end && key.sport <= spec->server_port_end;
         key.dport++, key.sport++) {
        INFO("adding rtsp expected flow {}", key);
        ret = g_rtsp_state->alloc_and_insert_exp_flow(app_sess, key, &exp_flow);
        if (ret != HAL_RET_OK) {
            ERR("failed to insert expected flow ret=", ret);
            return ret;
        }

        exp_flow->entry.handler = expected_flow_handler;
        exp_flow->isCtrl = FALSE;
        exp_flow->alg =  nwsec::APP_SVC_RTSP;
        exp_flow->info = rtsp_sess;
    }

    return HAL_RET_OK;
}

/*
 * Returns app_session of the specified key (creates if it doesn't exist)
 */
static inline alg_utils::app_session_t *
get_app_session(const rtsp_session_key_t &key)
{
    hal_ret_t ret;

    alg_utils::app_session_t *app_sess;
    rtsp_session_t *rtsp_sess;

    ret = g_rtsp_state->lookup_app_sess(&key, &app_sess);
    if (ret == HAL_RET_OK) {
        return app_sess;
    }

    DEBUG("get_app_session - new rtsp session key={}", key);
    app_sess = (alg_utils::app_session_t *)g_rtsp_state->app_sess_slab()->alloc();
    rtsp_sess = (rtsp_session_t *)g_rtsp_state->alg_info_slab()->alloc();
    rtsp_sess->sess_key = key;
    rtsp_sess->timeout = DEFAULT_SESSION_TIMEOUT;

    app_sess->oper = rtsp_sess;
    g_rtsp_state->insert_app_sess(app_sess);

    return app_sess;
}

static inline hal_ret_t
process_req_message(fte::ctx_t& ctx, alg_utils::app_session_t *app_sess,
                    rtsp_msg_t *msg)
{
    switch (msg->req.method) {
    case RTSP_METHOD_TEARDOWN:
        g_rtsp_state->cleanup_app_session(app_sess);
        break;
    default:
        break;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
process_resp_message(fte::ctx_t& ctx, alg_utils::app_session_t *app_sess,
                    rtsp_msg_t *msg)
{
    hal_ret_t ret = HAL_RET_OK;
    rtsp_session_t *rtsp_sess = (rtsp_session_t *)app_sess->oper;

    if (msg->rsp.status_code != RTSP_STATUS_OK) {
        DEBUG("error resp status={}", msg->rsp.status_code);
        return HAL_RET_OK;
    }


    if (!msg->hdrs.valid.transport) {
        DEBUG("no transport header");
        return HAL_RET_OK;
    }

    if (msg->hdrs.session.timeout) {
        // TODO(goli) start aging timer
        rtsp_sess->timeout = msg->hdrs.session.timeout;
    }

    uint8_t af = (ctx.key().flow_type == hal::FLOW_TYPE_V6) ? IP_AF_IPV6 : IP_AF_IPV4;
    // add expected flows
    for (uint8_t i = 0; i < msg->hdrs.transport.nspecs; i++) {
        rtsp_transport_t *spec = &msg->hdrs.transport.specs[i];


        // Set IPs to flow IPs if not specified in the spec
        if (ip_addr_is_zero(&spec->client_ip)) {
            spec->client_ip.af = af;
            spec->client_ip.addr = ctx.key().dip;
        }

        if (ip_addr_is_zero(&spec->server_ip)) {
            spec->server_ip.af = af;
            spec->server_ip.addr = ctx.key().sip;
        }

        ret = add_expected_flows(app_sess, spec);
        if (ret != HAL_RET_OK) {
            ERR("failed to add expected flow ret={}", ret);
            return ret;
        }
    }

    return ret;
}

static inline hal_ret_t
process_control_message(fte::ctx_t& ctx, const rtsp_session_t *ctrl_sess)
{
    uint32_t offset = ctx.cpu_rxhdr()->payload_offset;

    alg_utils::app_session_t *app_sess;
    rtsp_session_key_t sess_key = ctrl_sess->sess_key;

    while (offset < ctx.pkt_len()) {
        rtsp_msg_t msg = {};

        if (!rtsp_parse_msg((const char*)ctx.pkt(), ctx.pkt_len(), &offset, &msg)) {
            // TODO(goli) hadle TCP segmentation
            ERR("failed to parse message");
            return HAL_RET_ERR;
        }

        DEBUG("rtsp control msg {}", msg);

        // noop if session id is not known
        if (!msg.hdrs.valid.session) {
            return HAL_RET_OK;
        }

        // Lookup/create session
        memcpy(sess_key.id, msg.hdrs.session.id, sizeof(sess_key.id));
        app_sess = get_app_session(sess_key);
        HAL_ASSERT_RETURN(app_sess, HAL_RET_OOM);

        // create expected flows if it is a rtsp resp with transport header
        switch (msg.type) {
        case RTSP_MSG_REQUEST:
            return process_req_message(ctx, app_sess, &msg);
        case RTSP_MSG_RESPONSE:
            return process_resp_message(ctx, app_sess, &msg);
        }
    }
    return HAL_RET_OK;
}

/*
 * RTSP ALG completion handler - invoked when the session creation is done.
 */
static void rtsp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    alg_utils::l4_alg_status_t *l4_sess = alg_utils::alg_status(ctx.feature_session_state());
    HAL_ASSERT(l4_sess);

    // cleanup the session if it is an abort
    if (status == false) {
        if (l4_sess->isCtrl) {
            g_rtsp_state->cleanup_app_session(l4_sess->app_session);
        } else {
            g_rtsp_state->cleanup_l4_sess(l4_sess);
        }
    } else {
        l4_sess->sess_hdl = ctx.session()->hal_handle;
        if (l4_sess->isCtrl) {
            // parse the payload
            process_control_message(ctx, (rtsp_session_t *)l4_sess->info);
        }
    }
}

static hal_ret_t
rtsp_new_control_session(fte::ctx_t &ctx)
{
    hal_ret_t ret;
    alg_utils::app_session_t *app_sess;
    alg_utils::l4_alg_status_t *l4_sess;
    fte::flow_update_t  flowupd = {};

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        rtsp_session_key_t sess_key = {};
        sess_key.vrf_id = ctx.key().svrf_id;
        sess_key.ip.af = (ctx.key().flow_type == hal::FLOW_TYPE_V6) ?
            IP_AF_IPV6 : IP_AF_IPV4;
        sess_key.ip.addr = ctx.key().dip;
        sess_key.port = ctx.key().dport;
        app_sess = get_app_session(sess_key);
        HAL_ASSERT_RETURN(app_sess, HAL_RET_OOM);

        ret = g_rtsp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
        if (ret != HAL_RET_OK) {
            return ret;
        }

        l4_sess->isCtrl = TRUE;
        l4_sess->info = app_sess->oper;

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

    if (ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }

    sfw_info = (sfw::sfw_info_t*)ctx.feature_state(sfw::FTE_FEATURE_SFW);

    if (sfw_info->alg_proto == nwsec::APP_SVC_RTSP && !ctx.existing_session()) {
        ret = rtsp_new_control_session(ctx);
    } else if (ctx.feature_session_state()) {
        // parse the packet in the completion handler
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
