//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "utils.hpp"
#include "lib/list/list.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

namespace hal {
namespace plugins {
namespace alg_ftp {

#define FTP_DATA_PORT 20

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

static void incr_parse_error(ftp_info_t *info) {
    SDK_ATOMIC_INC_UINT32(&info->parse_errors, 1);
}

static inline bool session_state_is_reset(session_t *session) {
    return(session &&
           ((session->iflow->state == session::FLOW_TCP_STATE_RESET) ||
            (session->rflow && session->rflow->state == session::FLOW_TCP_STATE_RESET)));
}

/*
 * FTP Info cleanup handler
 */
void ftpinfo_cleanup_hdlr(l4_alg_status_t *l4_sess) {
    if (l4_sess->info != NULL)
        g_ftp_state->alg_info_slab()->free((ftp_info_t *)l4_sess->info);

    if (l4_sess->sess_hdl != HAL_HANDLE_INVALID &&
        !dllist_empty(&l4_sess->fte_feature_state.session_feature_lentry))
        dllist_del(&l4_sess->fte_feature_state.session_feature_lentry);
}

/*
 * APP Session get handler
 */
fte::pipeline_action_t alg_ftp_session_get_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    SessionGetResponse            *sess_resp = ctx.sess_get_resp();
    l4_alg_status_t               *l4_sess = NULL;

    if (!ctx.sess_get_resp() || ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_FTP)
        return fte::PIPELINE_CONTINUE;

    sess_resp->mutable_status()->set_alg(nwsec::APP_SVC_FTP);

    if (l4_sess->isCtrl == true) {
        ftp_info_t *info = ((ftp_info_t *)l4_sess->info);
        if (info) {
            sess_resp->mutable_status()->mutable_ftp_info()->\
                                 set_parse_error(info->parse_errors);
        }
        sess_resp->mutable_status()->mutable_ftp_info()->\
                                set_iscontrol(true);
        sess_resp->mutable_status()->mutable_ftp_info()->\
                      set_num_data_sess(dllist_count(&l4_sess->app_session->l4_sess_lhead)-1);
        sess_resp->mutable_status()->mutable_ftp_info()->\
                      set_num_exp_flows(dllist_count(&l4_sess->app_session->exp_flow_lhead));

        if (l4_sess->tcpbuf[DIR_IFLOW]) {
            l4_sess->tcpbuf[DIR_IFLOW]->tcp_buff_to_proto(sess_resp->mutable_status()->\
                                                     mutable_ftp_info()->mutable_iflow_tcp_buf());
        }
        if (l4_sess->tcpbuf[DIR_RFLOW]) {
            l4_sess->tcpbuf[DIR_RFLOW]->tcp_buff_to_proto(sess_resp->mutable_status()->\
                                                     mutable_ftp_info()->mutable_rflow_tcp_buf());
        }

        if (info) {
            sess_resp->mutable_status()->mutable_ftp_info()->set_expected_cmd_type(\
                    (info->callback == __parse_ftp_rsp) ?
                    FTPCmdType::FTP_CMD_RESPONSE : FTPCmdType::FTP_CMD_REQUEST);
        }

        g_ftp_state->expected_flows_to_proto_buf(l4_sess->app_session,
                sess_resp->mutable_status()->mutable_ftp_info()->mutable_expected_flows());

        g_ftp_state->active_data_sessions_to_proto_buf(l4_sess->app_session,
                sess_resp->mutable_status()->mutable_ftp_info()->mutable_created_sessions());

    } else {
        sess_resp->mutable_status()->mutable_ftp_info()->\
                               set_iscontrol(false);
    }

    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
alg_ftp_sync_session_proc (fte::ctx_t &ctx, l4_alg_status_t *l4_sess)
{
    FTPALGInfo       alg_req  = ctx.sess_status()->ftp_info();
    ftp_info_t      *info     = (ftp_info_t *) l4_sess->info;
    ftp_callback_t   callback = ((alg_req.expected_cmd_type() == \
                                FTPCmdType::FTP_CMD_RESPONSE) ? __parse_ftp_rsp : __parse_ftp_req);

    // If any TCP Buff alread exits, free them up and repopulate from the sync
    if (!l4_sess->tcpbuf[DIR_IFLOW]) {
        l4_sess->tcpbuf[DIR_IFLOW]->free();
        l4_sess->tcpbuf[DIR_IFLOW] = NULL;
    }
    if (!l4_sess->tcpbuf[DIR_RFLOW]) {
        l4_sess->tcpbuf[DIR_RFLOW]->free();
        l4_sess->tcpbuf[DIR_RFLOW] = NULL;
    }

    if (alg_req.has_iflow_tcp_buf()) {
        // Setup TCP buffer for IFLOW
        l4_sess->tcpbuf[DIR_IFLOW] = tcp_buffer_t::factory(alg_req.iflow_tcp_buf(), NULL,
                                                           callback, g_ftp_tcp_buffer_slabs);
    }
    if (alg_req.has_rflow_tcp_buf()) {
        l4_sess->tcpbuf[DIR_RFLOW] = tcp_buffer_t::factory(alg_req.rflow_tcp_buf(), NULL,
                                                           callback, g_ftp_tcp_buffer_slabs);
    }

    if (info) {
        info->callback = callback;
    }

    for (int i = 0; i < alg_req.expected_flows().flow_size(); i++) {
        add_expected_flow_from_proto(ctx, l4_sess, alg_req.expected_flows().flow(i));
    }
    // Add the active sessions also as an expected flow (in the control session), so that
    // when the data session sync is given to the new node, the expected flow will be
    // converted to the active flow 
    for (int i = 0; i < alg_req.created_sessions().active_session_size(); i++) {
        add_expected_flow_from_proto(ctx, l4_sess, alg_req.created_sessions().active_session(i));
    }
    return fte::PIPELINE_CONTINUE;
}

/*
 * APP session delete handler
 */
fte::pipeline_action_t alg_ftp_session_delete_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    l4_alg_status_t               *l4_sess =  NULL;
    app_session_t                 *app_sess = NULL;

    if (ctx.role() != hal::FLOW_ROLE_INITIATOR) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_FTP)
        return fte::PIPELINE_CONTINUE;

    ctx.flow_log()->alg = l4_sess->alg;
 
    app_sess = l4_sess->app_session;
    if (l4_sess->isCtrl == true) {
        if (ctx.force_delete() == true || session_state_is_reset(ctx.session()) ||
            (dllist_count(&app_sess->l4_sess_lhead) == 1 &&
             ((l4_alg_status_t *)dllist_entry(app_sess->l4_sess_lhead.next,\
                                 l4_alg_status_t, l4_sess_lentry)) == l4_sess)) {
            /*
             * Clean up app session if (a) its a force delete or
             * (b) if we receive a RESET as a result of connection termination or
             * (c) if there are no expected flows or L4 data sessions
             * hanging off of this ctrl session.
             */
            g_ftp_state->cleanup_app_session(l4_sess->app_session);
            return fte::PIPELINE_CONTINUE;
        } else if ((ctx.session()->iflow->state >= session::FLOW_TCP_STATE_FIN_RCVD) ||
                   (ctx.session()->rflow &&
                    (ctx.session()->rflow->state >= session::FLOW_TCP_STATE_FIN_RCVD))) {
            l4_alg_status_t   *ctrl_l4_sess = g_ftp_state->get_ctrl_l4sess(app_sess);
            /*
             * We received FIN on the control session
             * and we have data sessions hanging. We cannot
             * honor this
             */
            HAL_TRACE_DEBUG("Received FIN when data session is active - bailing");
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
    /*
     * Cleanup the data session that is getting timed out
     */
    g_ftp_state->cleanup_l4_sess(l4_sess);
    if (dllist_count(&app_sess->l4_sess_lhead) == 1) {
        l4_alg_status_t   *ctrl_l4_sess = (l4_alg_status_t *)dllist_entry(\
                  app_sess->l4_sess_lhead.next, l4_alg_status_t, l4_sess_lentry);
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

/*
 * Get port: number up to delimiter
 */
static int __parse_port(const char *data, int start, uint32_t dlen,
                        char delim, uint16_t *port) {
    u_int16_t tmp_port = 0;
    uint32_t i;

    for (i = start; i < dlen; i++) {
        /* Finished? */
        if (data[i] == delim) {
            if (tmp_port == 0)
                break;
            *port = tmp_port;
            return i + 1;
        } else if (data[i] >= '0' && data[i] <= '9') {
            tmp_port = tmp_port*10 + data[i] - '0';
        } else {
            //parse error
            HAL_TRACE_ERR("__parse_port: Invalid character");
            break;
        }
    }
    return 0;
}

/*
 * Parse IPv6 address from the EPSV response/EPRT commands
 */
static int __parse_ipv6(const char *src, uint32_t dlen,
                        uint8_t *dst, u_int8_t term) {
    const char *end;
    int ret = in6_pton(src,
                min_t(uint32_t, dlen, 0xffff), dst, term, &end);
    if (ret > 0)
        return (int)(end - src);

    return 0;
}

/*
 * Parse IPv4 address from the PORT response/EPRT commands
 */
static int __parse_ipv4(const char *data, uint32_t dlen, u_int32_t array[],
                        uint32_t array_size, char sep, char term) {
    uint32_t i, len;

    memset(array, 0, sizeof(array[0])*array_size);

    /* Keep data pointing at next char. */
    for (i = 0, len = 0; len < dlen && i < array_size; len++, data++) {
        if (*data >= '0' && *data <= '9') {
            array[i] = array[i]*10 + *data - '0';
        } else if (*data == sep) {
            i++;
        } else {
            /* Unexpected character; true if it's the
               terminator (or we don't care about one)
               and we're finished. */
            if ((*data == term || !term) && i == array_size - 1)
                return len;

            HAL_TRACE_ERR("__parse_ipv4: Char {} (got {} nums) {} unexpected",
                            len, i, *data);
            return 0;
        }
    }
    HAL_TRACE_ERR("__parse_ipv4: Failed to fill {} numbers separated by {}",
                             array_size, sep);
    return 0;
}

/*
 * Parse port command to get the details
 */
static int __parse_port_cmd(const char *data, uint32_t dlen,
                            char term, uint32_t *offset,
                            ftp_info_t *ftp_info) {
    int length;
    u_int32_t array[6];

    length = __parse_ipv4(data, dlen, array, 6, ',', term);
    if (length == 0)
        return 0;

    ftp_info->dip.v4_addr =  htonl((array[3] << 24) | (array[2] << 16) |
                             (array[1] << 8) | array[0]);
    ftp_info->dport = htons((array[5] << 8) | array[4]);
    ftp_info->add_exp_flow = true;
    //ftp_info->sport = FTP_DATA_PORT;
    return length;
}

/*
 * Parse EPRT command
 */
static int __parse_eprt_cmd(const char *data, uint32_t dlen,
                            char term, uint32_t *offset,
                            ftp_info_t *ftp_info) {
    char delim;
    int length, ret=0;

    /* First character is delimiter, then "1" for IPv4 or "2" for IPv6,
       then delimiter again. */
    if (dlen <= 3) {
        // parse error
        incr_parse_error(ftp_info);
        HAL_TRACE_ERR("EPRT: command too short");
        return 0;
    }

    delim = data[0];
    if (isdigit(delim) || delim < 33 || delim > 126 || data[2] != delim) {
        //parse error
        incr_parse_error(ftp_info);
        HAL_TRACE_ERR("EPRT: Invalid delimiter {}", data[0]);
        return 0;
    }

    if ((data[1] != '1') && (data[1] != '2')) {
        //parse error
        incr_parse_error(ftp_info);
        HAL_TRACE_ERR("EPRT: invalid protocol number {}", data[1]);
        return 0;
    }


    if (data[1] == 1) {
        u_int32_t array[4];

        /* Now we have IP address. */
        length = __parse_ipv4(data + 3, dlen - 3, array, 4, '.', delim);
        if (length != 0)
            ftp_info->dip.v4_addr = htonl((array[3] << 24) | (array[2] << 16)\
                                         | (array[1] << 8) | array[0]);
    } else {
        ftp_info->isIPv6 = TRUE;
        /* Now we have IPv6 address. */
        length = __parse_ipv6(data + 3, dlen - 3,
                               ftp_info->dip.v6_addr.addr8, delim);
    }

    if (length == 0) {
        incr_parse_error(ftp_info);
        return 0;
    }


    /* Start offset includes initial "|1|", and trailing delimiter */
    ret = __parse_port(data, 3 + length + 1, dlen, delim, &ftp_info->dport);
    if (!ret)
        incr_parse_error(ftp_info);

    ftp_info->add_exp_flow = true;
    //ftp_info->sport = FTP_DATA_PORT;

    return ret;
}

/*
 * Parse PASV response
 */
static int __parse_pasv_response(const char *data, uint32_t dlen,
                                 char term, uint32_t *offset,
                                 ftp_info_t *ftp_info) {
    uint32_t i;
    int      ret=0;

    for (i = 0; i < dlen; i++)
        if (isdigit(data[i]))
            break;

    if (i == dlen)
        return 0;

    *offset += i;

    ret = __parse_port_cmd(data + i, dlen - i, 0, offset, ftp_info);
    if (!ret) {
        incr_parse_error(ftp_info);
    } else  {
        ftp_info->add_exp_flow = true;
        ftp_info->sport = 0;
        memset(&ftp_info->sip, 0, sizeof(ipvx_addr_t));
    }

    return ret;
}

/*
 * Parse EPSV response command
 */
static int __parse_epsv_response(const char *data, uint32_t dlen,
                                 char term, uint32_t *offset,
                                 ftp_info_t *ftp_info) {
    char delim;
    int  ret=0;

    /* Three delimiters. */
    if (dlen <= 3) return 0;
    delim = data[0];
    if (isdigit(delim) || delim < 33 || delim > 126 ||
        data[1] != delim || data[2] != delim)
        return 0;

    ret = __parse_port(data, 3, dlen, delim, &ftp_info->dport);
    if (!ret) {
        incr_parse_error(ftp_info);
    } else {
        ftp_info->add_exp_flow = true;
        memset(&ftp_info->sip, 0, sizeof(ipvx_addr_t));
    }

    return ret;
}

/*
 * Helper to parse the commands
 */
static int find_pattern(const char *data, uint32_t dlen,
                        const char *pattern, uint32_t plen,
                        char skip, char term, ftp_state_t state,
                        uint32_t *offset, uint32_t *matchlen,
                        ftp_info_t *info, parse_cb_t cb) {
    uint32_t i = plen;

    HAL_TRACE_DEBUG("find_pattern {}: dlen = {}", pattern, dlen);

    if (dlen <= plen) {
        /*
         * Short packet: try for partial?
         */
        if (strncasecmp(data, pattern, dlen) == 0) {
            return -1;
        } else {
            return 0;
        }
    }

    if (strncasecmp(data, pattern, plen) != 0) {
        return 0;
    }

    HAL_TRACE_DEBUG("Pattern matches!");

    /* Now we've found the constant string, try to skip
       to the 'skip' character */
    if (skip) {
        for (i = plen; data[i] != skip; i++) {
            if (i == (dlen - 1)) return -1;
        }

        /* Skip over the last character */
        i++;
    }

    HAL_TRACE_DEBUG("Skipped up to {}!\n", skip);

    *offset = i;
    if (cb != NULL)  {
        HAL_TRACE_DEBUG("Offset: {} data: {}", *offset, (data+i));
        *matchlen = cb(data + i, dlen - i, term, offset, info);
        if (!*matchlen)
            return -1;
    }

    info->state = state;
    HAL_TRACE_DEBUG("Match succeeded!");

    return 1;
}

/*
 * FTP command patterns and state transitions
 */
ftp_search_t ftp_req[FTP_MAX_REQ] = {
           __FTP_CMD("PORT", ' ',  '\r', FTP_PORT,  __parse_port_cmd),
           __FTP_CMD("EPRT", ' ',  '\r', FTP_EPRT,  __parse_eprt_cmd),
           __FTP_CMD("PASV", '\0', '\r', FTP_PASV,  NULL),
           __FTP_CMD("EPSV", '\0', '\r', FTP_EPSV,  NULL),
#ifdef LOGIN_ERR_NEEDED
           __FTP_CMD("USER", ' ',  '\r', FTP_USER,  NULL),
           __FTP_CMD("PASS", ' ',  '\r', FTP_PASS,  NULL),
#endif
};


ftp_search_t ftp_rsp[FTP_MAX_RSP] = {
           __FTP_CMD("\0",        '\0', '\0',  FTP_INIT,  NULL),  /* Parse errror - not a valid response */
           __FTP_CMD("200 PORT ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PORT Success response  */
           __FTP_CMD("200 EPRT ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_EPRT Success response  */
           __FTP_CMD("227 ",      '\0', '\0',  FTP_INIT,  __parse_pasv_response), /* FTP_PASV response */
           __FTP_CMD("229 ",      '(',  ')',   FTP_INIT,  __parse_epsv_response), /* FTP_EPSV response */
#ifdef LOGIN_ERR_NEEDED
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_USER success response */
           __FTP_CMD("331 ",      '\0', '\0',  FTP_PASS,  NULL),  /* FTP_USER Password needed response */
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PASS success response */
           __FTP_CMD("332 ",      '\0', '\0',  FTP_ACCT,  NULL),  /* FTP_PASS Acct needed response */
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_ACCT success response */
#endif
           __FTP_CMD("202 ",      '\0', '\0',  FTP_INIT,  NULL),  /* Syntax error response */
           __FTP_CMD("4",         '\0', '\0',  FTP_INIT,  NULL),  /* Transient error response */
           __FTP_CMD("5",         '\0', '\0',  FTP_INIT,  NULL),  /* Error response */
};

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, expected_flow_t *wentry) {
    sfw_info_t *sfw_info = sfw::sfw_feature_state(ctx);
    l4_alg_status_t *entry = NULL;
    ftp_info_t      *ftp_info = NULL;
    hal_ret_t        ret = HAL_RET_OK;

    entry = (l4_alg_status_t *)wentry;
    ftp_info = (ftp_info_t *)entry->info;
    if (entry->isCtrl != true && sfw_info != NULL) {
        sfw_info->skip_sfw = ftp_info->skip_sfw;
        sfw_info->idle_timeout = entry->idle_timeout;
    }
    ctx.set_feature_name(FTE_FEATURE_ALG_FTP.c_str());
    ctx.register_feature_session_state(&entry->fte_feature_state);

    flow_update_t flowupd = {type: FLOWUPD_SFW_INFO};
    flowupd.sfw_info.skip_sfw_reval = 1;
    flowupd.sfw_info.sfw_rule_id = entry->rule_id;
    flowupd.sfw_info.sfw_action = (uint8_t)nwsec::SECURITY_RULE_ACTION_ALLOW;
    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update sfw action");
    }

    ctx.flow_log()->sfw_action = nwsec::SECURITY_RULE_ACTION_ALLOW;
    ctx.flow_log()->rule_id = entry->rule_id;
    ctx.flow_log()->alg = entry->alg;
    ctx.flow_log()->parent_session_id = entry->sess_hdl;

    return HAL_RET_OK;
}

static void add_expected_flow(fte::ctx_t &ctx, l4_alg_status_t *l4_sess,
                                   ftp_info_t *info) {
    l4_alg_status_t *exp_flow = NULL;
    ftp_info_t      *data_ftp_info = NULL;
    hal::flow_key_t  key;

    /*
     * Install a new expected data flow for this control session
     * Expected data flow key - (flow_type, vrf, dip & dport)
     */
    memset(&key, 0, sizeof(hal::flow_key_t));
    key = ctx.key();
    key.sport = info->sport;
    key.dport = info->dport;
    key.sip = info->sip;
    if (!isNullip(info->dip, (info->isIPv6)?IP_PROTO_IPV6:IP_PROTO_IPV4)) {
        if ((!memcmp(&info->dip, &ctx.key().dip, sizeof(ipvx_addr_t)) || 
             !memcmp(&info->dip, &ctx.key().sip, sizeof(ipvx_addr_t))) ||
            (info->allow_mismatch_ip_address)) {
            key.dip = info->dip;
        } else {
            if (key.flow_type == hal::FLOW_TYPE_V4) {
                HAL_TRACE_ERR("Mismatch IP address not allowed dip: {} key.dip: {} -- bailing add_expected_flow",
                          ipv4addr2str(info->dip.v4_addr), ipv4addr2str(ctx.key().dip.v4_addr));
            } else {
                HAL_TRACE_ERR("Mismatch IP address not allowed dip: {} key.dip: {} -- bailing add_expected_flow",
                           info->dip.v6_addr, ctx.key().dip.v6_addr);
            }
            return;
        }
    }
    g_ftp_state->alloc_and_insert_exp_flow(l4_sess->app_session, key, &exp_flow);
    SDK_ASSERT(exp_flow != NULL);
    exp_flow->entry.handler = expected_flow_handler;
    exp_flow->isCtrl = false;
    exp_flow->alg = l4_sess->alg;
    exp_flow->rule_id = (ctx.session())?ctx.session()->sfw_rule_id:0;
    exp_flow->idle_timeout = l4_sess->idle_timeout;
    data_ftp_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
    SDK_ASSERT(data_ftp_info != NULL);

    exp_flow->info = data_ftp_info;
    data_ftp_info->skip_sfw = TRUE;
    info->add_exp_flow = false;
    HAL_TRACE_DEBUG("Adding expected flow with key: {}", key);
}

void
add_expected_flow_from_proto(fte::ctx_t &ctx, l4_alg_status_t *l4_sess,
                             const FlowGateKey &proto_key)
{
    l4_alg_status_t *exp_flow = NULL;
    ftp_info_t      *data_ftp_info = NULL;
    hal::flow_key_t  key;
    expected_flow_t  expected_flow;
    hal_ret_t        ret = HAL_RET_OK;

    memset(&expected_flow, 0, sizeof(expected_flow_t));
    memset(&key, 0, sizeof(hal::flow_key_t));

    flow_gate_key_from_proto(&expected_flow, proto_key);

    key             = ctx.key();
    key.sip.v4_addr = expected_flow.key.sip;
    key.dip.v4_addr = expected_flow.key.dip;
    key.proto       = (types::IPProtocol)expected_flow.key.proto;
    key.sport       = expected_flow.key.sport;
    key.dport       = expected_flow.key.dport;

    // Look for whether entry already exists, if exists just overwrite with values
    ret = g_ftp_state->alloc_and_insert_exp_flow(l4_sess->app_session, key, &exp_flow,
                                                 false, 0, true);
    SDK_ASSERT(exp_flow != NULL);

    if (ret != HAL_RET_ENTRY_EXISTS) {
        data_ftp_info = (ftp_info_t *) g_ftp_state->alg_info_slab()->alloc();
        SDK_ASSERT(data_ftp_info != NULL);
        data_ftp_info->skip_sfw = TRUE;
        exp_flow->info          = data_ftp_info;
    }

    exp_flow->entry.handler = expected_flow_handler;
    exp_flow->isCtrl        = false;
    exp_flow->alg           = l4_sess->alg;
    exp_flow->rule_id       = (ctx.session())?ctx.session()->sfw_rule_id:0;
    exp_flow->idle_timeout  = l4_sess->idle_timeout;

    HAL_TRACE_DEBUG("Adding expected flow with key: {}", key);
}

/*
 * Walks through list of acceptable responses, updates errors and
 * adds exp_flow for new data sessions to aid opening of pinholes.
 */
size_t __parse_ftp_rsp(void *ctxt, uint8_t *payload, size_t data_len) {
    fte::ctx_t      *ctx = (fte::ctx_t *)ctxt;
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(ctx->feature_session_state());
    ftp_info_t      *info = (ftp_info_t *)l4_sess->info;
    uint32_t         offset = 0, matchlen = 0;
    ftp_search_t     cmd;
    int              found = 0;
    ftp_state_t      prev_state;
    l4_alg_status_t *exp_flow = NULL;

    memset (&cmd, 0, sizeof(ftp_search_t));

    prev_state = info->state;
    if (info->state < FTP_MAX_RSP)
        cmd = ftp_rsp[info->state];

    if (cmd.pattern != '\0') {
        found = find_pattern((char *)payload, data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb);
        if (found <= 0) {
            /*
             * Check for any error responses
             */
            for (uint8_t i=FTP_ERR_RSP; i<FTP_MAX_RSP; i++) {
                 cmd = ftp_rsp[i];
                 found = find_pattern((char *)payload, data_len, cmd.pattern,
                                      cmd.plen, cmd.skip, cmd.term, cmd.state,
                                      &offset, &matchlen, info, cmd.cb);
                 if (found) {
                     /*
                      * Clean up the previously added expected flow from PORT/EPRT
                      * if there is an error
                      */
                     if ((prev_state == FTP_PORT || prev_state == FTP_EPRT) &&
                         (exp_flow = g_ftp_state->get_next_expflow(
                          l4_sess->app_session)) != NULL) {
                         g_ftp_state->cleanup_exp_flow(exp_flow);
                     }
                     return data_len;
                 }
            }

#ifdef LOGIN_ERR_NEEDED
            if (info->state == FTP_USER || info->state == FTP_PASS) {
                /*
                 * Increment login errors
                 */
                if (found) {
                    SDK_ATOMIC_INC_UINT32(&((ftp_info_t *)l4_sess->info)->login_errors, 1);
                    return data_len;
                }

                /*
                 * If no error responses found, wait for completion
                 */
                cmd = ftp_rsp[info->state + 1];
                found = find_pattern((char *)payload, data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb);
            }
#endif

            /*
             * We dont need to update anything for these commands
             */
            return data_len;
        }

        if (info->add_exp_flow) {
            add_expected_flow(*ctx, l4_sess, info);
        }
    }

    info->callback = __parse_ftp_req; 
    return data_len;
}

/*
 * Walks through all the possible FTP requests that we care
 * about. If none is matching, its a no-op for us. If there are
 * any parse errors found, we update the counters.
 */
size_t __parse_ftp_req(void *ctxt, uint8_t *payload, size_t data_len) {
    fte::ctx_t      *ctx = (fte::ctx_t *)ctxt;
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(
                                          ctx->feature_session_state());
    ftp_info_t      *info = (ftp_info_t *)l4_sess->info;
    uint32_t         i;
    uint32_t         matchlen, offset;
    ftp_search_t     cmd;
    int              found = 0;

    /*
     * Compute total datalen
     */
    for (i=0; i < FTP_MAX_REQ; i++) {
        cmd = ftp_req[i];
        found = find_pattern((char *)payload, data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb);
        if (found) break;
    }

    if (!found) {
        return data_len;
    } else if (found == -1) {
        /*
         * Parse errors -- update ctrl session info with this
         */
        SDK_ATOMIC_INC_UINT32(&((ftp_info_t *)l4_sess->info)->parse_errors, 1);
    } else {
        /*
         * Found a match -- update the callback
         * and wait for a response
         */
        info->callback = __parse_ftp_rsp; 
        if (info->add_exp_flow) {
            add_expected_flow(*ctx, l4_sess, info);
        }
    }

    return data_len;
}

/*
 * FTP ALG completion handler - invoked when the session creation is done.
 */
static void ftp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(\
                               ctx.feature_session_state(FTE_FEATURE_ALG_FTP));

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == true) {
            g_ftp_state->cleanup_app_session(l4_sess->app_session);
        }
    } else if (l4_sess) {
        l4_sess->sess_hdl = ctx.session()->hal_handle;
        if (l4_sess->isCtrl == false) {
            /*
             * Data session flow has been installed sucessfully
             * Cleanup expected flow from the exp flow table and app
             * session list and move it to l4 session list
             */
            g_ftp_state->move_expflow_to_l4sess(l4_sess->app_session, l4_sess);
        }
    }
}

/*
 * FTP Exec - process only when FTP ALG is invoked by firewall(sfw) or
 * we are processing an expected flow.
 */
fte::pipeline_action_t alg_ftp_exec(fte::ctx_t &ctx) {
    hal_ret_t                      ret = HAL_RET_OK;
    app_session_t                 *app_sess = NULL;
    l4_alg_status_t               *l4_sess = NULL;
    ftp_info_t                    *ftp_info = NULL;
    fte::flow_update_t             flowupd;
    fte::feature_session_state_t  *alg_state = NULL;
    sfw_info_t                    *sfw_info =
                                   (sfw_info_t*)ctx.feature_state(\
                                      FTE_FEATURE_SFW);

    if ((hal::g_hal_state->is_flow_aware()) ||
        (ctx.protobuf_request() && !ctx.sync_session_request())) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (sfw_info->alg_proto == nwsec::APP_SVC_FTP &&
        (!ctx.existing_session())) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            /*
             * Alloc APP session, L4 Session and FTP info
             */
            ret = g_ftp_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            ret = g_ftp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            l4_sess->alg = nwsec::APP_SVC_FTP;
            ftp_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
            SDK_ASSERT_RETURN((ftp_info != NULL), fte::PIPELINE_CONTINUE);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = ftp_info;
            l4_sess->idle_timeout = sfw_info->idle_timeout;
            ftp_info->state = FTP_INIT;
            ftp_info->callback = __parse_ftp_req;
            ftp_info->sip = ctx.key().dip;
            ftp_info->add_exp_flow = false;
            ftp_info->allow_mismatch_ip_address =
                  sfw_info->alg_opts.opt.ftp_opts.allow_mismatch_ip_address;

            /*
             * Register Feature session state & completion handler
             */
            ctx.register_completion_handler(ftp_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);

            if (ctx.sync_session_request()) {
                alg_ftp_sync_session_proc(ctx, l4_sess);
            } else {
                if ((ctx.cpu_rxhdr()->tcp_flags & (TCP_FLAG_SYN)) == TCP_FLAG_SYN) {
                    // Setup TCP buffer for IFLOW
                    l4_sess->tcpbuf[DIR_IFLOW] =
                        tcp_buffer_t::factory(htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1, 
                                              NULL, __parse_ftp_req, g_ftp_tcp_buffer_slabs);
                }
            }
        }

        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_info.mcast_en = 1;
        flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
        flowupd.mcast_info.proxy_mcast_ptr = 0;
        ret = ctx.update_flow(flowupd);
    } else if (alg_state != NULL && (ctx.role() == hal::FLOW_ROLE_INITIATOR)) {
        l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
        if (l4_sess != NULL && (l4_sess->alg == nwsec::APP_SVC_FTP)) {
            ftp_info = (ftp_info_t *)l4_sess->info;

            if (ctx.sync_session_request()) {
                alg_ftp_sync_session_proc(ctx, l4_sess);

                if (!ctx.existing_session()) {
                    ctx.register_completion_handler(ftp_completion_hdlr);
                }
                return fte::PIPELINE_CONTINUE;
            }

            if (!l4_sess->tcpbuf[DIR_RFLOW] && ctx.is_flow_swapped() && 
                !ctx.payload_len()) {
                // Set up TCP buffer for RFLOW only when we see handshake
                // If due to load we miss the handshake then there is no point
                // to process the packets.
                l4_sess->tcpbuf[DIR_RFLOW] = tcp_buffer_t::factory(
                                          htonl(ctx.cpu_rxhdr()->tcp_seq_num)+1, 
                                          NULL, __parse_ftp_req, g_ftp_tcp_buffer_slabs);
            }
            /*
             * Process only when we are expecting something.
             */
            if (l4_sess->isCtrl == true) {
                /*
                 * This will only be executed for control channel packets that
                 * would lead to opening up pinholes for FTP data sessions.
                 */
                uint8_t buff = ctx.is_flow_swapped()?1:0;
                if (ctx.payload_len() &&
                    (l4_sess->tcpbuf[0] && l4_sess->tcpbuf[1]))
                    l4_sess->tcpbuf[buff]->insert_segment(ctx, ftp_info->callback);
            } else if (!ctx.existing_session()) {
                /*
                 * We have received request for data session. Register completion
                 * handler to cleanup the exp_flow and move it to l4_session list
                 */
                ctx.register_completion_handler(ftp_completion_hdlr);
            }
        }
    }

    return fte::PIPELINE_CONTINUE;
}

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
