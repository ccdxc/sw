#include "nic/hal/plugins/network/alg/alg_tftp.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"

#define TFTPOP_OFFSET 0x1

namespace hal {
namespace net {

hal_ret_t
process_tftp_first_packet(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    fte::alg_proto_state_t  state = fte::ALG_PROTO_STATE_NONE;
    const uint8_t          *pkt = ctx.pkt();
    uint8_t                 offset = 0;
    fte::alg_entry_t        newentry = ctx.alg_entry();

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;

    // Opcode offset
    offset += TFTPOP_OFFSET;

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        return ret;
    }

    // Only act on it if there is a known opcode
    if (pkt[offset] == 1) { /* RRQ */
        state = fte::ALG_PROTO_STATE_TFTP_RRQ;
    } else if (pkt[offset] == 2) { /* WRQ */
        state = fte::ALG_PROTO_STATE_TFTP_WRQ;
    } 

    if (state != fte::ALG_PROTO_STATE_NONE) {
        HAL_TRACE_DEBUG("Setting alg proto state to : {}", state);
        // Set Rflow to be invalid and ALG proto state
        // We want the flow miss to happen on Rflow
        ctx.set_valid_rflow(false);
        newentry.key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
        newentry.alg_proto_state = state;
        ctx.set_alg_entry(newentry);
        ctx.register_completion_handler(fte::alg_completion_hdlr);
    }

    return ret;
}

hal_ret_t
process_tftp(fte::ctx_t& ctx)
{
    hal_ret_t             ret = HAL_RET_OK;
    hal::flow_key_t       key = ctx.key();
    const uint8_t        *pkt = ctx.pkt();
    uint8_t               offset = 0;
    fte::alg_entry_t      *entry;

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
 
    // Opcode offset
    offset += TFTPOP_OFFSET;

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        return ret;
    }

    switch (ctx.alg_proto_state())
    {
        case fte::ALG_PROTO_STATE_TFTP_RRQ:
            HAL_TRACE_DEBUG("Received response for RRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (pkt[offset] != 3 && /* DATA */
                pkt[offset] != 6 && /* OACK */
                pkt[offset] != 5) { /* ERROR */
                // Dont do any action based on Opcode. Just log for now
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
            }
            break;

        case fte::ALG_PROTO_STATE_TFTP_WRQ:
            HAL_TRACE_DEBUG("Received response for WRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (pkt[offset] != 4 && /* ACK */
                pkt[offset] != 6 && /* OACK */
                pkt[offset] != 5) { /* ERROR */
                // Dont do any action based on Opcode. Just log for now
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
            }
            break;

        default:
            return ret; 
    }

    // Remove the ALG entry from wildcard table
    // as we have processed the flow already and
    // installed/dropped.
    key.sport = 0;
    entry = (fte::alg_entry_t *)fte::remove_alg_entry(key);
    HAL_FREE(alg_entry_t, entry);

    return ret;
}

}
}
