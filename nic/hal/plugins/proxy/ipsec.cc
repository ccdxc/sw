#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

namespace hal {
namespace proxy {

static inline hal_ret_t
update_host_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
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
        return ctx.update_flow(flowupd);
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
update_esp_flow_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
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
        return ctx.update_flow(flowupd);
    }
    return HAL_RET_OK;
}

fte::pipeline_action_t
ipsec_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t       *pfi = NULL, *tpfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service 
    pfi = proxy_get_flow_info(types::PROXY_TYPE_IPSEC,
                              &flow_key);
    
    if(pfi) {
        if(flow_key.proto == IPPROTO_ESP) {
            HAL_TRACE_DEBUG("ipsec ESP flow");
            ret = update_esp_flow_fwding_info(ctx, pfi);
            // Update flow
            if (ret != HAL_RET_OK) {
                ctx.set_feature_status(ret);
                return fte::PIPELINE_END; 
            }
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
            ret = update_host_flow_fwding_info(ctx, pfi);
            if (ret != HAL_RET_OK) {
                ctx.set_feature_status(ret);
                return fte::PIPELINE_END; 
            }
        }
    } else {
        HAL_TRACE_DEBUG("ipsec is not enabled for the flow");
    } 
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
