#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace net {

static inline hal_ret_t
update_alg_flow(fte::ctx_t& ctx, fte::flow_update_type_t type, 
            session::FlowAction action, hal::flow_key_t key = {}) {
    fte::flow_update_t flowupd;
    hal_ret_t          ret = HAL_RET_OK;

    flowupd.type = type;
    flowupd.action = action;
    if (type == fte::FLOWUPD_ACTION) {
        // Only Drop action needs update
        ret = ctx.update_flow(flowupd);
    } else if (type == fte::FLOWUPD_KEY) {
        flowupd.key = key;
        ret = ctx.update_flow(flowupd, hal::FLOW_ROLE_RESPONDER); 
    }

    return ret;
}

static inline hal_ret_t 
process_tftp_first_packet(fte::ctx_t& ctx)
{
    hal_ret_t    ret = HAL_RET_OK;
    hal::alg_proto_state_t state = hal::ALG_PROTO_STATE_NONE;
    const uint8_t *pkt = ctx.pkt();
    uint8_t offset = 0; 
    const hal::flow_key_t key_ = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
    hal::flow_key_t  key = {};

    // Payload offset from CPU header 
    offset = ctx.cpu_rxhdr()->payload_offset;

    if (pkt[offset++] != 0) { /* First byte */
        ret = update_alg_flow(ctx, fte::FLOWUPD_ACTION, 
                 session::FLOW_ACTION_DROP); 
    } else if (pkt[offset] == 1) { /* RRQ */ 
        state = hal::ALG_PROTO_STATE_TFTP_RRQ;
    } else if (pkt[offset] == 2) { /* WRQ */
        state = hal::ALG_PROTO_STATE_TFTP_WRQ;
    } else {
        ret = update_alg_flow(ctx, fte::FLOWUPD_ACTION, 
                              session::FLOW_ACTION_DROP, key);
    }

    if (ret != HAL_RET_OK) {
        return ret;
    }
 
    // Install a software rflow and do not update anything yet
    // Update the reverse flow key to have sport as wildcard 
    memcpy(&key,  &key_, sizeof(hal::flow_key_t));
    key.sport = 0;
    ret = update_alg_flow(ctx, fte::FLOWUPD_KEY, 
                          session::FLOW_ACTION_ALLOW, key);
    ctx.set_pgm_rflow(false); 
    ctx.set_alg_proto_state(state);

    return ret;
}

static inline hal_ret_t
process_tftp(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    hal::flow_key_t       key;
    const hal::flow_key_t key_ = ctx.key();
    session::FlowAction   action = session::FLOW_ACTION_ALLOW;
    const uint8_t        *pkt = ctx.pkt();
    uint8_t               offset = 0;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
    
    switch (ctx.session()->alg_proto_state)
    {
        case ALG_PROTO_STATE_TFTP_RRQ:
            HAL_TRACE_DEBUG("Received response for RRQ offset: {} opcode", 
                            offset, pkt[offset+1]); 
            if (pkt[offset++] != 0 ||
                (pkt[offset] != 3 && /* DATA */
                pkt[offset] != 6)) { /* OACK */
                action = session::FLOW_ACTION_DROP;
            } else {
                ctx.set_pgm_rflow(true);
            }
            break;

        case ALG_PROTO_STATE_TFTP_WRQ:
            if (pkt[offset++] != 0 ||
                (pkt[offset] != 4 && /* ACK */
                pkt[offset] != 6)) { /* OACK */
                action = session::FLOW_ACTION_DROP;
            } else {
                ctx.set_pgm_rflow(true);
            } 
            break;
 
        default:
            action = session::FLOW_ACTION_DROP;
            break;
    }

    if (action == session::FLOW_ACTION_DROP) {
        ret = update_alg_flow(ctx, fte::FLOWUPD_ACTION,
                 session::FLOW_ACTION_DROP);
        ctx.set_hal_cleanup(true);
    } else {
        // Install a software rflow and do not update anything yet
        // Update the reverse flow key to have sport as wildcard
        memcpy(&key,  &key_, sizeof(hal::flow_key_t));
        ret = update_alg_flow(ctx, fte::FLOWUPD_KEY,
                          session::FLOW_ACTION_ALLOW, key);
        ctx.set_pgm_rflow(true);
    }

    return ret;
}

static bool
IsAlgResponderFlow(fte::ctx_t& ctx)
{
    session_t  *sess = ctx.session();

    if (!sess) 
        return false;

    HAL_TRACE_DEBUG("ALG Exec role: {} alg_proto_state: {}", 
                     ctx.role(), sess->alg_proto_state);
    return ((ctx.role() == hal::FLOW_ROLE_RESPONDER) && \
            (sess->alg_proto_state != ALG_PROTO_STATE_NONE));
}


fte::pipeline_action_t
alg_exec(fte::ctx_t& ctx)
{
    hal_ret_t       ret = HAL_RET_OK;
  
    if (!ctx.protobuf_request() && 
        ((ctx.alg_proto() != nwsec::APP_SVC_NONE) ||
        (IsAlgResponderFlow(ctx)))) {
         if (ctx.role() == hal::FLOW_ROLE_RESPONDER) {
             switch(ctx.session()->alg_proto_state) {
             case ALG_PROTO_STATE_TFTP_RRQ:
             case ALG_PROTO_STATE_TFTP_WRQ:
                 ret = process_tftp(ctx);
                 if (ret != HAL_RET_OK) {
                    ctx.set_feature_status(ret);
                    return fte::PIPELINE_END;
                 }
                 break;
             default:
                 break;
             }
         } else { 
             switch(ctx.alg_proto()) {
             case nwsec::APP_SVC_TFTP:
                ret = process_tftp_first_packet(ctx);
                if (ret != HAL_RET_OK) {
                    ctx.set_feature_status(ret);
                    return fte::PIPELINE_END;
                }
                break;

            case nwsec::APP_SVC_FTP:
                break;

            case nwsec::APP_SVC_DNS:
                break;

            default:
                break;
            }
        }
    }
    
    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
