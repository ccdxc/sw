#include "nic/hal/plugins/network/alg/alg_rpc.hpp"

namespace hal {
namespace net {

hal_ret_t
parse_rpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    //const uint8_t        *pkt = ctx.pkt();
    //session::FlowAction   action = session::FLOW_ACTION_ALLOW;
    uint8_t               rpc_msg_offset = 0;
    //fte::alg_entry_t     *entry;

    // Payload offset from CPU header
    rpc_msg_offset = ctx.cpu_rxhdr()->payload_offset;
    if (ctx.pkt_len() < rpc_msg_offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  rpc_msg_offset);
        return HAL_RET_ERR;
    }

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_RPC_INIT:
            break;

        case fte::ALG_PROTO_STATE_RPC_GETPORT:
            break;

        case fte::ALG_PROTO_STATE_RPC_CALLIT:
            return HAL_RET_ERR;

        default:
            break;
    }
 
    return ret;
}

hal_ret_t
process_rpc_control_flow(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    fte::flow_update_t    flowupd;

    if (ctx.alg_proto() == nwsec::APP_SVC_SUN_RPC) {
        // UDP could have the portmapper queries at the 
        // start of the session
        if (ctx.key().proto == IP_PROTO_UDP) {
            ctx.set_alg_proto_state(fte::ALG_PROTO_STATE_RPC_INIT);
            parse_rpc_control_flow(ctx); 
        }
        flowupd.type = fte::FLOWUPD_MCAST_COPY;
        flowupd.mcast_copy_en = 1;
        ret = ctx.update_flow(flowupd);
        if (ret == HAL_RET_OK) {
            // Update Responder flow also to do a Mcast copy (or redirect)
            ret = ctx.update_flow(flowupd, hal::FLOW_ROLE_RESPONDER);
        }
    }

    return ret;
}

}
}
