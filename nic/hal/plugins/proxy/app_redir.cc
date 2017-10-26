#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "app_redir.hpp"

namespace hal {
namespace proxy {

hal::pd::cpupkt_ctxt_t* app_chain_ctx = NULL;

static inline hal_ret_t
app_redir_init(fte::ctx_t& ctx)
{
    const fte::app_redir_ctx_t& redir_ctx = ctx.app_redir();
    hal_ret_t                   ret = HAL_RET_OK;

    redir_ctx.init();
    if (app_chain_ctx == NULL)   {
        app_chain_ctx = hal::pd::cpupkt_ctxt_alloc_init();
        if (app_chain_ctx == NULL)   {
            ret = HAL_RET_NO_RESOURCE;
        }
    }

    return ret;
}

/*
 * Exported application packet send function
 */
hal_ret_t
app_redir_pkt_send(fte::ctx_t& ctx)
{
    const fte::app_redir_ctx_t&     redir_ctx = ctx.app_redir();
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
        pkt = ctx.pkt() + redir_ctx.hdr_len_total();
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
    }

    return ret;
}

/*
 * Validate Pensando app header in redirected packet.
 */
static inline hal_ret_t
app_redir_app_hdr_validate(fte::ctx_t& ctx)
{
    const fte::app_redir_ctx_t&         redir_ctx = ctx.app_redir();
    pen_app_redir_header_v1_full_t      *app_hdr;
    const fte::cpu_rxhdr_t              *cpu_rxhdr;
    size_t                              pkt_len;
    size_t                              hdr_len;
    uint16_t                            h_proto;
    uint16_t                            flags = 0;
    hal_ret_t                           ret = HAL_RET_OK;

    pkt_len = ctx.pkt_len();
    if (pkt_len < (PEN_APP_REDIR_HEADER_SIZE +
                   PEN_APP_REDIR_VERSION_HEADER_SIZE)) {
        HAL_TRACE_ERR("{} pkt_len {} too small", __FUNCTION__, pkt_len);
        ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
        goto done;
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
        if (hdr_len != (PEN_APP_REDIR_VERSION_HEADER_SIZE +
                        PEN_RAW_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid hdr_len {}", __FUNCTION__,
                          app_hdr->ver.format, hdr_len);
            ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
            goto done;
        }

        redir_ctx.set_chain_qtype(APP_REDIR_RAWC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_RAWC);
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        redir_ctx.flags = ntohs(app_hdr->proxy.flags);
        HAL_TRACE_DEBUG("{} flow_id {:#x} flags {:#x} vrf {}", __FUNCTION__,
                        ntohl(app_hdr->proxy.flow_id), redir_ctx.flags,
                        app_hdr->proxy.vrf);
        if (hdr_len != (PEN_APP_REDIR_VERSION_HEADER_SIZE +
                        PEN_PROXY_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid hdr_len {}", __FUNCTION__,
                          app_hdr->ver.format, hdr_len);
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
    return ret;
}

/*
 * Forward redirected packet to application or simply loop it back (DOL case).
 */
static inline hal_ret_t
app_redir_pkt_fwd(fte::ctx_t& ctx)
{
    const fte::app_redir_ctx_t& redir_ctx = ctx.app_redir();
    hal_ret_t                   ret;

    ret = app_redir_app_hdr_validate(ctx);
    if (ret == HAL_RET_OK) {

        if (redir_ctx.redir_flags() & PEN_APP_REDIR_PIPELINE_LOOPBK_EN) {
            ret = app_redir_pkt_send(ctx);

            /*
             * DOL has sent the packet so don't let the remainder
             * of the pipeline send it again.
             */
            redir_ctx.set_chain_pkt_sent(TRUE);
        }
    }

    return ret;
}


static inline hal_ret_t
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


fte::pipeline_action_t
app_redir_miss_exec(fte::ctx_t& ctx)
{
    proxy_flow_info_t           *pfi;
    hal_ret_t                   ret = HAL_RET_OK;
    flow_key_t                  flow_key = ctx.key();

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

    return fte::PIPELINE_CONTINUE;

error:
    ctx.set_feature_status(ret);
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
app_redir_exec(fte::ctx_t& ctx)
{
    hal_ret_t           ret;

    ret = app_redir_init(ctx);
    if (ret != HAL_RET_OK) {
        goto error;
    }

    ret = app_redir_pkt_fwd(ctx);
    if (ret != HAL_RET_OK) {
        goto error;
    }
    return fte::PIPELINE_CONTINUE;

error:
    ctx.set_feature_status(ret);
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
