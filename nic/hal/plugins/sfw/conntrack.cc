//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "lib/periodic/periodic.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/iris/include/hal_state.hpp"

namespace session {
std::ostream& operator<<(std::ostream& os, const session::FlowTCPState& val)
{
    switch (val) {
        case session::FLOW_TCP_STATE_INIT: return os << "INIT";
        case session::FLOW_TCP_STATE_SYN_RCVD: return os << "SYN RCVD";
        case session::FLOW_TCP_STATE_ACK_RCVD: return os << "ACK RCVD";
        case session::FLOW_TCP_STATE_SYN_ACK_RCVD: return os << "SYN ACK RCVD";
        case session::FLOW_TCP_STATE_ESTABLISHED: return os << "ESTABLISHED";
        case session::FLOW_TCP_STATE_FIN_RCVD: return os << "FIN RCVD";
        case session::FLOW_TCP_STATE_BIDIR_FIN_RCVD: return os << "BIDIR FIN RCVD";
        case session::FLOW_TCP_STATE_RESET: return os << "RST RCVD";
        default: return os;
    }
}
}

namespace hal {
namespace plugins {
namespace sfw {

#define DEFAULT_MSS 546 // RFC6691
#define DEFAULT_WINDOW_SIZE 512
//------------------------------------------------------------------------------
// extract all the TCP related state from session spec
//------------------------------------------------------------------------------
static hal_ret_t
net_conntrack_extract_session_state_from_spec (fte::flow_state_t *flow_state,
                                 const session::FlowData& flow_data)
{
    auto conn_track_info = flow_data.conn_track_info();
    flow_state->state = flow_data.flow_info().tcp_state();
    flow_state->tcp_seq_num = conn_track_info.tcp_seq_num();
    flow_state->tcp_ack_num = conn_track_info.tcp_ack_num();
    flow_state->tcp_win_sz = conn_track_info.tcp_win_sz();
    flow_state->tcp_win_scale = conn_track_info.tcp_win_scale();
    flow_state->tcp_mss = conn_track_info.tcp_mss();
    flow_state->create_ts = conn_track_info.flow_create_ts();
    flow_state->last_pkt_ts = flow_state->create_ts;
    flow_state->packets = conn_track_info.flow_packets();
    flow_state->bytes = conn_track_info.flow_bytes();
    if (conn_track_info.exceptions().tcp_syn_retransmit()) {
        flow_state->exception_bmap |= TCP_SYN_REXMIT;
    }
    if (conn_track_info.exceptions().tcp_win_zero_drop()) {
        flow_state->exception_bmap |= TCP_WIN_ZERO_DROP;
    }
    if (conn_track_info.exceptions().tcp_full_retransmit()) {
        flow_state->exception_bmap |= TCP_FULL_REXMIT;
    }
    if (conn_track_info.exceptions().tcp_partial_overlap()) {
        flow_state->exception_bmap |= TCP_PARTIAL_OVERLAP;
    }
    if (conn_track_info.exceptions().tcp_packet_reorder()) {
        flow_state->exception_bmap |= TCP_PACKET_REORDER;
    }
    if (conn_track_info.exceptions().tcp_out_of_window()) {
        flow_state->exception_bmap |= TCP_OUT_OF_WINDOW;
    }
    if (conn_track_info.exceptions().tcp_invalid_ack_num()) {
        flow_state->exception_bmap |= TCP_ACK_ERR;
    }
    if (conn_track_info.exceptions().tcp_normalization_drop()) {
        flow_state->exception_bmap |= TCP_NORMALIZATION_DROP;
    }
    if (conn_track_info.exceptions().tcp_split_handshake_detected()) {
        flow_state->exception_bmap |= TCP_SPLIT_HANDSHAKE_DETECTED;
    }
    if (conn_track_info.exceptions().tcp_data_after_fin()) {
        flow_state->exception_bmap |= TCP_DATA_AFTER_FIN;
    }
    if (conn_track_info.exceptions().tcp_non_rst_pkt_after_rst()) {
        flow_state->exception_bmap |= TCP_NON_RST_PKT_AFTER_RST;
    }
    if (conn_track_info.exceptions().tcp_invalid_first_pkt_from_responder()) {
        flow_state->exception_bmap |= TCP_INVALID_RESPONDER_FIRST_PKT;
    }
    if (conn_track_info.exceptions().tcp_unexpected_pkt()) {
        flow_state->exception_bmap |= TCP_UNEXPECTED_PKT;
    }
    if (conn_track_info.exceptions().tcp_rst_with_invalid_ack_num()) {
        flow_state->exception_bmap |= TCP_RST_WITH_INVALID_ACK_NUM;
    }
    return HAL_RET_OK;
}


static inline bool
net_conntrack_configured(fte::ctx_t &ctx)
{

    if (ctx.protobuf_request()) {
        return ctx.sess_spec()->conn_track_en();
    }

    if (ctx.key().proto != types::IPPROTO_TCP || 
        (hal::g_hal_state->is_policy_enforced() == false)) {
        return false;
    }

    // lookup Security profile
    if (ctx.svrf() && ctx.svrf()->nwsec_profile_handle  != HAL_HANDLE_INVALID) {
        hal::nwsec_profile_t  *nwsec_prof =
            find_nwsec_profile_by_handle(ctx.svrf()->nwsec_profile_handle);
        if (nwsec_prof != NULL) {
            return nwsec_prof->cnxn_tracking_en;
        }
    }
    return false;
}

/*
 * API to process TCP FIN/RST on IFlow/RFlow
 *
 * When the first FIN is seen on the connection, start the TCP half closed
 * timeout to wait for the ACK & FIN from the other side.
 * When the FIN seen is a second FIN on the connection or a TCP RST is
 * received on the connection then start the TCP close timer to cleanup
 * the session. Timer is started to ensure the other end receives and ACKs
 */
static void
process_tcp_close(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t  *cpu_rxhdr = ctx.cpu_rxhdr();
     int8_t                  tcp_flags;
    session::FlowTCPState    state;
    hal::flow_role_t         role = (ctx.is_flow_swapped())?\
                         hal::FLOW_ROLE_RESPONDER:hal::FLOW_ROLE_INITIATOR;

    tcp_flags = cpu_rxhdr->tcp_flags;
    if (tcp_flags & TCP_FLAG_FIN) {
        state = session::FLOW_TCP_STATE_FIN_RCVD;
    } else if (tcp_flags & TCP_FLAG_RST) {
        state = session::FLOW_TCP_STATE_RESET;
    }

    HAL_TRACE_VERBOSE("Received {} for session with key: {}", state, ctx.key());
    // Start a timer to cleanup session if this is the first FIN/RST received
    // Timer is started in order to wait for either side to close the TCP
    // connection
    if (ctx.session()->iflow->state <= session::FLOW_TCP_STATE_ESTABLISHED &&
         (!ctx.session()->rflow || (ctx.session()->rflow &&
          ctx.session()->rflow->state <= session::FLOW_TCP_STATE_ESTABLISHED))) {
        if (tcp_flags & TCP_FLAG_FIN) {
            state = session::FLOW_TCP_STATE_FIN_RCVD;
            hal::schedule_tcp_half_closed_timer(ctx.session());
            goto done;
        }
    } else if ((state == session::FLOW_TCP_STATE_FIN_RCVD) && 
               ((role == hal::FLOW_ROLE_RESPONDER && 
                 ctx.session()->iflow->state == session::FLOW_TCP_STATE_FIN_RCVD) ||
		(role == hal::FLOW_ROLE_INITIATOR &&
                 ctx.session()->rflow->state == session::FLOW_TCP_STATE_FIN_RCVD))) {
            state = session::FLOW_TCP_STATE_BIDIR_FIN_RCVD;
            if (role == hal::FLOW_ROLE_INITIATOR) 
                session_set_tcp_state(ctx.session(), hal::FLOW_ROLE_RESPONDER, state);
            else
                session_set_tcp_state(ctx.session(), hal::FLOW_ROLE_INITIATOR, state);
    } else {
        return;
    }

    hal::schedule_tcp_close_timer(ctx.session());

done:
    /*
     * We run the TCP close pipeline only for IFLOW so we ALWAYS
     * update only iflow state. It is okay for TCP close processing
     */
    session_set_tcp_state(ctx.session(), role, state);
}

/*
 * API to start the connection setup timer.
 *
 * When the first TCP SYN is seen on the packet, TCP connection
 * setup timer is started to ensure that we have the connection setup
 * within a given timeout.
 */
static void
start_tcp_cxnsetup_timer (fte::ctx_t& ctx, bool status) {
    if (status && ctx.session() && !ctx.drop()) {
        hal::schedule_tcp_cxnsetup_timer(ctx.session());
    }
}

/*
 *   API to start the process TCP SYN
 *
 *   When the first TCP SYN is seen on the connection, register
 *   a completion handler to start a connection setup timer on
 *   the session. If the session is not created due to some error
 *   in the pipeline, then this timer will not be started.
 */
bool
process_tcp_syn(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t  *cpu_rxhdr = ctx.cpu_rxhdr();
    uint8_t                  tcp_flags;

    if (cpu_rxhdr == NULL) {
        return true;
    }

    tcp_flags = cpu_rxhdr->tcp_flags;

    HAL_TRACE_DEBUG("Processing TCP SYN Flags:{} Existing:{}", tcp_flags, ctx.existing_session());

    if (!ctx.existing_session()) {
        if (tcp_flags & TCP_FLAG_SYN) {
            // Start TCP connection setup timer
            ctx.register_completion_handler(start_tcp_cxnsetup_timer);
        } else {
            // Non Syn packet is received, and session is a new one
            // Assumption is - In general, TCP Non Syn packet will be dropped in the P4
            // pipeline itself during normalization logic.
            // But in case of vMotion, for migration scenarios from a non-pensando device to
            // a pensando device, reception of non-syn packet is a valid one. Those flows will
            // be crated with 'Connection Tracking' disabled. Return false here, to continue
            // the pipeline instead of enabling connection tracking. 
            return false;
        }
    }
    return true;
}

fte::pipeline_action_t
conntrack_exec(fte::ctx_t& ctx)
{
    hal_ret_t       ret;

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FLOW_STATE};

    if (!net_conntrack_configured(ctx)) {
        return fte::PIPELINE_CONTINUE;
    }

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        if (ctx.tcp_close() && ctx.existing_session()) {
            process_tcp_close(ctx);
            return fte::PIPELINE_CONTINUE;
        }

        if (ctx.flow_miss() && !ctx.drop()) {
            if (!process_tcp_syn(ctx)) {
                return fte::PIPELINE_CONTINUE;
            }
        }

        if (ctx.protobuf_request()) {
            net_conntrack_extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->initiator_flow().flow_data());

            if (ctx.sess_spec()->initiator_flow().has_flow_data() &&
                ctx.sess_spec()->initiator_flow().flow_data().has_conn_track_info()) {
                flowupd.flow_state.syn_ack_delta = 
                            ctx.sess_spec()->initiator_flow().flow_data().conn_track_info().\
                                  iflow_syn_ack_delta();
            }
        } else {
            const fte::cpu_rxhdr_t *rxhdr = ctx.cpu_rxhdr();
            flowupd.flow_state.state = session::FLOW_TCP_STATE_INIT;
            // Expectation is to program the next expected seq num.
            flowupd.flow_state.tcp_seq_num = ntohl(rxhdr->tcp_seq_num) + 1;
            flowupd.flow_state.tcp_ack_num = ntohl(rxhdr->tcp_ack_num);
            flowupd.flow_state.tcp_win_sz = ntohs(rxhdr->tcp_window);

            //set defaults
            flowupd.flow_state.tcp_mss = DEFAULT_MSS;
            flowupd.flow_state.tcp_ws_option_sent = 0;
            flowupd.flow_state.tcp_ts_option_sent = 0;
            flowupd.flow_state.tcp_sack_perm_option_sent = 0;

            // TCP Options
            if (rxhdr->flags & CPU_FLAGS_TCP_OPTIONS_PRESENT) {
                // MSS Option
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_MSS) {
                    flowupd.flow_state.tcp_mss = ntohs(rxhdr->tcp_mss);
                }

                // Window Scale option
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_WINDOW_SCALE) {
                    flowupd.flow_state.tcp_ws_option_sent = 1;
                    flowupd.flow_state.tcp_win_scale = rxhdr->tcp_ws;
                }

                // timestamp option
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_TIMESTAMP) {
                    flowupd.flow_state.tcp_ts_option_sent = 1;
                }
                // sack_permitted option
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_SACK_PERMITTED) {
                    flowupd.flow_state.tcp_sack_perm_option_sent = 1;
                }
            }
        }
    } else { /* rflow */
        if (ctx.protobuf_request()) {
            net_conntrack_extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->responder_flow().flow_data());
        } else {
            flowupd.flow_state.state = session::FLOW_TCP_STATE_INIT;
            flowupd.flow_state.tcp_mss = DEFAULT_MSS;
            flowupd.flow_state.tcp_win_sz = DEFAULT_WINDOW_SIZE; //This is to allow the SYN Packet to go through.
        }
    }

    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
