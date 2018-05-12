//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace proxy {

static inline fte::pipeline_action_t
update_host_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret = HAL_RET_OK;
    HAL_TRACE_DEBUG("IPSec Host flow forwarding role: {} direction: {}", ctx.role(), ctx.direction());
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == FLOW_DIR_FROM_ENIC)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK)) ||   //temporary - only
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_DEBUG("IPSec updating lport = {}", pfi->proxy->meta->lif_info[0].lport_id);

        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 0;
        //flowupd.fwding.qid = pfi->qid1;
        flowupd.fwding.qid = 0;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    } else {
        // Set the lookup vrfid
        //fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_INFO};
        //flowupd.lkp_info.vrf_hwid = hal::pd::pd_vrf_get_lookup_id(ctx.vrf());
        //ret = ctx.update_flow(flowupd);
        //if (ret != HAL_RET_OK) {
        //    ctx.set_feature_status(ret);
        //    return fte::PIPELINE_END; 
        //}
    }

    return  fte::PIPELINE_CONTINUE;
}

static inline fte::pipeline_action_t
update_esp_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    hal_ret_t ret;
    HAL_TRACE_DEBUG("IPSec ESP flow forwarding role: {}  direction: {}", ctx.role(), ctx.direction());
    if (
          ((ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
           (ctx.direction() == FLOW_DIR_FROM_UPLINK)) ||
          ((ctx.role() ==  hal::FLOW_ROLE_RESPONDER) &&
           (ctx.direction() == FLOW_DIR_FROM_ENIC))
        ) {

        fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};
        HAL_TRACE_DEBUG("IPSec updating lport = {}", pfi->proxy->meta->lif_info[0].lport_id);
        // update fwding info
        flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
        flowupd.fwding.qid_en = true;
        flowupd.fwding.qtype = 1;
        //flowupd.fwding.qid = pfi->qid1;
        flowupd.fwding.qid = 0;
        ret = ctx.update_flow(flowupd);
        ctx.set_feature_status(ret);
        return fte::PIPELINE_FINISH;  // Fwding to IPSEC proxy, no other fte featrures needed
    }
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
ipsec_exec(fte::ctx_t& ctx)
{
    proxy_flow_info_t       *pfi = NULL, *tpfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // TODO(goli) - this is to workaround ipsec issue, currently the encrupted pkt
    // is sent in the reverse direction for decryption without swapping the sip to dip.
    // For this pkt lport shouldn't be the ipsec proxy-lif.
    if (ctx.existing_session()) {
        return fte::PIPELINE_CONTINUE;
    }

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service
    pfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                              &flow_key);

    if(pfi) {
        if(flow_key.proto == IPPROTO_ESP) {
            HAL_TRACE_DEBUG("ipsec ESP flow");
            return update_esp_flow_fwding_info(ctx, pfi);
        } else {
            HAL_TRACE_DEBUG("ipsec HOST flow");
            // Get Tunnel PFI
            tpfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                                       &(pfi->u.ipsec.u.host_flow.esp_flow_key));
            if(!tpfi) {
                HAL_TRACE_ERR("Failed to get tunnel info for host flow: {}", flow_key);
                //ctx.set_feature_status(HAL_RET_FLOW_NOT_FOUND);
                //return fte::PIPELINE_END;
            }
            // Update flow
            return update_host_flow_fwding_info(ctx, pfi);
        }
    } else {
        HAL_TRACE_DEBUG("ipsec is not enabled for the flow");
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
