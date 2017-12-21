#include "nic/hal/src/proxy.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "app_redir_plugin.hpp"
#include "app_redir.hpp"
#include "app_redir_headers.hpp"
#include "app_redir_cb_ops.hpp"

namespace hal {
namespace app_redir {

/*
 * TODO: determine whether the operational flow key should
 * be pre or post rewrite.
 */
#define APP_REDIR_OPER_FLOW_KEY_PRE_REWRITE     1


const flow_key_t&
app_redir_oper_flow_key_get(fte::ctx_t& ctx)
{
    return APP_REDIR_OPER_FLOW_KEY_PRE_REWRITE ? ctx.key() :
                                                 ctx.get_key(ctx.role());
}


/*
 * Evaluate and set the reverse role
 */
static inline void
app_redir_rev_role_set(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    redir_ctx.set_chain_rev_role(ctx.role() == hal::FLOW_ROLE_INITIATOR ?
                                 hal::FLOW_ROLE_RESPONDER :
                                 hal::FLOW_ROLE_INITIATOR);
}


/*
 * Set pipeline action
 * (pipeline_action_t not available in fte_ctx.hpp which necessitates
 * using a bool to maintain state).
 */
static inline void
app_redir_pipeline_action_set(fte::ctx_t& ctx,
                              fte::pipeline_action_t action)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    redir_ctx.set_pipeline_end(action == fte::PIPELINE_END);
}


/*
 * Return pipeline action continue or end
 */
static inline fte::pipeline_action_t
app_redir_pipeline_action(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    return redir_ctx.pipeline_end() ?
           fte::PIPELINE_END : fte::PIPELINE_CONTINUE;
}


/*
 * Set feature status and return the same status.
 */
static hal_ret_t
app_redir_feature_status_set(fte::ctx_t& ctx,
                             hal_ret_t status,
                             bool pipeline_end_on_error=true)
{
    if (status != HAL_RET_OK) {
        ctx.set_feature_status(status);
        if (pipeline_end_on_error) {
            app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);
        }
    }

    return status;
}


/*
 * Establish service chaining by creating control blocks (cb) for
 * P4+ raw redirect programs.
 */
static hal_ret_t
app_redir_rawrcb_rawccb_create(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    rawrcb_t                rawrcb;
    rawccb_t                rawccb;
    hal_ret_t               ret = HAL_RET_OK;

    if (ctx.pkt()) {
        app_redir_rawccb_init(redir_ctx.chain_flow_id(), rawccb);
        app_redir_rawrcb_init(redir_ctx.chain_flow_id(), rawrcb);

        /*
         * For rawrcb, we need to set up chain_rxq_base and its related attributes.
         * However, when both chain_rxq_base and chain_txq_base are set to zero,
         * PD will automatically supply the correct ARQ defaults for chain_rxq_base.
         * When multi cores are supported, only the chain_rxq_ring_index_select
         * would need to be specified here.
         *
         * For rawccb, PD will supply the correct defaults when my_txq_base is
         * left at zero.
         */
        rawrcb.chain_rxq_ring_index_select = fte::fte_id();
        ret = app_redir_rawccb_create(rawccb);
        if (ret == HAL_RET_OK) {
            ret = app_redir_rawrcb_create(rawrcb);
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Establish service chaining by creating control blocks (cb) for
 * P4+ TCP/TLS proxy redirect programs.
 */
/*static*/ hal_ret_t
app_redir_proxyrcb_proxyccb_create(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    proxyrcb_t              proxyrcb;
    proxyccb_t              proxyccb;
    const flow_key_t&       flow_key = app_redir_oper_flow_key_get(ctx);
    hal_ret_t               ret = HAL_RET_OK;

    if (ctx.pkt()) {
        app_redir_proxyccb_init(app_redir_proxyc_oper_cb_id_eval(flow_key,
                                          redir_ctx.chain_flow_id()), proxyccb);
        app_redir_proxyrcb_init(app_redir_proxyr_oper_cb_id_eval(flow_key,
                                          redir_ctx.chain_flow_id()), proxyrcb);

        /*
         * For proxyrcb, we need to set up chain_rxq_base and its related
         * attributes. However, when chain_rxq_base is set to zero, PD will
         * automatically supply the correct ARQ defaults for chain_rxq_base.
         * When multi cores are supported, only the chain_rxq_ring_index_select
         * would need to be specified here.
         *
         * For proxyccb, PD will supply the correct defaults when my_txq_base
         * is left at zero, but for chain_txq info, appropriate value will be
         * constructed by app_redir_proxyccb_chain_txq_build().
         */
        proxyrcb.chain_rxq_ring_index_select = fte::fte_id();
        ret = app_redir_proxyccb_chain_txq_build(flow_key, proxyccb);
        if (ret == HAL_RET_OK) {
            ret = app_redir_proxyrcb_flow_key_build(flow_key, proxyrcb);
        }

        if (ret == HAL_RET_OK) {
            ret = app_redir_proxyccb_create(proxyccb);
        }
        if (ret == HAL_RET_OK) {
            ret = app_redir_proxyrcb_create(proxyrcb);
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Application packet Tx enqueue function
 */
static hal_ret_t
app_redir_pkt_tx_enqueue(fte::ctx_t& ctx)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    uint8_t                         *pkt;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          pkt_len;
    uint32_t                        flow_id = redir_ctx.chain_flow_id();
    hal::pd::cpu_to_p4plus_header_t cpu_header;
    hal::pd::p4plus_to_p4_header_t  p4_header;
    hal_ret_t                       ret;

    if (!ctx.pkt() || redir_ctx.chain_pkt_enqueued()) {
        return HAL_RET_OK;
    }

    /*
     * Register current ctx chain queue ID with cpupkt API
     * for sending packet.
     */
    cpu_rxhdr = ctx.cpu_rxhdr();
    memset(&cpu_header, 0, sizeof(cpu_header)); 
    memset(&p4_header, 0, sizeof(p4_header)); 
    cpu_header.flags = redir_ctx.redir_flags();
    cpu_header.src_lif = cpu_rxhdr->src_lif;
    p4_header.flags |= P4PLUS_TO_P4_FLAGS_LKP_INST;

    /*
     * Enqueue packet without app redirect headers
     */
    pkt = ctx.pkt();
    pkt_len = ctx.pkt_len();
    if (!redir_ctx.redir_miss_pkt_p()) {
        pkt += redir_ctx.hdr_len_total();
        pkt_len -= redir_ctx.hdr_len_total();
    }

    /*
     * For proxy flow, evaluate the correct CB for chaining to
     * either TCP or TLS.
     */
    if (redir_ctx.chain_wring_type() == types::WRING_TYPE_APP_REDIR_PROXYC) {
        flow_id = app_redir_proxyc_oper_cb_id_eval(ctx.key(), flow_id);
    }

    HAL_TRACE_DEBUG("{} flow_id {} pkt_len {} src_lif {}", __FUNCTION__,
                    flow_id, pkt_len, cpu_header.src_lif);
    ret = hal::pd::cpupkt_register_tx_queue(redir_ctx.arm_ctx(),
                                            redir_ctx.chain_wring_type(),
                                            flow_id);
    if (ret == HAL_RET_OK) {
        ret = ctx.queue_txpkt(pkt, pkt_len, &cpu_header, &p4_header,
                              SERVICE_LIF_APP_REDIR, redir_ctx.chain_qtype(),
                              flow_id, redir_ctx.chain_ring(),
                              redir_ctx.chain_wring_type());
    }

    redir_ctx.set_chain_pkt_enqueued(true);
    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Insert app redirect headers for a packet resulting in flow miss
 */
static hal_ret_t
app_redir_miss_hdr_insert(fte::ctx_t& ctx,
                          uint8_t format)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    pen_app_redir_header_v1_full_t& redir_miss_hdr = redir_ctx.redir_miss_hdr();
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          hdr_len = 0;
    hal_ret_t                       ret = HAL_RET_OK;

    redir_miss_hdr.app.h_proto = htons(PEN_APP_REDIR_ETHERTYPE);
    cpu_rxhdr = ctx.cpu_rxhdr();
    redir_miss_hdr.ver.format = format;
    switch (format) {

    case PEN_RAW_REDIR_V1_FORMAT:
    {
        hdr_len = PEN_APP_REDIR_VERSION_HEADER_SIZE +
                  PEN_RAW_REDIR_HEADER_V1_SIZE;
        redir_miss_hdr.ver.hdr_len = htons(hdr_len);
        redir_miss_hdr.raw.vrf = htons(cpu_rxhdr->lkp_vrf);
        redir_miss_hdr.raw.flags = htons(PEN_APP_REDIR_L3_CSUM_CHECKED |
                                         PEN_APP_REDIR_L4_CSUM_CHECKED);
        redir_miss_hdr.raw.flow_id = htonl(redir_ctx.chain_flow_id());
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());
        redir_miss_hdr.raw.redir_miss_pkt_p = (uint64_t)ctx.pkt();

        HAL_TRACE_DEBUG("{} hdr_len {} format {}", __FUNCTION__,
                        hdr_len + PEN_APP_REDIR_HEADER_SIZE, format);
        break;
    }

    case PEN_PROXY_REDIR_V1_FORMAT:
    {
        const flow_key_t&   flow_key = app_redir_oper_flow_key_get(ctx);

        hdr_len = PEN_APP_REDIR_VERSION_HEADER_SIZE +
                  PEN_PROXY_REDIR_HEADER_V1_SIZE;
        redir_miss_hdr.ver.hdr_len = htons(hdr_len);
        redir_miss_hdr.proxy.vrf = htons(cpu_rxhdr->lkp_vrf);
        redir_miss_hdr.proxy.flags = htons(PEN_APP_REDIR_L3_CSUM_CHECKED |
                                           PEN_APP_REDIR_L4_CSUM_CHECKED);
        redir_miss_hdr.proxy.flow_id = htonl(redir_ctx.chain_flow_id());
        assert(cpu_rxhdr->payload_offset > 0);
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());
        redir_miss_hdr.proxy.redir_miss_pkt_p = (uint64_t)ctx.pkt() +
                                                cpu_rxhdr->payload_offset;
        redir_miss_hdr.proxy.tcp_flags = cpu_rxhdr->tcp_flags;

        /*
         * Construct flow key for proxy header
         */
        redir_miss_hdr.proxy.ip_proto = IPPROTO_TCP;
        redir_miss_hdr.proxy.sport = htons(flow_key.sport);
        redir_miss_hdr.proxy.dport = htons(flow_key.dport);
        if (flow_key.flow_type == hal::FLOW_TYPE_V4) {
            redir_miss_hdr.proxy.af = AF_INET;
            redir_miss_hdr.proxy.ip_sa[0] = htonl(flow_key.sip.v4_addr);
            redir_miss_hdr.proxy.ip_da[0] = htonl(flow_key.dip.v4_addr);
        } else {
            redir_miss_hdr.proxy.af = AF_INET6;
            memcpy(&redir_miss_hdr.proxy.ip_sa[0], &flow_key.sip.v6_addr,
                   sizeof(redir_miss_hdr.proxy.ip_sa));
            memcpy(&redir_miss_hdr.proxy.ip_da[0], &flow_key.dip.v6_addr,
                   sizeof(redir_miss_hdr.proxy.ip_da));
        }

        HAL_TRACE_DEBUG("{} hdr_len {} format {}", __FUNCTION__,
                        hdr_len + PEN_APP_REDIR_HEADER_SIZE, format);
        break;
    }

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__, format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        break;
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Validate Pensando app header in redirected packet.
 */
static hal_ret_t
app_redir_app_hdr_validate(fte::ctx_t& ctx)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    pen_app_redir_header_v1_full_t  *app_hdr;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    hal::l2seg_t                    *l2seg;
    flow_key_t                      flow_key = {0};
    size_t                          pkt_len;
    size_t                          hdr_len;
    uint16_t                        h_proto;
    uint16_t                        flags = 0;
    hal_ret_t                       ret = HAL_RET_OK;

    app_hdr = (pen_app_redir_header_v1_full_t *)ctx.pkt();
    pkt_len = ctx.pkt_len();
    if (redir_ctx.redir_miss_pkt_p()) {

        /*
         * Assume certain pkt_len for validation below
         */
        app_hdr = &redir_ctx.redir_miss_hdr();
        pkt_len += redir_ctx.redir_miss_hdr_size();
    }

    if (pkt_len < PEN_APP_REDIR_HEADER_MIN_SIZE) {
        HAL_TRACE_ERR("{} pkt_len {} too small", __FUNCTION__, pkt_len);
        ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
        goto done;
    }

    cpu_rxhdr = ctx.cpu_rxhdr();
    h_proto = ntohs(app_hdr->app.h_proto);
    hdr_len = ntohs(app_hdr->ver.hdr_len);
    HAL_TRACE_DEBUG("{} pkt_len {} h_proto {:#x} hdr_len {} format {}",
                    __FUNCTION__, pkt_len, h_proto, hdr_len, app_hdr->ver.format);
    if (h_proto != PEN_APP_REDIR_ETHERTYPE) {
        HAL_TRACE_ERR("{} unexpected h_proto {:#x}", __FUNCTION__, h_proto);
        ret = HAL_RET_APP_REDIR_HDR_ERR;
        goto done;
    }

    switch (app_hdr->ver.format) {

    case PEN_RAW_REDIR_V1_FORMAT:

        /*
         * Raw vrf not accessible from P4+ so have to set it here
         */
        flags = ntohs(app_hdr->raw.flags);
        app_hdr->raw.vrf = cpu_rxhdr->lkp_vrf;
        HAL_TRACE_DEBUG("{} flow_id {:#x} flags {:#x} vrf {}", __FUNCTION__,
                        ntohl(app_hdr->raw.flow_id), flags, app_hdr->raw.vrf);
        if ((pkt_len < PEN_RAW_REDIR_HEADER_V1_FULL_SIZE) ||
            ((hdr_len - PEN_APP_REDIR_VERSION_HEADER_SIZE) != 
                        PEN_RAW_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid pkt_len {} or hdr_len {}",
                          __FUNCTION__, pkt_len, app_hdr->ver.format, hdr_len);
            ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
            goto done;
        }

        redir_ctx.set_chain_qtype(APP_REDIR_RAWC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_RAWC);
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        flags = ntohs(app_hdr->proxy.flags);
        HAL_TRACE_DEBUG("{} flow_id {:#x} flags {:#x} vrf {}", __FUNCTION__,
                        ntohl(app_hdr->proxy.flow_id), flags, app_hdr->proxy.vrf);
        if ((pkt_len < PEN_PROXY_REDIR_HEADER_V1_FULL_SIZE) ||
            ((hdr_len - PEN_APP_REDIR_VERSION_HEADER_SIZE) != 
                        PEN_PROXY_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid pkt_len {} or hdr_len {}",
                          __FUNCTION__, pkt_len, app_hdr->ver.format, hdr_len);
            ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
            goto done;
        }

        /*
         * For proxy flow miss, the current packet came with full L2/L3/L4 headers
         * from which FTE already extracted the key. For flow hit, only the
         * app_redir proxy header is available and we have to extract the
         * flow key on FTE's behalf.
         */
        if (!ctx.flow_miss()) {
            flow_key.vrf_id = ntohs(app_hdr->proxy.vrf);
            l2seg =  hal::pd::find_l2seg_by_hwid(flow_key.vrf_id);
            if (l2seg) {
                flow_key.vrf_id = hal::vrf_lookup_by_handle(l2seg->vrf_handle)->vrf_id; 
            } else {
                HAL_TRACE_ERR("{} l2seg not found, hwid={}", __FUNCTION__,
                              flow_key.vrf_id);
            }

            if (app_hdr->proxy.af == AF_INET) {
                flow_key.flow_type = hal::FLOW_TYPE_V4;
                flow_key.sip.v4_addr = ntohl(app_hdr->proxy.ip_sa[0]);
                flow_key.dip.v4_addr = ntohl(app_hdr->proxy.ip_da[0]);
            } else {
                flow_key.flow_type = hal::FLOW_TYPE_V6;
                memcpy(&flow_key.sip.v6_addr, &app_hdr->proxy.ip_sa[0],
                       sizeof(flow_key.sip.v6_addr));
                memcpy(&flow_key.dip.v6_addr, &app_hdr->proxy.ip_da[0],
                       sizeof(flow_key.dip.v6_addr));
            }

            flow_key.proto = (types::IPProtocol)app_hdr->proxy.ip_proto;
            flow_key.sport = ntohs(app_hdr->proxy.sport);
            flow_key.dport = ntohs(app_hdr->proxy.dport);
            flow_key.dir = FLOW_DIR_FROM_DMA;
            if (ntohl(app_hdr->proxy.flow_id) & 
                               PROXYR_OPER_CB_OFFSET(PROXYR_TLS_PROXY_DIR)) {
                assert(PROXYR_TLS_PROXY_DIR);
                flow_key.dir = FLOW_DIR_FROM_UPLINK;
            }

            ctx.set_key(flow_key);
        }

        redir_ctx.set_chain_qtype(APP_REDIR_PROXYC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_PROXYC);
        break;

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__,
                      app_hdr->ver.format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        goto done;
    }

    redir_ctx.set_redir_flags(flags);
    redir_ctx.set_hdr_len_total(hdr_len + PEN_APP_REDIR_HEADER_SIZE);

done:
    app_redir_feature_status_set(ctx, ret);
    return ret;
}


/*
 * Return pointer to current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
uint8_t *
app_redir_pkt(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    return redir_ctx.redir_miss_pkt_p() ?
           (uint8_t *)&redir_ctx.redir_miss_hdr() : ctx.pkt();
}


/*
 * Return length of current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
size_t
app_redir_pkt_len(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    const fte::cpu_rxhdr_t  *cpu_rxhdr;
    size_t                  pkt_len;

    pkt_len = ctx.pkt_len();
    if (redir_ctx.redir_miss_pkt_p()) {
        if (redir_ctx.tcp_tls_proxy_flow()) {
            cpu_rxhdr = ctx.cpu_rxhdr();
            assert((cpu_rxhdr->payload_offset > 0) &&
                   ((int)pkt_len >= cpu_rxhdr->payload_offset));
            pkt_len -= cpu_rxhdr->payload_offset;
        }

        pkt_len += redir_ctx.hdr_len_total();
    }

    return pkt_len;
}


/*
 * Forward redirected packet to application or simply loop it back (DOL case).
 */
static hal_ret_t
app_redir_pkt_process(fte::ctx_t& ctx)
{
    return app_redir_app_hdr_validate(ctx);
}


/*
 * Return proxy info for the current flow.
 */
static proxy_flow_info_t *
app_redir_proxy_flow_info_find(fte::ctx_t& ctx,
                               flow_key_t& flow_key,
                               bool include_tcp_tls_flows)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    proxy_flow_info_t       *pfi;

    pfi = redir_ctx.proxy_flow_info() ?
          redir_ctx.proxy_flow_info() :
          proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR, &flow_key);

    if (!pfi && include_tcp_tls_flows) {
        /*
         * See if flow was configured as TCP/TLS proxy flow
         */
        pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, &flow_key);
        if (pfi) {
            redir_ctx.set_tcp_tls_proxy_flow(true);
        }
    }

    if (!pfi) {
        HAL_TRACE_DEBUG("app_redir is not applicable for the flow");
    }
    return pfi;
}


/*
 * Return proxy info for the current flow if, by configuration or by certain
 * test criteria, the flow is subject to app redirect. 
 */
static proxy_flow_info_t *
app_redir_proxy_flow_info_get(fte::ctx_t& ctx,
                              flow_key_t& flow_key,
                              bool include_tcp_tls_flows)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    proxy_flow_info_t       *pfi;
    proxy_flow_info_t       *rpfi = NULL;
    flow_key_t              rev_flow_key;
    hal_ret_t               ret = HAL_RET_OK;

    pfi = app_redir_proxy_flow_info_find(ctx, flow_key, include_tcp_tls_flows);
    if (!pfi) {

        /*
         * Create flow only for flow miss case
         */
        if (!ctx.pkt() || !ctx.flow_miss() || !redir_ctx.redir_policy_applic()) {
            goto done;
        }

        if (ctx.existing_session()) {

            /*
             * Ignore direction. Always set it to 0 for flow_info lookup.
             */
            rev_flow_key = ctx.get_key(redir_ctx.chain_rev_role());
            rev_flow_key.dir = 0;
            rpfi = app_redir_proxy_flow_info_find(ctx, rev_flow_key,
                                                  include_tcp_tls_flows);
            if (!rpfi) {
                HAL_TRACE_DEBUG("app_redir existing_session for rev_role {} "
                                "not found", redir_ctx.chain_rev_role());

                ret = HAL_RET_FLOW_NOT_FOUND;
                goto done;
            }
        }

        /*
         * Create the flow
         */
        ret = proxy_flow_enable(types::PROXY_TYPE_APP_REDIR, flow_key,
                                !ctx.existing_session(), NULL, NULL);
        if (ret != HAL_RET_OK) {
            goto done;
        }

        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR, &flow_key);
        assert(pfi);
        if (rpfi) {
            pfi->qid1 = rpfi->qid1;
            pfi->qid2 = rpfi->qid2;
        }
    }

    if (pfi) {
        redir_ctx.set_proxy_flow_info(pfi);
        redir_ctx.set_redir_policy_applic(true);
        if (!redir_ctx.appid_started()) {
            redir_ctx.set_appid_needed();
        }
    }

done:
    app_redir_feature_status_set(ctx, ret);
    return pfi;
}


/*
 * Update fwding info for the current flow, redirecting it to us.
 */
static hal_ret_t
app_redir_flow_fwding_update(fte::ctx_t& ctx,
                             proxy_flow_info_t *pfi,
                             flow_key_t& flow_key)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    fte::flow_update_t      flowupd = {type: fte::FLOWUPD_FWDING_INFO};
    hal_ret_t               ret = HAL_RET_OK;

    /*
     * Update fwding info if applicable
     */
    assert(pfi);
    flowupd.fwding.qid = ctx.role() ==  hal::FLOW_ROLE_INITIATOR ?
                         pfi->qid1 : pfi->qid2;
    redir_ctx.set_chain_flow_id(flowupd.fwding.qid);
    if (!redir_ctx.tcp_tls_proxy_flow()) {
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = APP_REDIR_RAWR_QTYPE;

        HAL_TRACE_DEBUG("app_redir flow forwarding role: {} qid1: {} qid2: {}",
                        ctx.role(), pfi->qid1, pfi->qid2);
        HAL_TRACE_DEBUG("app_redir updating lport = {} for sport = {} dport = {}",
                        flowupd.fwding.lport, flow_key.sport, flow_key.dport);
        ret = ctx.update_flow(flowupd);
    }
    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Wrapper for app_redir_proxy_flow_info_get() and app_redir_flow_fwding_update().
 */
static hal_ret_t
app_redir_proxy_flow_info_update(fte::ctx_t& ctx,
                                 bool include_tcp_tls_flows)
{
    flow_key_t              flow_key;
    proxy_flow_info_t       *pfi;
    hal_ret_t               ret = HAL_RET_OK;

    /*
     * Ignore direction. Always set it to 0 for flow_info lookup.
     */
    flow_key = ctx.key();
    flow_key.dir = 0;
    pfi = app_redir_proxy_flow_info_get(ctx, flow_key, include_tcp_tls_flows);
    if (pfi) {
        ret = app_redir_flow_fwding_update(ctx, pfi, flow_key);
    }

    return ret;
}


/*
 * Set app redirct verdict for the current packet, independent of any 
 * FTE drop decision made in other pipelines/features.
 */
void
app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                          app_redir_verdict_t verdict)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    HAL_TRACE_DEBUG("{} verdict {}", __FUNCTION__, verdict);
    redir_ctx.set_chain_pkt_verdict(verdict);
}


/*
 * Exported API for outside features to indicate redirect policy
 * applicability for the current flow.
 */
hal_ret_t
app_redir_policy_applic_set(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t               ret = HAL_RET_OK;

    redir_ctx.set_redir_policy_applic(true);

    /*
     * Update flow to redirect to us.
     */
    if (ctx.pkt() && ctx.flow_miss()) {

        app_redir_proxy_flow_info_update(ctx, true);
        if (!redir_ctx.redir_miss_pkt_p()) {

            /*
             * Insert app redirect header for the flow miss case
             */
            ret = app_redir_miss_hdr_insert(ctx, 
                                     redir_ctx.tcp_tls_proxy_flow() ?
                                     PEN_PROXY_REDIR_V1_FORMAT          :
                                     PEN_RAW_REDIR_V1_FORMAT);
            if (ret == HAL_RET_OK) {
                ret = app_redir_pkt_process(ctx);
            }
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * App redirect pipeline flow miss exec handler
 */
fte::pipeline_action_t
app_redir_miss_exec(fte::ctx_t& ctx)
{
    /*
     * Evaluate initial flow ID
     */
    app_redir_rev_role_set(ctx);
    app_redir_proxy_flow_info_update(ctx, false);

    return app_redir_pipeline_action(ctx);
}


/*
 * App redirect pipeline flow hit exec handler
 */
fte::pipeline_action_t
app_redir_exec(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    assert(ctx.pkt());

    /*
     * Evaluate initial flow ID
     */
    redir_ctx.set_chain_flow_id(ctx.cpu_rxhdr()->qid);
    redir_ctx.set_redir_policy_applic(true);

    app_redir_rev_role_set(ctx);
    app_redir_pkt_process(ctx);
    return app_redir_pipeline_action(ctx);
}


/*
 * Presumably executed as last feature in the redirect pipeline:
 * send or block the current packet based on the type of redirect
 * and packet verdict.
 */
fte::pipeline_action_t
app_redir_exec_fini(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t               ret = HAL_RET_OK;

    if (!ctx.drop()) {

        if (ctx.flow_miss()) {

            if (redir_ctx.proxy_flow_info()) {

                /*
                 * Note that FTE postpones the flow programming until after
                 * the pipeline exits so the ordering here actually
                 * works out correctly, that is, the CBs will end up
                 * getting programmed first.
                 */
                if (ctx.pkt()) {
                    ret = redir_ctx.tcp_tls_proxy_flow() ?
                          app_redir_proxyrcb_proxyccb_create(ctx) :
                          app_redir_rawrcb_rawccb_create(ctx);
                }
            }
        }

        /*
         * Queue the packet for forwarding to the next service chain if not
         * dropped by any app features and the packet did not come from SPAN.
         */
        if ((ret == HAL_RET_OK)                  &&
            redir_ctx.redir_policy_applic()      &&
            redir_ctx.chain_pkt_verdict_pass()   &&
            !redir_ctx.chain_pkt_span_instance()) {

            app_redir_pkt_tx_enqueue(ctx);
        }
    }

    return app_redir_pipeline_action(ctx);
}


} // namespace app_redir
} // namespace hal
