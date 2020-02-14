//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/sfw/alg_utils/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

#define MAX_DNS_DOMAINNAME_LEN 256
#define MAX_DNS_LABEL_LEN      63
#define MAX_DNS_MSG_LEN        8192
#define DNS_ALG_RFLOW_TIMEOUT  (30 * TIME_MSECS_PER_MIN) // 30 mins
#define DNS_HEADER_LENGTH      12 // bytes
#define DNS_OPCODE_SHIFT       3
#define DNS_HEADER_OPCODE_LEN  2
#define DNS_HEADER_RR_LEN      6
#define DNS_MIN_TIMEOUT        1 

namespace hal {
namespace plugins {
namespace alg_dns {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

// Holds the information for a dns question.
typedef struct dns_question {
    char     name[MAX_DNS_DOMAINNAME_LEN];
    uint16_t type;
    uint16_t cls;
} dns_question;

static void incr_parse_error (l4_alg_status_t *sess)
{
    SDK_ATOMIC_INC_UINT32(&(((dns_info_t *)sess->info)->parse_errors), 1);
}

/*
 * DNS info cleanup handler
 */
void dnsinfo_cleanup_hdlr (l4_alg_status_t *l4_sess)
{
    if (l4_sess->info != NULL)
        g_dns_state->alg_info_slab()->free((dns_info_t *)l4_sess->info);
}

/*
 *  APP Session delete handler
 */
fte::pipeline_action_t alg_dns_session_delete_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    l4_alg_status_t               *l4_sess = NULL;

    if (ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_DNS)
        return fte::PIPELINE_CONTINUE;

    ctx.flow_log()->alg = l4_sess->alg;

    /*
     * Cleanup ALG state. No concept of control vs data here
     * so cleanup everything.
     */
    g_dns_state->cleanup_app_session(l4_sess->app_session);
    

    return fte::PIPELINE_CONTINUE;
}

/*
 * APP Session get handler
 */
fte::pipeline_action_t alg_dns_session_get_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    SessionGetResponse            *sess_resp = ctx.sess_get_resp();
    l4_alg_status_t               *l4_sess = NULL;

    if (!ctx.sess_get_resp() || ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || l4_sess->alg != nwsec::APP_SVC_DNS)
        return fte::PIPELINE_CONTINUE;

    sess_resp->mutable_status()->set_alg(nwsec::APP_SVC_DNS);

    dns_info_t *info = ((dns_info_t *)l4_sess->info);
    if (info) {
        sess_resp->mutable_status()->mutable_dns_info()->\
          set_parse_errors(info->parse_errors);
        sess_resp->mutable_status()->mutable_dns_info()->\
          set_time_remaining(sdk::lib::get_timeout_remaining(info->timer));
        sess_resp->mutable_status()->mutable_dns_info()->\
          set_dns_id(info->dnsid);
    }

    return fte::PIPELINE_CONTINUE;
}

/*
 * DNS ALG timer expiry callback -- if this is fired means we didnt receive
 * a DNS response hence we have to cleanup the HAL session and all ALG 
 * resources pertaining to it.
 */
void dns_rflow_timeout_cb (void *timer, uint32_t timer_id, void *ctxt) {
    hal_handle_t  sess_hdl = (hal_handle_t)ctxt;
    session_t    *session = NULL;

    session = hal::find_session_by_handle(sess_hdl);
    if (session != NULL) {
        /* Post a force delete on timer expiry */
        session_delete_async(session, true);
    }
}

/*
 * DNS ALG completion handler - invoked when the session creation is done.
 */
static void dns_completion_hdlr (fte::ctx_t& ctx, bool status) {
    l4_alg_status_t     *l4_sess = (l4_alg_status_t *)alg_status(\
                             ctx.feature_session_state(FTE_FEATURE_ALG_DNS));
    dns_info_t          *dns_info = (dns_info_t *)l4_sess->info;
    sfw_info_t          *sfw_info = sfw::sfw_feature_state(ctx);

    SDK_ASSERT(l4_sess != NULL);

    if (!status) {
        if (l4_sess)
            g_dns_state->cleanup_app_session(l4_sess->app_session);
    } else {
        HAL_TRACE_DEBUG("In DNS Completion handler ctrl");
        l4_sess->sess_hdl = ctx.session()->hal_handle;

        dns_info->timer = sdk::lib::timer_schedule(DNS_ALG_SESS_TIMER_ID, 
                                  sfw_info->alg_opts.opt.dns_opts.query_response_timeout, 
                                  (void *)l4_sess->sess_hdl, 
                                  dns_rflow_timeout_cb, false);
        if (!dns_info->timer) {
            HAL_TRACE_ERR("Failed to start timer for dns session with key: {}"\
                          " and dns id: {}", ctx.key(), dns_info->dnsid);
        }
    } 
    return;
}

static hal_ret_t read_rr_name(const uint8_t * packet, uint32_t * packet_p, 
                       uint32_t id_pos, uint32_t len, char *name, uint32_t *name_len_p) {
    uint32_t i, next, pos=*packet_p;
    uint32_t end_pos = 0;
    uint32_t name_len=0;
    uint32_t steps = 0;

    // Scan through the name, one character at a time. We need to look at 
    // each character to look for values we can't print in order to allocate
    // extra space for escaping them.  'next' is the next position to look
    // for a compression jump or name end.
    // It's possible that there are endless loops in the name. Our protection
    // against this is to make sure we don't read more bytes in this process
    // than twice the length of the data.  Names that take that many steps to 
    // read in should be impossible.
    next = pos;
    while (pos < len && !(next == pos && packet[pos] == 0)
           && steps < len*2) {
        uint8_t c = packet[pos];
        steps++;
        if (next == pos) {
            // Handle message compression.  
            // If the length byte starts with the bits 11, then the rest of
            // this byte and the next form the offset from the dns proto start
            // to the start of the remainder of the name.
            if ((c & 0xc0) == 0xc0) {
                if (pos + 1 >= len) return HAL_RET_INVALID_ARG;
                if (end_pos == 0) end_pos = pos + 1;
                pos = id_pos + ((c & 0x3f) << 8) + packet[pos+1];
                next = pos;
            } else {
                name_len++;
                pos++;
                next = next + c + 1; 
            }
        } else {
            if (c >= '!' && c <= 'z' && c != '\\') name_len++;
            else name_len += 4;
            pos++;
        }
    }
    if (end_pos == 0) end_pos = pos;

    // Due to the nature of DNS name compression, it's possible to get a
    // name that is infinitely long. Return an error in that case.
    // We use the len of the packet as the limit, because it shouldn't 
    // be possible for the name to be that long.
    if (steps >= 2*len || pos >= len) return HAL_RET_INVALID_ARG;

    name_len++;

    *name_len_p = name_len;
    HAL_TRACE_DEBUG("Name len: {}", *name_len_p);
    if (name_len > MAX_DNS_DOMAINNAME_LEN) {
        HAL_TRACE_ERR("Domain name > 255 bytes");
        return HAL_RET_NOT_SUPPORTED;
    }
    pos = *packet_p;

    //Now actually assemble the name.
    //We've already made sure that we don't exceed the packet length, so
    // we don't need to make those checks anymore.
    // Non-printable and whitespace characters are replaced with a question
    // mark. They shouldn't be allowed under any circumstances anyway.
    // Other non-allowed characters are kept as is, as they appear sometimes
    // regardless.
    // This shouldn't interfere with IDNA (international
    // domain names), as those are ascii encoded.
    next = pos;
    i = 0;
    while (next != pos || packet[pos] != 0) {
        if (pos == next) {
            if ((packet[pos] & 0xc0) == 0xc0) {
                pos = id_pos + ((packet[pos] & 0x3f) << 8) + packet[pos+1];
                next = pos;
            } else {
                // Add a period except for the first time.
                if (i != 0) name[i++] = '.'; 
                next = pos + packet[pos] + 1;
                pos++;
            }
        } else {
            uint8_t c = packet[pos];
            if (c >= '!' && c <= '~' && c != '\\') {
                name[i] = packet[pos];
                i++; pos++;
            } else {
                name[i] = '\\';
                name[i+1] = 'x';
                name[i+2] = c/16 + 0x30;
                name[i+3] = c%16 + 0x30;
                if (name[i+2] > 0x39) name[i+2] += 0x27;
                if (name[i+3] > 0x39) name[i+3] += 0x27;
                i+=4; 
                pos++;
            }
        }
    }
    name[i] = 0;

    *packet_p = end_pos + 1;

    return HAL_RET_OK;
}

static hal_ret_t parse_dns_questions(sfw_info_t *sfw_info, uint8_t *pkt, 
                              uint16_t count, uint32_t *offset, 
                              uint32_t start_offset, uint32_t len) {
    dns_question current;
    uint16_t     i;
    char        *label = NULL;
    hal_ret_t    ret = HAL_RET_OK;
    char         *name = NULL;
    uint32_t     name_offset = *offset, label_len = 0, name_len= 0;

    HAL_TRACE_DEBUG("Parsing DNS question drop_large_domain: {} drop_long_label: {} count: {} offset: {}",
                    sfw_info->alg_opts.opt.dns_opts.drop_large_domain_name_packets,
                    sfw_info->alg_opts.opt.dns_opts.drop_long_label_packets, count, *offset);
    for (i=0; i < count; i++) {
        name = current.name;
        ret = read_rr_name(pkt, offset, start_offset, len, name, &name_len);
        if (ret != HAL_RET_OK) {
            if ((ret == HAL_RET_NOT_SUPPORTED &&
                 (sfw_info->alg_opts.opt.dns_opts.drop_large_domain_name_packets)) ||
                (ret == HAL_RET_INVALID_ARG)) { 
                 return ret; 
            }
            ret = HAL_RET_OK;
        }

        label_len = pkt[name_offset++];
        label = (char *)&pkt[name_offset];
        HAL_TRACE_DEBUG("Name Len: {} Name offset: {} pkt_len: {}", name_len, name_offset, len);
        name_len += name_offset;
        while (label != NULL && label_len != 0)
        {
            HAL_TRACE_DEBUG("Label: {} Label length: {}", label, label_len);
            if ((label_len > MAX_DNS_LABEL_LEN) && 
                sfw_info->alg_opts.opt.dns_opts.drop_long_label_packets) {
                HAL_TRACE_ERR("Label len > 63 bytes -- dropping packet");
                return HAL_RET_NOT_SUPPORTED;
            }
            if ((name_offset+label_len) == name_len) {
                //done parsing
                break;
            } else if ((name_offset+label_len) < name_len) {
                name_offset = name_offset+label_len;
                label_len = pkt[name_offset++];
                label = (char *)&pkt[name_offset];
            } else {
                HAL_TRACE_ERR("Label parse error label len: {} greater than name len: {}", label_len, name_len);
                label = NULL;
                return HAL_RET_INVALID_ARG;
            }
        } 
    }

    return ret;
}

static hal_ret_t parse_dns_packet (fte::ctx_t& ctx, uint16_t *dns_id)
{
    const uint8_t *pkt = ctx.pkt();
    uint32_t       offset = 0, ques_offset = 0;
    sfw_info_t    *sfw_info = sfw::sfw_feature_state(ctx);
    uint32_t       payload_offset = ctx.cpu_rxhdr()->payload_offset;
    uint8_t        opcode = 0;
    uint16_t       questions = 0;

    HAL_TRACE_DEBUG("Parsing DNS Packet");

    // Payload offset from CPU header
    offset = payload_offset;
    if (ctx.pkt_len() < (offset + DNS_HEADER_LENGTH)) {
        HAL_TRACE_ERR("Packet len: {} is less than DNS header size: {}", \
                      ctx.pkt_len(),  (offset+12));
        return HAL_RET_INVALID_ARG;
    }

    if ((ctx.pkt_len()-offset) > 
        sfw_info->alg_opts.opt.dns_opts.max_msg_length) {
        HAL_TRACE_ERR("DNS Message len: {} is greater than configured "\
                      "message length: {}",
                      (ctx.pkt_len()-offset),
                      sfw_info->alg_opts.opt.dns_opts.max_msg_length);
        // Increment errors
        return HAL_RET_NOT_SUPPORTED;
    }

    *dns_id = __pack_uint16(pkt, &offset);

    opcode = pkt[offset];
    if ((opcode >> DNS_OPCODE_SHIFT)) { // Not a query packet 
        HAL_TRACE_ERR("Not a Query packet -- dropping");
        // Increment errors
        return HAL_RET_NOT_SUPPORTED;
    }

    // Move the offset to read # of questions
    offset = offset + DNS_HEADER_OPCODE_LEN;
    HAL_TRACE_DEBUG("Offset: {}", offset);

    // Check for number of questions
    questions = __pack_uint16(pkt, &offset);
    if (questions > 1 && 
        sfw_info->alg_opts.opt.dns_opts.drop_multi_question_packets) {
        HAL_TRACE_ERR("Question count more than 1 -- dropping");
        // Increment errors
        return HAL_RET_NOT_SUPPORTED;
    }

    ques_offset = offset + DNS_HEADER_RR_LEN; 
    HAL_TRACE_DEBUG("Question offset: {}", ques_offset);
    return (parse_dns_questions(sfw_info, ctx.pkt(), questions, &ques_offset, 
                              payload_offset, ctx.pkt_len()));
}

bool get_dnsid_pkt (fte::ctx_t& ctx, l4_alg_status_t *sess)
{
    const uint8_t          *pkt = ctx.pkt();
    uint32_t                offset = 0;
    dns_info_t             *dns_info = (dns_info_t *)sess->info;
    uint16_t                dnsid = 0;
    uint8_t                 opcode = 0;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
    if (ctx.pkt_len() < offset) {
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        incr_parse_error(sess);
        return false;
    }

    // Fetch 2-byte opcode
    dnsid = __pack_uint16(pkt, &offset);
    HAL_TRACE_DEBUG("Received DNS id:{}", dnsid);

    // Not a response ? dont cleanup session yet!
    opcode = (pkt[offset]>>DNS_OPCODE_SHIFT);
    if (!opcode) 
        return false;

    if (dns_info->dnsid != dnsid) {
        HAL_TRACE_ERR("DNS ID received {} doesnt match the query {}", \
                      dnsid, dns_info->dnsid);
        incr_parse_error(sess);
        return false;
    }

    return true;
}

/*
 * DNS Exec
 */
fte::pipeline_action_t alg_dns_exec (fte::ctx_t &ctx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    app_session_t                   *app_sess = NULL;
    l4_alg_status_t                 *l4_sess = NULL;
    dns_info_t                      *dns_info = NULL;
    uint16_t                        dnsid = 0;
    sfw_info_t                      *sfw_info;
    fte::feature_session_state_t    *alg_state = NULL;
    fte::flow_update_t               flowupd;

    sfw_info = sfw::sfw_feature_state(ctx);
    if (hal::g_hal_state->is_flow_aware() || ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (alg_state != NULL && ctx.role() != hal::FLOW_ROLE_RESPONDER) {
        /* Session already exists - DNS response packet */
        l4_sess = (l4_alg_status_t *)alg_status(alg_state);
        if (!l4_sess) {
            HAL_TRACE_DEBUG("DNS ALG - L4 session is NULL");
            return fte::PIPELINE_CONTINUE;
        }
        if (l4_sess->alg != nwsec::APP_SVC_DNS) {
            HAL_TRACE_DEBUG("DNS ALG - L4 session type is NOT DNS");
            return fte::PIPELINE_CONTINUE;
        }
        HAL_TRACE_DEBUG("DNS ALG - Session exists");
        dns_info = (dns_info_t *)l4_sess->info;
        SDK_ASSERT(dns_info);

        /* Get the DNS id in the packet */
        bool response = get_dnsid_pkt(ctx, l4_sess);
      
        /* Cleanup if the response is seen 
         * There is a possibility that an old response
         * is sitting in the flow-miss queue along with 
         * a new query. In that case, we dont want to 
         * cleanup the session yet
         */
        if (ctx.alg_cflow() == true && response == true) { 
            if (dns_info->timer) {
                sdk::lib::timer_delete(dns_info->timer);
                dns_info->timer = NULL;
            }

            dns_info->response_rcvd = response;

            /* Now that we have seen a DNS response cleanup the session */
            session_delete_in_fte(ctx.session()->hal_handle);
        }
    } else if (sfw_info->alg_proto == nwsec::APP_SVC_DNS) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            /* Parse DNS packet and get dns id */
            ret = parse_dns_packet(ctx, &dnsid);
            if (ret == HAL_RET_NOT_SUPPORTED) {
                ctx.set_drop();
                HAL_TRACE_ERR("Dropping DNS ALG packet");
                return fte::PIPELINE_END;
            } else if (ret == HAL_RET_INVALID_ARG) {
                HAL_TRACE_ERR("Parsing errors found");
                return fte::PIPELINE_CONTINUE;
            }

            /* New DNS session */
            /* Alloc APP session */
            HAL_TRACE_DEBUG("DNS ALG - Got new session");
            ret = g_dns_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            /* Alloc L4 session */
            ret = g_dns_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
            SDK_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
            l4_sess->alg = nwsec::APP_SVC_DNS;
            /* Allocate dns info to store the dllist head */
            dns_info = (dns_info_t *)g_dns_state->alg_info_slab()->alloc();
            SDK_ASSERT_RETURN((dns_info != NULL), fte::PIPELINE_CONTINUE);
            /* Store the head node in L4 session info */
            l4_sess->info = (void *)dns_info;
            l4_sess->isCtrl = true;
            l4_sess->idle_timeout = sfw_info->idle_timeout;
            dns_info->dnsid = dnsid;
            dns_info->response_rcvd = false;

            /* Update the flow to receive Mcast copy */
            flowupd.type = fte::FLOWUPD_MCAST_COPY;
            flowupd.mcast_info.mcast_en = 1;
            flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
            flowupd.mcast_info.proxy_mcast_ptr = 0;
            ret = ctx.update_flow(flowupd);

            /*
             * Register Feature session state & completion handler
             */
            ctx.register_completion_handler(dns_completion_hdlr);
            ctx.register_feature_session_state(&l4_sess->fte_feature_state);
        } else if (ctx.feature_session_state() != NULL) {
            /* Update the flow to receive Mcast copy */
            flowupd.type = fte::FLOWUPD_MCAST_COPY;
            flowupd.mcast_info.mcast_en = 1;
            flowupd.mcast_info.mcast_ptr = P4_NW_MCAST_INDEX_FLOW_REL_COPY;
            flowupd.mcast_info.proxy_mcast_ptr = 0;
            ret = ctx.update_flow(flowupd);
        }
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace alg_dns
} // namespace plugins
} // namespace hal
