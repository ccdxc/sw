#include "nic/hal/plugins/network/alg/alg_tftp.hpp"

namespace hal {
namespace net {

hal_ret_t
process_tftp_first_packet(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    fte::alg_proto_state_t  state = fte::ALG_PROTO_STATE_NONE;
    const uint8_t          *pkt = ctx.pkt();
    uint8_t                 offset = 0;
    fte::flow_update_t      flowupd;
    session::FlowAction     action;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        return ret;
    }

    if (pkt[offset++] != 0) { /* First byte */
        action = session::FLOW_ACTION_DROP;
    } else if (pkt[offset] == 1) { /* RRQ */
        state = fte::ALG_PROTO_STATE_TFTP_RRQ;
    } else if (pkt[offset] == 2) { /* WRQ */
        state = fte::ALG_PROTO_STATE_TFTP_WRQ;
    } else {
        action = session::FLOW_ACTION_DROP;
    }

    if (action == session::FLOW_ACTION_DROP) {
        flowupd.type = fte::FLOWUPD_ACTION;
        flowupd.action = session::FLOW_ACTION_DROP;
        ret = ctx.update_flow(flowupd);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // Set Rflow to be invalid and ALG proto state
    // We want the flow miss to happen on Rflow
    ctx.set_valid_rflow(false);
    ctx.set_alg_proto_state(state);

    return ret;
}

hal_ret_t
process_tftp(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    hal::flow_key_t       key = ctx.key();
    session::FlowAction   action = session::FLOW_ACTION_ALLOW;
    const uint8_t        *pkt = ctx.pkt();
    uint8_t               offset = 0;
    fte::flow_update_t    flowupd;
    fte::alg_entry_t      *entry;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_TFTP_RRQ:
            HAL_TRACE_DEBUG("Received response for RRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (pkt[offset++] != 0 ||
                (pkt[offset] != 3 && /* DATA */
                pkt[offset] != 6)) { /* OACK */
                action = session::FLOW_ACTION_DROP;
            }
            break;

        case fte::ALG_PROTO_STATE_TFTP_WRQ:
            HAL_TRACE_DEBUG("Received response for WRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (pkt[offset++] != 0 ||
                (pkt[offset] != 4 && /* ACK */
                pkt[offset] != 6)) { /* OACK */
                action = session::FLOW_ACTION_DROP;
            }
            break;

        default:
            action = session::FLOW_ACTION_DROP;
            break;
    }

    if (action == session::FLOW_ACTION_DROP) {
        flowupd.action = action;
        flowupd.type = fte::FLOWUPD_ACTION;
        ret = ctx.update_flow(flowupd);
        ctx.set_hal_cleanup(true);
    }

    // Remove the ALG entry from wildcard table
    // as we have processed the flow already and
    // installed/dropped.
    key.sport = 0;
    entry = fte::remove_alg_entry(key);
    HAL_FREE(alg_entry_t, entry);

    return ret;
}

}
}
