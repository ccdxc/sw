/*
 * core.cc
 */

#include "core.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_dns {

using namespace hal::plugins::alg_utils;
using namespace hal::plugins::sfw;

static void incr_parse_error (l4_alg_status_t *sess)
{
    HAL_ATOMIC_INC_UINT32(&((dns_info_t *)sess->info)->parse_errors, 1);
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
 * DNS ALG completion handler - invoked when the session creation is done.
 */
static void dns_completion_hdlr (fte::ctx_t& ctx, bool status) {
    /* Nothing to be done for now */
    return;
}

bool check_dnsid_exists (dns_info_t *head, uint16_t dnsid, dns_info_t **out) 
{
    dllist_ctxt_t   *next, *entry;

    HAL_TRACE_DEBUG("DNS ALG - Iterating through the linked list");
    dllist_for_each_safe(entry, next, &head->lentry)
    {
        dns_info_t *dentry = dllist_entry(entry, dns_info_t, lentry);
        HAL_TRACE_DEBUG("DNS ALG - Got DNS id: %d", dentry->dnsid);
        if (dentry->dnsid == dnsid) {
            *out = dentry;
            return (TRUE);
        }
    }
    return (FALSE);
}

hal_ret_t get_dnsid_pkt (fte::ctx_t& ctx, l4_alg_status_t *sess,
                         uint16_t *dnsid)
{
    hal_ret_t               ret = HAL_RET_OK;
    const uint8_t          *pkt = ctx.pkt();
    uint32_t                offset = 0;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
    if (ctx.pkt_len() < offset) {
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        incr_parse_error(sess);
        return HAL_RET_INVALID_ARG;
    }
    // Fetch 2-byte opcode
    *dnsid = __pack_uint16(pkt, &offset);
    HAL_TRACE_DEBUG("Received DNS id:{}", *dnsid);
    // Rflow is set to invalid so that flow miss happens for the DNS response
    // coming in the reverse direction
    // TODO: Change reverse flow to redirect entry instead
    ctx.set_valid_rflow(false);
    return ret;
}

/*
 * DNS Exec 
 */
fte::pipeline_action_t alg_dns_exec (fte::ctx_t &ctx)
{
    hal_ret_t                       ret = HAL_RET_OK;
    app_session_t                   *app_sess = NULL;
    l4_alg_status_t                 *l4_sess = NULL;
    dns_info_t                      *dns_info = NULL, *dns_out = NULL;
    uint16_t                        dnsid = 0;
    sfw_info_t                      *sfw_info;
    fte::feature_session_state_t    *alg_state = NULL;
    dllist_ctxt_t                   *head = NULL;

    sfw_info = (sfw_info_t *) ctx.feature_state(FTE_FEATURE_SFW);
    if (ctx.protobuf_request() || 
        ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        return fte::PIPELINE_CONTINUE;
    }
    if (sfw_info->alg_proto != nwsec::APP_SVC_DNS) {
        return fte::PIPELINE_CONTINUE;
    }
 
    alg_state = ctx.feature_session_state();
    if (alg_state != NULL) {
        /* Session already exists - DNS response packet */
        l4_sess = (l4_alg_status_t *) alg_status(alg_state); 
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
        HAL_ASSERT(dns_info);
        /* Get the DNS id in the packet */
        ret = get_dnsid_pkt(ctx, l4_sess, &dnsid);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("DNS ALG - Failed to get DNS id from pkt");
            return fte::PIPELINE_CONTINUE;
        }
        /* Check if the dns-id is present in the list */
        if (check_dnsid_exists(dns_info, dnsid, &dns_out)) {
            /* We got a DNS response for this (session, dnsid). Remove ent */
            dllist_del(&dns_out->lentry);
            /* Delete node from the slab */
            g_dns_state->alg_info_slab()->free(dns_out);
            /* If there are no more dnsids associated with this session then
             * it can be cleaned up*/
            if (dllist_empty(&dns_info->lentry)) {
                /* TODO: Cleanup session upon timer expiry */
                HAL_TRACE_DEBUG("DNS ALG - No more DNSids. Cleanup session!");
                g_dns_state->cleanup_app_session(l4_sess->app_session);
            }
        }
    } else {
        /* New DNS session */
        /* Alloc APP session */
        HAL_TRACE_DEBUG("DNS ALG - Got new session");
        ret = g_dns_state->alloc_and_init_app_sess(ctx.key(), &app_sess);
        HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
        /* Alloc L4 session */
        ret = g_dns_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
        HAL_ASSERT_RETURN((ret == HAL_RET_OK), fte::PIPELINE_CONTINUE);
        l4_sess->alg = nwsec::APP_SVC_DNS;
        /* Allocate dns info to store the dllist head */
        dns_info = (dns_info_t *)g_dns_state->alg_info_slab()->alloc();
        HAL_ASSERT_RETURN((dns_info != NULL), fte::PIPELINE_CONTINUE);
        /* Get DNS id in the packet */
        ret = get_dnsid_pkt(ctx, l4_sess, &dnsid);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("DNS ALG - Failed to get DNS id from pkt");
            return fte::PIPELINE_CONTINUE;
        }
        dllist_reset(&dns_info->lentry);
        /* Store the head node in L4 session info */
        l4_sess->info = (void *)dns_info;
        head = &dns_info->lentry;

        /* Allocate dns info to store the next node */
        dns_info = (dns_info_t *)g_dns_state->alg_info_slab()->alloc();
        HAL_ASSERT_RETURN((dns_info != NULL), fte::PIPELINE_CONTINUE);
        dllist_reset(&dns_info->lentry);
        dns_info->dnsid = dnsid;
        /* Add node to the linked list */
        /* TODO: Start a timer for every DNS session */
        dllist_add(head, &dns_info->lentry);
        /*
         * Register Feature session state & completion handler
         */
        ctx.register_completion_handler(dns_completion_hdlr);
        ctx.register_feature_session_state(&l4_sess->fte_feature_state);
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace alg_dns
} // namespace plugins
} // namespace hal
