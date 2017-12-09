#include "nic/hal/plugins/network/alg/alg_tftp.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"

namespace hal {
namespace net {

static void
incr_parse_error(session_t *sess)
{
    HAL_ATOMIC_INC_UINT32(&sess->app_session->alg_info.parse_errors, 1);
}

static void
incr_unknown_opcode(session_t *sess)
{
    HAL_ATOMIC_INC_UINT32(&sess->app_session->alg_info.unknown_opcode, 1);
}

hal_ret_t
process_tftp_first_packet(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    fte::alg_proto_state_t  state = fte::ALG_PROTO_STATE_NONE;
    const uint8_t          *pkt = ctx.pkt();
    uint32_t                offset = 0;
    uint16_t                tftpop = 0;
    fte::alg_entry_t         *alg_entry = NULL;

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;

    // Fetch 2-byte opcode
    tftpop = __pack_uint16(pkt, &offset);

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        return ret;
    }

    // Only act on it if there is a known opcode
    if (tftpop == 1) { /* RRQ */
        state = fte::ALG_PROTO_STATE_TFTP_RRQ;
    } else if (tftpop == 2) { /* WRQ */
        state = fte::ALG_PROTO_STATE_TFTP_WRQ;
    } else {
        HAL_TRACE_DEBUG("Unknown Opcode -- parse error");
        return ret;
    } 

    if (state != fte::ALG_PROTO_STATE_NONE) {
        HAL_TRACE_DEBUG("Setting alg proto state to : {}", state);
        // Set Rflow to be invalid and ALG proto state
        // We want the flow miss to happen on Rflow
        ctx.set_valid_rflow(false);
        alg_entry->skip_sfw = TRUE;
        alg_entry->entry.key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
        alg_entry->entry.key.sport = 0;
        alg_entry->alg_proto_state = state;
        HAL_TRACE_DEBUG("Alg Entry key: {}", alg_entry->entry.key);
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
    uint32_t              offset = 0;
    uint16_t              tftpop = 0;
    fte::alg_entry_t     *alg_entry = NULL;
    hal::session_t       *session = NULL;

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (alg_entry == NULL) {
        HAL_TRACE_ERR("ALG entry is not found in the context -- bailing");
        return HAL_RET_ERR;
    }

    session = alg_entry->session;
    if (session == NULL) {
        HAL_TRACE_ERR("Session is null for existing session -- bailing");
        return HAL_RET_ERR;
    }

    // Payload offset from CPU header
    offset = ctx.cpu_rxhdr()->payload_offset;
 
    // Fetch 2-byte opcode
    tftpop = __pack_uint16(pkt, &offset);

    if (ctx.pkt_len() < offset) {
        // Should we drop the packet at this point ?
        HAL_TRACE_ERR("Packet len: {} is less than payload offset: {}", \
                      ctx.pkt_len(),  offset);
        incr_parse_error(session);
        return ret;
    }

    switch (alg_entry->alg_proto_state)
    {
        case fte::ALG_PROTO_STATE_TFTP_RRQ:
            HAL_TRACE_DEBUG("Received response for RRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (tftpop != 3 && /* DATA */
                tftpop != 6 && /* OACK */
                tftpop != 5) { /* ERROR */
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
                incr_unknown_opcode(session);
            }
            break;

        case fte::ALG_PROTO_STATE_TFTP_WRQ:
            HAL_TRACE_DEBUG("Received response for WRQ offset: {} opcode: {}",
                            offset, pkt[offset+1]);
            if (tftpop != 4 && /* ACK */
                tftpop != 6 && /* OACK */
                tftpop != 5) { /* ERROR */
                HAL_TRACE_DEBUG("TFTP Unknown Opcode response received");
                incr_unknown_opcode(session);
            }
            break;

        default:
            return ret; 
    }

    // Remove the ALG entry from wildcard table
    // as we have processed the flow already and
    // installed/dropped.
    key.sport = 0;
    alg_entry = (fte::alg_entry_t *)fte::remove_expected_flow(key);
    HAL_FREE(hal::HAL_MEM_ALLOC_ALG, alg_entry);

    return ret;
}

}
}
