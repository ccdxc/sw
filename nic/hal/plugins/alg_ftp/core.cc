/*
 * core.cc
 */

#include "core.hpp"
#include "utils.hpp"
#include "sdk/list.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace plugins {
namespace alg_ftp {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

static void incr_parse_error(ftp_info_t *info) {
    HAL_ATOMIC_INC_UINT32(&info->parse_errors, 1);
}

/*
 * APP session delete handler
 */
fte::pipeline_action_t alg_ftp_session_delete_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    l4_alg_status_t               *l4_sess =  NULL;
    app_session_t                 *app_sess = NULL;

    alg_state = ctx.feature_session_state();
    if (alg_state != NULL) {
        l4_sess = (l4_alg_status_t *)alg_status(alg_state);
        app_sess = l4_sess->app_session;
        if (l4_sess->isCtrl == TRUE) {
            if (dllist_empty(&app_sess->exp_flow_lhead) &&
                dllist_count(&app_sess->l4_sess_lhead) == 1 &&
                ((l4_alg_status_t *)dllist_entry(app_sess->l4_sess_lhead.next,\
                                 l4_alg_status_t, l4_sess_lentry)) == l4_sess) {
                /*
                 * If there are no expected flows or L4 data sessions
                 * hanging off of this ctrl session, then go ahead and clean
                 * up the app session
                 */
                 g_ftp_state->cleanup_app_session(l4_sess->app_session);
                 HAL_TRACE_DEBUG("Cleaned up app session");
            } else if ((ctx.session()->iflow->state >= session::FLOW_TCP_STATE_FIN_RCVD) ||
                       (ctx.session()->rflow && 
                        (ctx.session()->rflow->state >= session::FLOW_TCP_STATE_FIN_RCVD))) {
                /*
                 * We received FIN/RST on the control session
                 * We let the HAL cleanup happen while we keep the 
                 * app_session state if there are data sessions
                 */
                l4_sess->session = NULL;
                HAL_TRACE_DEBUG("Received FIN/RST.. keeping the control context");
                return fte::PIPELINE_CONTINUE;
            } else {
               /*
                * Dont cleanup if control session is timed out
                * we need to keep it around until the data session
                * goes away
                */
                HAL_TRACE_DEBUG("Data session is alive. Bailing session ageout on control");
                return fte::PIPELINE_END;
            }
        } 
        /*
         * Cleanup the data session that is getting timed out
         */
        g_ftp_state->cleanup_l4_sess(l4_sess);
        if (dllist_empty(&app_sess->exp_flow_lhead) &&
            dllist_count(&app_sess->l4_sess_lhead) == 1 &&
            ((l4_alg_status_t *)dllist_entry(app_sess->l4_sess_lhead.next,\
                      l4_alg_status_t, l4_sess_lentry))->session == NULL) {
            /*
             * If this was the last session hanging and there is no
             * HAL session for control session. This is the right time
             * to clean it
             */
            g_ftp_state->cleanup_app_session(l4_sess->app_session);
            HAL_TRACE_DEBUG("Cleaned up app session");
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
            HAL_TRACE_DEBUG("__parse_port: {}", tmp_port);
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
    HAL_TRACE_DEBUG("Ret: {}", ret);
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

    HAL_TRACE_DEBUG("Dlen: {}", dlen);
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

            HAL_TRACE_ERR("__parse_ipv4: Char {} (got {} nums) {} unexpected\n",
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

    ftp_info->ip.v4_addr =  htonl((array[3] << 24) | (array[2] << 16) |
                             (array[1] << 8) | array[0]);
    ftp_info->port = htons((array[5] << 8) | array[4]);
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

    HAL_TRACE_DEBUG("EPRT: Got {}{}{}", delim, data[1], delim);

    if (data[1] == 1) {
        u_int32_t array[4];

        /* Now we have IP address. */
        length = __parse_ipv4(data + 3, dlen - 3, array, 4, '.', delim);
        if (length != 0) 
            ftp_info->ip.v4_addr = htonl((array[3] << 24) | (array[2] << 16)\
                                         | (array[1] << 8) | array[0]);
    } else {
        ftp_info->isIPv6 = TRUE;
        /* Now we have IPv6 address. */
        length = __parse_ipv6(data + 3, dlen - 3,
                               ftp_info->ip.v6_addr.addr8, delim);
    }

    if (length == 0) {
        incr_parse_error(ftp_info);
        return 0;
    }

    HAL_TRACE_DEBUG("EPRT: Got IP address!");

    /* Start offset includes initial "|1|", and trailing delimiter */
    ret = __parse_port(data, 3 + length + 1, dlen, delim, &ftp_info->port);
    if (!ret)
        incr_parse_error(ftp_info);

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

    HAL_TRACE_DEBUG("Offset to parse: {} data: {}", *offset, (data+i));
    ret = __parse_port_cmd(data + i, dlen - i, 0, offset, ftp_info);
    if (!ret)
        incr_parse_error(ftp_info);
 
    return ret;
}

/*
 * Parse EPSV response command
 */
static int __parse_epsv_response(const char *data, uint32_t dlen,
                                 char term, uint32_t *offset,
                                 ftp_info_t *ftp_info) {
    char delim;

    /* Three delimiters. */
    if (dlen <= 3) return 0;
    delim = data[0];
    if (isdigit(delim) || delim < 33 || delim > 126 ||
        data[1] != delim || data[2] != delim)
        return 0;

    return __parse_port(data, 3, dlen, delim, &ftp_info->port);
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
           __FTP_CMD("USER", ' ',  '\r', FTP_USER,  NULL),
           __FTP_CMD("PASS", ' ',  '\r', FTP_PASS,  NULL),
};


ftp_search_t ftp_rsp[FTP_MAX_RSP] = {
           __FTP_CMD("\0",        '\0', '\0',  FTP_INIT,  NULL),  /* Parse errror - not a valid response */
           __FTP_CMD("200 PORT ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PORT Success response  */
           __FTP_CMD("200 EPRT ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_EPRT Success response  */
           __FTP_CMD("227 ",      '\0', '\0',  FTP_INIT,  __parse_pasv_response), /* FTP_PASV response */
           __FTP_CMD("229 ",      '(',  ')',   FTP_INIT,  __parse_epsv_response), /* FTP_EPSV response */
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_USER success response */
           __FTP_CMD("331 ",      '\0', '\0',  FTP_PASS,  NULL),  /* FTP_USER Password needed response */
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PASS success response */
           __FTP_CMD("332 ",      '\0', '\0',  FTP_ACCT,  NULL),  /* FTP_PASS Acct needed response */
           __FTP_CMD("230 ",      '\0', '\0',  FTP_INIT,  NULL),  /* FTP_ACCT success response */
           __FTP_CMD("202 ",      '\0', '\0',  FTP_INIT,  NULL),  /* Syntax error response */
           __FTP_CMD("4",         '\0', '\0',  FTP_INIT,  NULL),  /* Transient error response */
           __FTP_CMD("5",         '\0', '\0',  FTP_INIT,  NULL),  /* Error response */
};

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, expected_flow_t *wentry) {
    sfw_info_t *sfw_info =
        (sfw_info_t*)ctx.feature_state(FTE_FEATURE_SFW);
    l4_alg_status_t *entry = NULL;
    ftp_info_t      *ftp_info = NULL;

    HAL_TRACE_DEBUG("ALG_FTP: expected_flow_handler");
    entry = (l4_alg_status_t *)wentry;
    HAL_TRACE_DEBUG("Entry: {:p}", (void *)entry);
    ftp_info = (ftp_info_t *)entry->info;
    if (entry->isCtrl != TRUE && sfw_info != NULL) {
        sfw_info->skip_sfw = ftp_info->skip_sfw;
    }
    ctx.set_feature_name(FTE_FEATURE_ALG_FTP.c_str());
    ctx.register_feature_session_state(&entry->fte_feature_state);

    return HAL_RET_OK;
}

/*
 * Walks through list of acceptable responses, updates errors and
 * adds exp_flow for new data sessions to aid opening of pinholes.
 */
void __parse_ftp_rsp(fte::ctx_t &ctx, ftp_info_t *info) {
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
    ftp_info_t      *data_ftp_info = NULL;
    hal_ret_t        ret = HAL_RET_OK;
    l4_alg_status_t *exp_flow = NULL;
    uint32_t         payload_offset = ctx.cpu_rxhdr()->payload_offset;
    uint32_t         data_len = 0, offset = 0, matchlen = 0;
    uint8_t         *pkt = ctx.pkt();
    ftp_search_t     cmd;
    int              found = 0;
    hal::flow_key_t  key;

    HAL_TRACE_DEBUG("In __parse_ftp_rsp");

    info->callback = __parse_ftp_req; 
    if (info->state < FTP_MAX_RSP) 
        cmd = ftp_rsp[info->state];

    data_len = (ctx.pkt_len() - payload_offset);
    if (cmd.pattern != '\0') {
        found = find_pattern((char *)&pkt[payload_offset], data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb); 
        if (found <= 0) {
            /*
             * Check for any error responses
             */
            for (uint8_t i=FTP_SYNTAX_ERR; i<FTP_MAX_RSP; i++) {
                 cmd = ftp_rsp[i];
                 found = find_pattern((char *)&pkt[payload_offset], data_len, cmd.pattern,
                                      cmd.plen, cmd.skip, cmd.term, cmd.state,
                                      &offset, &matchlen, info, cmd.cb);
                 if (found) return;
            }

            if (info->state == FTP_USER || info->state == FTP_PASS) {
                /*
                 * Increment login errors
                 */
                if (found) {
                    HAL_ATOMIC_INC_UINT32(&((ftp_info_t *)l4_sess->info)->login_errors, 1);
                    return;
                }
                
                /*
                 * If no error responses found, wait for completion
                 */
                cmd = ftp_rsp[info->state + 1];
                found = find_pattern((char *)&pkt[payload_offset], data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb);
            }

            /*
             * We dont need to update anything for these commands
             */
            return;
        }
       
        /*
         * Install a new expected data flow for this control session
         * Expected data flow key - (flow_type, vrf, dip & dport)
         */
         memset(&key, 0, sizeof(hal::flow_key_t));
         key = ctx.key();
         key.dir = 0;
         key.sport = 0;
         memset(&key.sip, 0, sizeof(ipvx_addr_t));
         if (!isNullip(info->ip, (info->isIPv6)?IP_PROTO_IPV6:IP_PROTO_IPV4)) {
             memcpy(&key.dip, &info->ip, sizeof(ipvx_addr_t));
         }
         memcpy(&key.dport, &info->port, sizeof(info->port)); 
         HAL_TRACE_DEBUG("Dip: {} Dport: {}", key.dip.v4_addr, key.dport);
         g_ftp_state->alloc_and_insert_exp_flow(l4_sess->app_session, key, &exp_flow);
         exp_flow->entry.handler = expected_flow_handler;
         HAL_ASSERT(ret != HAL_RET_OOM);
         exp_flow->isCtrl = FALSE;
         exp_flow->alg = l4_sess->alg;
         data_ftp_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
         HAL_ASSERT(data_ftp_info != NULL);
         exp_flow->info = data_ftp_info;
         data_ftp_info->skip_sfw = TRUE;
    }
}

/*
 * Walks through all the possible FTP requests that we care
 * about. If none is matching, its a no-op for us. If there are
 * any parse errors found, we update the counters.
 */
void __parse_ftp_req(fte::ctx_t &ctx, ftp_info_t *info) {
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
    uint32_t         payload_offset = ctx.cpu_rxhdr()->payload_offset;
    uint32_t         i, data_len = 0;
    uint32_t         matchlen, offset;
    uint8_t         *pkt = ctx.pkt();
    ftp_search_t     cmd;
    int              found = 0;

    HAL_TRACE_DEBUG("In __parse_ftp_req");

    /*
     * Compute total datalen
     */
    data_len = ctx.pkt_len()-payload_offset;

    for (i=0; i < FTP_MAX_REQ; i++) {
        cmd = ftp_req[i];
        found = find_pattern((char *)&pkt[payload_offset], data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb);
        if (found) break; 
    }

    if (!found) {
        return;
    } else if (found == -1) {
        /*
         * Parse errors -- update ctrl session info with this
         */
        HAL_ATOMIC_INC_UINT32(&((ftp_info_t *)l4_sess->info)->parse_errors, 1);  
    } else {
        /*
         * Found a match -- update the callback 
         * and wait for a response
         */
         info->callback = __parse_ftp_rsp;
    }

    return; 
}

/* 
 * FTP ALG completion handler - invoked when the session creation is done.
 */
static void ftp_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(\
                               ctx.feature_session_state(FTE_FEATURE_ALG_FTP));

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            g_ftp_state->cleanup_app_session(l4_sess->app_session);
        }
    } else {
        l4_sess->session = ctx.session();
        if (l4_sess && l4_sess->isCtrl == FALSE) {
            HAL_TRACE_DEBUG("In FTP Completion handler");
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
    uint32_t                       payload_offset;
    fte::feature_session_state_t  *alg_state = NULL;
    sfw_info_t                    *sfw_info =
                                   (sfw_info_t*)ctx.feature_state(\
                                      FTE_FEATURE_SFW);

    if (ctx.protobuf_request()) {
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
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            ret = g_ftp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            l4_sess->alg = nwsec::APP_SVC_FTP;
            ftp_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
            HAL_ASSERT_RETURN((ftp_info != NULL), fte::PIPELINE_CONTINUE);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = ftp_info;
            ftp_info->state = FTP_INIT;
            ftp_info->callback = __parse_ftp_req;
            /*
             * Register Feature session state & completion handler
             */
            ctx.register_completion_handler(ftp_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);
        }

        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_info.mcast_en = 1;
        flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
        flowupd.mcast_info.proxy_mcast_ptr = 0;
        ret = ctx.update_flow(flowupd);
    } else if (alg_state != NULL) {
        l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
        if (l4_sess != NULL && (l4_sess->alg == nwsec::APP_SVC_FTP) && \
            (ctx.role() == hal::FLOW_ROLE_INITIATOR)) {
            ftp_info = (ftp_info_t *)l4_sess->info;
            /*
             * Process only when we are expecting something.
             */
            if (l4_sess->isCtrl == TRUE) {
                /*
                 * This will only be executed for control channel packets that
                 * would lead to opening up pinholes for FTP data sessions.
                 */
                payload_offset = ctx.cpu_rxhdr()->payload_offset;
                if (ctx.pkt_len() <= payload_offset) {
                    // The first iflow packet that get mcast copied could be an
                    // ACK from the TCP handshake.
                    HAL_TRACE_DEBUG("Ignoring the packet -- may be a handshake packet");
                    return fte::PIPELINE_CONTINUE;
                } else {
                    ftp_info->callback(ctx, ftp_info);
                }
            } else {
                HAL_TRACE_DEBUG("Not a control packet");
            }
            /*
             * We have received request for data session. Register completion
             * handler to cleanup the exp_flow and move it to l4_session list
             */  
            ctx.register_completion_handler(ftp_completion_hdlr);
        }
    }

    return fte::PIPELINE_CONTINUE;
}

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
