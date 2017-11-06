#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/include/app_redir_headers.hpp"

bool
fte::app_redir_ctx_t::chain_pkt_verdict_pass(void) {

    return !(redir_flags_ & PEN_APP_REDIR_SPAN_INSTANCE) &&
           (chain_pkt_verdict_ != fte::APP_REDIR_VERDICT_BLOCK);
}

namespace hal {
namespace proxy {

static thread_local hal::pd::cpupkt_ctxt_t* app_chain_ctx;

static hal_ret_t
app_redir_init(fte::ctx_t& ctx)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (app_chain_ctx == NULL)   {
        app_chain_ctx = hal::pd::cpupkt_ctxt_alloc_init();
        if (app_chain_ctx == NULL)   {
            ret = HAL_RET_NO_RESOURCE;
        }
    }

    return ret;
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
 * Exported application packet send function
 */
static hal_ret_t
app_redir_pkt_send(fte::ctx_t& ctx)
{
    fte::app_redir_ctx_t&           redir_ctx = ctx.app_redir();
    uint8_t                         *pkt;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          pkt_len;
    hal::pd::cpu_to_p4plus_header_t cpu_header;
    hal::pd::p4plus_to_p4_header_t  p4_header;
    hal_ret_t                       ret;

    if (redir_ctx.chain_pkt_sent()) {
        return HAL_RET_OK;
    }

    /*
     * Register current ctx chain queue ID with cpupkt API
     * for sending packet.
     */
    cpu_rxhdr = ctx.cpu_rxhdr();
    ret = hal::pd::cpupkt_register_tx_queue(app_chain_ctx, 
                                            redir_ctx.chain_wring_type(),
                                            cpu_rxhdr->qid);
    if (ret == HAL_RET_OK) {
        memset(&cpu_header, 0, sizeof(cpu_header)); 
        memset(&p4_header, 0, sizeof(p4_header)); 
        cpu_header.flags = redir_ctx.redir_flags();
        cpu_header.src_lif = cpu_rxhdr->src_lif;
        p4_header.flags |= P4PLUS_TO_P4_FLAGS_LKP_INST;

        /*
         * Send packet without app redirect headers
         */
        pkt = redir_ctx.redir_miss_pkt_p()            ?
              (uint8_t *)redir_ctx.redir_miss_pkt_p() : 
              ctx.pkt() + redir_ctx.hdr_len_total();
        pkt_len = ctx.pkt_len() - redir_ctx.hdr_len_total();
        HAL_TRACE_DEBUG("{} pkt_len {} src_lif {}", __FUNCTION__,
                        pkt_len, cpu_header.src_lif);
        ret = hal::pd::cpupkt_send(app_chain_ctx,
                                   redir_ctx.chain_wring_type(),
                                   cpu_rxhdr->qid,
                                   &cpu_header,
                                   &p4_header,
                                   pkt,
                                   pkt_len,
                                   SERVICE_LIF_APP_REDIR,
                                   redir_ctx.chain_qtype(),
                                   cpu_rxhdr->qid,
                                   0);
        redir_ctx.set_chain_pkt_sent(true);
    }

    return ret;
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
        redir_miss_hdr.raw.flow_id = cpu_rxhdr->qid;
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());
        redir_miss_hdr.raw.redir_miss_pkt_p =(uint64_t)redir_ctx.redir_miss_pkt_p();
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        HAL_TRACE_ERR("{} proxy hdr insertion coming soon", __FUNCTION__);
        ret = HAL_RET_ERR;
        goto exit;
        break;

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__, format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        goto exit;
    }

    ctx.set_pkt((uint8_t *)&redir_miss_hdr);
    ctx.set_pkt_len(ctx.pkt_len() + hdr_len + PEN_APP_REDIR_HEADER_SIZE);

exit:    
    return ret;
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

    pkt_len = ctx.pkt_len();
    if (pkt_len < PEN_APP_REDIR_HEADER_MIN_SIZE) {
        HAL_TRACE_ERR("{} pkt_len {} too small", __FUNCTION__, pkt_len);
        ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
        goto exit;
    }

    cpu_rxhdr = ctx.cpu_rxhdr();
    app_hdr = (pen_app_redir_header_v1_full_t *)ctx.pkt();
    h_proto = ntohs(app_hdr->app.h_proto);
    hdr_len = ntohs(app_hdr->ver.hdr_len);
    HAL_TRACE_DEBUG("{} pkt_len {} h_proto {:#x} hdr_len {} format {}",
                    __FUNCTION__, pkt_len, h_proto, hdr_len, app_hdr->ver.format);
    if (h_proto != PEN_APP_REDIR_ETHERTYPE) {
        HAL_TRACE_ERR("{} unexpected h_proto {:#x}", __FUNCTION__, h_proto);
        ret = HAL_RET_APP_REDIR_HDR_ERR;
        goto exit;
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
            goto exit;
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
            goto exit;
        }

        redir_ctx.set_chain_qtype(APP_REDIR_PROXYC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_PROXYC);
        break;

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__,
                      app_hdr->ver.format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        goto exit;
    }

    redir_ctx.set_redir_flags(flags);
    redir_ctx.set_hdr_len_total(hdr_len + PEN_APP_REDIR_HEADER_SIZE);

exit:    
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
            ret = app_redir_pkt_send(ctx);
            app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);
        }
    }

    return ret;
}


static hal_ret_t
app_redir_flow_fwding_update(fte::ctx_t& ctx,
                             flow_key_t &flow_key,
                             proxy_flow_info_t* pfi)
{
    fte::flow_update_t      flowupd = {type: fte::FLOWUPD_FWDING_INFO};

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
    return ctx.update_flow(flowupd);
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
 * App redirect pipeline flow miss exec handler
 */
fte::pipeline_action_t
app_redir_miss_exec(fte::ctx_t& ctx)
{
    proxy_flow_info_t       *pfi;
    hal_ret_t               ret = HAL_RET_OK;
    flow_key_t              flow_key = ctx.key();

    ret = app_redir_init(ctx);
    if (ret != HAL_RET_OK) {
        goto error;
    }

    /*
     * Ignore direction. Always set it to 0
     */
    flow_key.dir = 0;

    /*
     * get the flow info for the Application (L7) Redirect proxy service 
     */
    pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR,
                              &flow_key);
    if (pfi) {

        /*
         * Update flow
         */
        ret = app_redir_flow_fwding_update(ctx, flow_key, pfi);
        if (ret != HAL_RET_OK) {
            goto error;
        }
    } else {
        HAL_TRACE_DEBUG("app_redir is not enabled for the flow");
    }

    /*
     * if entered as a result of an rx packet, insert app redir headers
     * and process the packet
     */
    if (ctx.pkt()) {
        ret = app_redir_miss_hdr_insert(ctx, PEN_RAW_REDIR_V1_FORMAT);
        if (ret == HAL_RET_OK) {
            ret = app_redir_pkt_process(ctx);
        }

        if (ret != HAL_RET_OK) {
            goto error;
        }
    }

exit:
    return app_redir_pipeline_action(ctx);

error:
    ctx.set_feature_status(ret);
    app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);
    goto exit;
}

/*
 * App redirect pipeline flow hit exec handler
 */
fte::pipeline_action_t
app_redir_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret;

    ret = app_redir_init(ctx);
    if (ret != HAL_RET_OK) {
        goto error;
    }

    ret = app_redir_pkt_process(ctx);
    if (ret != HAL_RET_OK) {
        goto error;
    }

exit:
    return app_redir_pipeline_action(ctx);

error:
    ctx.set_feature_status(ret);
    app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);
    goto exit;
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
    hal_ret_t               ret;

    /*
     * Presumably executed as last feature in the pipeline
     */
    app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);

    if (!ctx.drop() && redir_ctx.chain_pkt_verdict_pass()) {
        ret = app_redir_pkt_send(ctx);
        ctx.set_feature_status(ret);
    }

    return app_redir_pipeline_action(ctx);
}

} // namespace hal
} // namespace net
