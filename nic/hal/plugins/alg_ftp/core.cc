/*
 * core.cc
 */

#include "core.hpp"
#include "utils.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace plugins {
namespace alg_ftp {

using namespace hal::plugins::alg_utils;

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

    ftp_info->ip.v4_addr =  htonl((array[0] << 24) | (array[1] << 16) |
                             (array[2] << 8) | array[3]);
    ftp_info->port = htons((array[4] << 8) | array[5]);
    return length;
}

/*
 * Parse EPRT command
 */
static int __parse_eprt_cmd(const char *data, uint32_t dlen,
                            char term, uint32_t *offset,
                            ftp_info_t *ftp_info) {
    char delim;
    int length;

    /* First character is delimiter, then "1" for IPv4 or "2" for IPv6,
       then delimiter again. */
    if (dlen <= 3) {
        // parse error
        HAL_TRACE_ERR("EPRT: command too short");
        return 0;
    }

    delim = data[0];
    if (isdigit(delim) || delim < 33 || delim > 126 || data[2] != delim) {
        //parse error
        HAL_TRACE_ERR("EPRT: Invalid delimiter");
        return 0;
    }

    if ((!ftp_info->isIPv6 && data[1] != '1') ||
        (ftp_info->isIPv6 && data[1] != '2')) {
        //parse error 
        HAL_TRACE_ERR("EPRT: invalid protocol number");
        return 0;
    }

    HAL_TRACE_DEBUG("EPRT: Got {}{}{}", delim, data[1], delim);

    if (data[1] == '1') {
        u_int32_t array[4];

        /* Now we have IP address. */
        length = __parse_ipv4(data + 3, dlen - 3, array, 4, '.', delim);
        if (length != 0) 
            ftp_info->ip.v4_addr = htonl((array[0] << 24) | (array[1] << 16)\
                                         | (array[2] << 8) | array[3]);
    } else {
        /* Now we have IPv6 address. */
        length = __parse_ipv6(data + 3, dlen - 3,
                               ftp_info->ip.v6_addr.addr8, delim);
    }

    if (length == 0)
        return 0;

    HAL_TRACE_DEBUG("EPRT: Got IP address!");

    /* Start offset includes initial "|1|", and trailing delimiter */
    return __parse_port(data, 3 + length + 1, dlen, delim, &ftp_info->port);
}

/*
 * Parse PASV response
 */
static int __parse_pasv_response(const char *data, uint32_t dlen,
                                 char term, uint32_t *offset, 
                                 ftp_info_t *ftp_info) {
    uint32_t i;
    for (i = 0; i < dlen; i++)
        if (isdigit(data[i]))
            break;

    if (i == dlen)
        return 0;

    *offset += i;

    return __parse_port_cmd(data + i, dlen - i, 0, offset, ftp_info);
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
            if (i == dlen - 1) return -1;
        }

        /* Skip over the last character */
        i++;
    }

    HAL_TRACE_DEBUG("Skipped up to {}!\n", skip);

    *offset = i;
    if (cb != NULL)  {
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
           __FTP_CMD("PORT", ' ',  '\r', FTP_PORT_EPRT, __parse_port_cmd),
           __FTP_CMD("EPRT", ' ',  '\r', FTP_PORT_EPRT, __parse_eprt_cmd),
           __FTP_CMD("PASV", '\0', '\r', FTP_PASV,      __parse_pasv_response),
           __FTP_CMD("EPSV", '\0', '\r', FTP_EPSV,      __parse_epsv_response),
           __FTP_CMD("USER", ' ',  '\r', FTP_USER,      NULL),
           __FTP_CMD("PASS", ' ',  '\r', FTP_PASS,      NULL),
};


ftp_search_t ftp_rsp[FTP_MAX_RSP] = {
           __FTP_CMD("\0",   '\0', '\0',  FTP_INIT,  NULL),  /* Parse errror - not a valid response */
           __FTP_CMD("200 ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PORT_EPRT Success response  */
           __FTP_CMD("227 ", '\0', '\0',  FTP_INIT,  __parse_pasv_response), /* FTP_PASV response */
           __FTP_CMD("229 ", '(',  ')',   FTP_INIT,  __parse_epsv_response), /* FTP_EPSV response */
           __FTP_CMD("230 ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_USER success response */
           __FTP_CMD("331 ", '\0', '\0',  FTP_PASS,  NULL),  /* FTP_USER Password needed response */
           __FTP_CMD("230 ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_PASS success response */
           __FTP_CMD("332 ", '\0', '\0',  FTP_ACCT,  NULL),  /* FTP_PASS Acct needed response */
           __FTP_CMD("230 ", '\0', '\0',  FTP_INIT,  NULL),  /* FTP_ACCT success response */
           __FTP_CMD("202 ", '\0', '\0',  FTP_INIT,  NULL),  /* Syntax error response */
           __FTP_CMD("4",    '\0', '\0',  FTP_INIT,  NULL),  /* Transient error response */
           __FTP_CMD("5",    '\0', '\0',  FTP_INIT,  NULL),  /* Error response */
};

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, fte::expected_flow_t *wentry) {
    hal::plugins::sfw::sfw_info_t *sfw_info =
        (hal::plugins::sfw::sfw_info_t*)ctx.feature_state(hal::plugins::sfw::FTE_FEATURE_SFW);
    l4_alg_status_t *entry = NULL;
    ftp_info_t      *ftp_info = NULL;

    entry = (l4_alg_status_t *)wentry;
    ftp_info = (ftp_info_t *)entry->info;
    if (entry->isCtrl != TRUE) {
        sfw_info->skip_sfw = ftp_info->skip_sfw;
    }
    ctx.register_feature_session_state(&entry->fte_feature_state);

    return HAL_RET_OK;
}

/*
 * Walks through list of acceptable responses, updates errors and
 * adds exp_flow for new data sessions to aid opening of pinholes.
 */
void __parse_ftp_rsp(fte::ctx_t &ctx, ftp_info_t *info) {
    l4_alg_status_t *l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
    hal_ret_t        ret = HAL_RET_OK;
    l4_alg_status_t *exp_flow = NULL;
    uint32_t         payload_offset = ctx.cpu_rxhdr()->payload_offset;
    uint32_t         data_len = 0, offset = 0, matchlen = 0;
    uint8_t         *pkt = ctx.pkt();
    ftp_search_t     cmd;
    int              found = 0;
    hal::flow_key_t  key;

    info->callback = __parse_ftp_req; 
    if (info->state < FTP_MAX_RSP) 
        cmd = ftp_rsp[info->state];

    if (cmd.pattern != '\0') {
        found = find_pattern((char *)&pkt[payload_offset], data_len, cmd.pattern,
                             cmd.plen, cmd.skip, cmd.term, cmd.state,
                             &offset, &matchlen, info, cmd.cb); 
        if (!found) {
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
                /*
                 * We dont need to update anything for these commands
                 */
                return;
            }
        }
        
        /*
         * Install a new expected data flow for this control session
         * Expected data flow key - (flow_type, vrf, dip & dport)
         */
         memset(&key, 0, sizeof(hal::flow_key_t));
         key.flow_type = ctx.key().flow_type;
         key.vrf_id    = ctx.key().vrf_id;
         memcpy(&key.dip, &info->ip, sizeof(ipvx_addr_t));
         memcpy(&key.dport, &info->port, sizeof(info->port)); 
         g_ftp_state->alloc_and_insert_exp_flow(l4_sess->app_session, key, &exp_flow);
         exp_flow->entry.handler = expected_flow_handler;
         HAL_ASSERT(ret != HAL_RET_OOM);
         exp_flow->isCtrl = FALSE;
         exp_flow->alg = l4_sess->alg;
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
    ftp_info_t      *exp_flow_info = NULL;
    l4_alg_status_t *exp_flow = NULL;
    hal_ret_t        ret;

    if (!status) {
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            g_ftp_state->cleanup_app_session(l4_sess->app_session);
        }
    } else {
        l4_sess->session = ctx.session();
        if (l4_sess && l4_sess->isCtrl == TRUE) {
            /*
             * Add an expected flow here for control session
             */
            ret = g_ftp_state->alloc_and_insert_exp_flow(l4_sess->app_session, 
                                                   ctx.key(), &exp_flow);
            HAL_ASSERT(ret != HAL_RET_OOM);
            exp_flow->entry.handler = expected_flow_handler;
            exp_flow->alg = nwsec::APP_SVC_FTP;
            exp_flow->isCtrl = TRUE;
            exp_flow_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
            HAL_ASSERT(exp_flow_info != NULL);
            exp_flow_info->state = FTP_INIT;
            exp_flow_info->callback = __parse_ftp_req;
            exp_flow->info = exp_flow_info;
        } else {
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
    l4_alg_status_t               *l4_sess = NULL, *exp_flow = NULL;
    ftp_info_t                    *ftp_info = NULL;
    fte::flow_update_t             flowupd;
    uint32_t                       payload_offset;
    hal::plugins::sfw::sfw_info_t *sfw_info =
        (hal::plugins::sfw::sfw_info_t*)ctx.feature_state(\
                       hal::plugins::sfw::FTE_FEATURE_SFW);

    l4_sess = (l4_alg_status_t *)alg_status(ctx.feature_session_state());
    if (l4_sess != NULL) 
        ftp_info = (ftp_info_t *)l4_sess->info;
    
    if (sfw_info->alg_proto == nwsec::APP_SVC_FTP) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            /*
             * Alloc APP session, L4 Session and FTP info
             */
            ret = g_ftp_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            HAL_ASSERT_RETURN((ret != HAL_RET_OOM), fte::PIPELINE_CONTINUE);
            ret = g_ftp_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            HAL_ASSERT_RETURN((ret != HAL_RET_OOM), fte::PIPELINE_CONTINUE);
            l4_sess->alg = nwsec::APP_SVC_FTP;
            ftp_info = (ftp_info_t *)g_ftp_state->alg_info_slab()->alloc();
            HAL_ASSERT_RETURN((ftp_info != NULL), fte::PIPELINE_CONTINUE);
            l4_sess->isCtrl = TRUE;
            l4_sess->info = ftp_info;

            /*
             * Register Feature session state & completion handler
             */
            ctx.register_completion_handler(ftp_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);
        }

        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_info.mcast_en = 1;
        flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
        ret = ctx.update_flow(flowupd);
    } else if (l4_sess != NULL && l4_sess->alg == nwsec::APP_SVC_FTP) {
        /*
         * Process only when we are expecting something.
         */
        if (l4_sess->isCtrl == TRUE) {
            /*
             * This will only be executed for control channel packets that
             * would lead to opening up pinholes for FTP data sessions.
             */

            /*
             * Derive the ftp info from the expected flow
             * for parsing
             */
            exp_flow = g_ftp_state->get_ctrl_expflow(l4_sess->app_session);
            if (!exp_flow)
                return fte::PIPELINE_CONTINUE;

            ftp_info = (ftp_info_t *)exp_flow->info;
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
