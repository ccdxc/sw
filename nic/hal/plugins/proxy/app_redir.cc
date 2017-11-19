#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "app_redir.hpp"

namespace hal {
namespace proxy {

/*
 * Evaluate and set the reverse role
 */
static inline void
app_redir_rev_role_set(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

    redir_ctx.set_pipeline_end(action == fte::PIPELINE_END);
}


/*
 * Return pipeline action continue or end
 */
static inline fte::pipeline_action_t
app_redir_pipeline_action(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    rawrcb_t                rawrcb;
    rawccb_t                rawccb;
    hal_ret_t               ret = HAL_RET_OK;

    if (ctx.pkt()) {
        app_redir_rawccb_init(&rawccb);
        app_redir_rawrcb_init(&rawrcb);

        /*
         * For rawrcb, we need to set up chain_rxq_base and its related attributes.
         * However, when both chain_rxq_base and chain_txq_base are set to zero,
         * PD will automatically supply the correct ARQ defaults for chain_rxq_base.
         * When multi cores are supported, only the chain_rxq_ring_index_select
         * would need to be specified here.
         *
         * For rawccb, PD will supply the correct defaults when my_txq_base is
         * left as zero.
         */
        ret = app_redir_rawccb_create(redir_ctx.chain_flow_id(), &rawccb);
        if (ret == HAL_RET_OK) {
            ret = app_redir_rawrcb_create(redir_ctx.chain_flow_id(), &rawrcb);
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
    fte::app_redir_ctx_t&           redir_ctx = ctx.app_redir();
    uint8_t                         *pkt;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          pkt_len;
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

    HAL_TRACE_DEBUG("{} pkt_len {} src_lif {}", __FUNCTION__,
                    pkt_len, cpu_header.src_lif);
    ret = hal::pd::cpupkt_register_tx_queue(redir_ctx.arm_ctx(),
                                            redir_ctx.chain_wring_type(),
                                            redir_ctx.chain_flow_id());
    if (ret == HAL_RET_OK) {
        ret = ctx.queue_txpkt(pkt, pkt_len, &cpu_header, &p4_header,
                              SERVICE_LIF_APP_REDIR, redir_ctx.chain_qtype(),
                              redir_ctx.chain_flow_id(), redir_ctx.chain_ring(),
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
    fte::app_redir_ctx_t&           redir_ctx = ctx.app_redir();
    pen_app_redir_header_v1_full_t& redir_miss_hdr = redir_ctx.redir_miss_hdr();
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          hdr_len = 0;
    hal_ret_t                       ret = HAL_RET_OK;

    redir_miss_hdr.app.h_proto = htons(PEN_APP_REDIR_ETHERTYPE);
    cpu_rxhdr = ctx.cpu_rxhdr();
    switch (format) {

    case PEN_RAW_REDIR_V1_FORMAT:
        redir_miss_hdr.ver.format = format;
        hdr_len = PEN_APP_REDIR_VERSION_HEADER_SIZE +
                  PEN_RAW_REDIR_HEADER_V1_SIZE;
        redir_miss_hdr.ver.hdr_len = htons(hdr_len);
        redir_miss_hdr.raw.vrf = cpu_rxhdr->lkp_vrf;
        redir_miss_hdr.raw.flags = htons(PEN_APP_REDIR_L3_CSUM_CHECKED |
                                         PEN_APP_REDIR_L4_CSUM_CHECKED);
        redir_miss_hdr.raw.flow_id = redir_ctx.chain_flow_id();
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());
        redir_miss_hdr.raw.redir_miss_pkt_p =(uint64_t)ctx.pkt();

        HAL_TRACE_DEBUG("{} hdr_len {} format {}", __FUNCTION__,
                        hdr_len + PEN_APP_REDIR_HEADER_SIZE, format);
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        HAL_TRACE_ERR("{} proxy hdr insertion coming soon", __FUNCTION__);
        ret = HAL_RET_ERR;
        break;

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
    fte::app_redir_ctx_t&           redir_ctx = ctx.app_redir();
    pen_app_redir_header_v1_full_t  *app_hdr;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
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
 * Forward redirected packet to application or simply loop it back (DOL case).
 */
static hal_ret_t
app_redir_pkt_process(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    hal_ret_t               ret;

    ret = app_redir_app_hdr_validate(ctx);
    if (ret == HAL_RET_OK) {

        /*
         * DOL can force an immediate packet send
         */
        if (redir_ctx.redir_flags() & PEN_APP_REDIR_PIPELINE_LOOPBK_EN) {

            /*
             * TODO: set a state in appid feature to skip detection
             */
        }
    }

    return ret;
}


/*
 * Criteria check for appid DOL testing
 */
static bool
app_redir_test_flow_criteria_check(fte::ctx_t& ctx)
{
#if 1
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    flow_key_t              flow_key = ctx.key();

    if (ctx.pkt()) {
        switch (flow_key.flow_type) {

        case hal::FLOW_TYPE_V4:
        case hal::FLOW_TYPE_V6:

            if (flow_key.proto == IPPROTO_TCP) {

#define APP_REDIR_KISMET_PORT       2501
#define APP_REDIR_MYSQL_PORT        3306

                if ((flow_key.sport == APP_REDIR_KISMET_PORT)   ||
                    (flow_key.dport == APP_REDIR_KISMET_PORT)   ||
                    (flow_key.sport == APP_REDIR_MYSQL_PORT)    ||
                    (flow_key.dport == APP_REDIR_MYSQL_PORT)) {

                    redir_ctx.set_redir_policy_applic(true);
                    return true;
                }
            }
            break;

        default:
            break;
        }
    }
#endif

    return false;
}


/*
 * Return proxy info for the current flow if, by configuration or by certain
 * test criteria, the flow is subject to app redirect. 
 */
static proxy_flow_info_t *
app_redir_proxy_flow_info_get(fte::ctx_t& ctx,
                              flow_key_t& flow_key)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    proxy_flow_info_t       *pfi = NULL;
    proxy_flow_info_t       *rpfi = NULL;
    flow_key_t              rev_flow_key;
    hal_ret_t               ret = HAL_RET_OK;

    /*
     * The flow might already have been created by the test infra,
     * in which case, return it.
     */
    pfi = redir_ctx.proxy_flow_info() ?
          redir_ctx.proxy_flow_info() :
          proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR, &flow_key);
    if (!pfi) {

        /*
         * Create flow only for flow hit case
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
            rpfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR, &rev_flow_key);
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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    fte::flow_update_t      flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    assert(pfi);
    HAL_TRACE_DEBUG("app_redir flow forwarding role: {} qid1: {} qid2: {}",
                    ctx.role(), pfi->qid1, pfi->qid2);
    HAL_TRACE_DEBUG("app_redir updating lport = {} for sport = {} dport = {}",
                    pfi->proxy->meta->lif_info[0].lport_id,
                    flow_key.sport, flow_key.dport);

    // update fwding info
    flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
    flowupd.fwding.qid_en = true;
    flowupd.fwding.qtype = APP_REDIR_RAWR_QTYPE;
    flowupd.fwding.qid = ctx.role() ==  hal::FLOW_ROLE_INITIATOR ?
                         pfi->qid1 : pfi->qid2;
    redir_ctx.set_chain_flow_id(flowupd.fwding.qid);
    return app_redir_feature_status_set(ctx, ctx.update_flow(flowupd));
}


/*
 * Wrapper for app_redir_proxy_flow_info_get() and app_redir_flow_fwding_update().
 */
static hal_ret_t
app_redir_proxy_flow_info_update(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    flow_key_t              flow_key;
    proxy_flow_info_t       *pfi;
    hal_ret_t               ret = HAL_RET_OK;

    /*
     * Ignore direction. Always set it to 0 for flow_info lookup.
     */
    flow_key = ctx.key();
    flow_key.dir = 0;
    pfi = app_redir_proxy_flow_info_get(ctx, flow_key);
    if (pfi) {
        redir_ctx.set_proxy_flow_info(pfi);
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
                          fte::app_redir_verdict_t verdict)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
    hal_ret_t               ret = HAL_RET_OK;

    redir_ctx.set_redir_policy_applic(true);

    /*
     * Update flow to redirect to us.
     */
    if (ctx.pkt() && ctx.flow_miss()) {

        app_redir_proxy_flow_info_update(ctx);
        if (!redir_ctx.redir_miss_pkt_p()) {

            /*
             * Insert app redirect header for the flow miss case
             */
            ret = app_redir_miss_hdr_insert(ctx, PEN_RAW_REDIR_V1_FORMAT);
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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

    /*
     * Evaluate initial flow ID
     */
    app_redir_rev_role_set(ctx);
    if (ctx.pkt()) {
        redir_ctx.set_chain_flow_id(ctx.cpu_rxhdr()->qid);
        if (app_redir_test_flow_criteria_check(ctx)) {
            app_redir_proxy_flow_info_update(ctx);
        }

    } else {

        /*
         * Handle proxy flow update for DOL testing purposes, i.e.,
         * when infra creates these flows before any packets are sent.
         */
        app_redir_proxy_flow_info_update(ctx);
    }

    return app_redir_pipeline_action(ctx);
}


/*
 * App redirect pipeline flow hit exec handler
 */
fte::pipeline_action_t
app_redir_exec(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();

    assert(ctx.pkt());

    /*
     * Evaluate initial flow ID
     */
    app_redir_rev_role_set(ctx);
    redir_ctx.set_chain_flow_id(ctx.cpu_rxhdr()->qid);

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
    fte::app_redir_ctx_t&   redir_ctx = ctx.app_redir();
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
                if ((ret == HAL_RET_OK) && ctx.pkt()) {
                    ret = app_redir_rawrcb_rawccb_create(ctx);
                }
            }
        }

        /*
         * Queue the packet for forwarding to the next service chain if not
         * dropped by any app features and the packet did not come from SPAN.
         */
        if ((ret == HAL_RET_OK)                 &&
            redir_ctx.redir_policy_applic()     &&
            redir_ctx.chain_pkt_verdict_pass()  &&
            !redir_ctx.chain_pkt_span_instance()) {

            app_redir_pkt_tx_enqueue(ctx);
        }
    }

    return app_redir_pipeline_action(ctx);
}


} // namespace proxy
} // namespace hal
