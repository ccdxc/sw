//{C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/fte/fte.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/p4/iris/include/defines.h"

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
    flow_state->exception_bmap = conn_track_info.exception_bits();

    return HAL_RET_OK;
}


static inline bool
net_conntrack_configured(fte::ctx_t &ctx)
{

    if (ctx.protobuf_request()) {
        return ctx.sess_spec()->conn_track_en();
    }

    if (ctx.key().proto != types::IPPROTO_TCP) {
        return false;
    }

    // lookup Security profile
    if (ctx.vrf()->nwsec_profile_handle  != HAL_HANDLE_INVALID) {
        hal::nwsec_profile_t  *nwsec_prof =
            find_nwsec_profile_by_handle(ctx.vrf()->nwsec_profile_handle);
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
    uint8_t                  tcp_flags;
    session::FlowTCPState    state;

    tcp_flags = cpu_rxhdr->tcp_flags;
    if (tcp_flags & TCP_FLAG_FIN) {
        state = session::FLOW_TCP_STATE_FIN_RCVD;
    } else if (tcp_flags & TCP_FLAG_RST) {
        state = session::FLOW_TCP_STATE_RESET;
    }
     
    HAL_TRACE_DEBUG("Received {} for session with key: {}", state, ctx.key());
    // Start a timer to cleanup session if this is the first FIN/RST received
    // Timer is started in order to wait for either side to close the TCP 
    // connection
    if ((ctx.session()->iflow->state <= session::FLOW_TCP_STATE_ESTABLISHED &&
         (!ctx.session()->rflow || (ctx.session()->rflow && 
          ctx.session()->rflow->state <= session::FLOW_TCP_STATE_ESTABLISHED)))) {
        if (tcp_flags & TCP_FLAG_FIN) {
            state = session::FLOW_TCP_STATE_FIN_RCVD;
            hal::schedule_tcp_half_closed_timer(ctx.session());
            goto done;
        }
    } else {
        if ((ctx.session()->iflow->state >= session::FLOW_TCP_STATE_BIDIR_FIN_RCVD ||
             (ctx.session()->rflow && 
              ctx.session()->rflow->state >= session::FLOW_TCP_STATE_BIDIR_FIN_RCVD)) ||
             ctx.session()->tcp_cxntrack_timer == NULL) {
            /*
             * We are here because we already have seen one of FIN/RST or both of them 
             * (a duplicate/retry ?) and in case of FIN the timer is NULL which means 
             * the close timer is already started so update the state and move on
             */
             goto done;
        } else {
             /*
              * If we are here then we have one FIN RCVD and we started a timer and the 
              * timer is still pending and we received a FIN/RST now. So, we can go 
              * ahead and delete the half closed timer and start the TCP close timer
              */
             hal::periodic::timer_delete(ctx.session()->tcp_cxntrack_timer);
             state = session::FLOW_TCP_STATE_BIDIR_FIN_RCVD;
        } 
    }
    hal::schedule_tcp_close_timer(ctx.session());

done:
    /*
     * We run the TCP close pipeline only for IFLOW so we ALWAYS
     * update only iflow state. It is okay for TCP close processing
     */
    session_set_tcp_state(ctx.session(), ctx.role(), state);
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
    if (status && ctx.session()) {
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
static void
process_tcp_syn(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t  *cpu_rxhdr = ctx.cpu_rxhdr();
    uint8_t                  tcp_flags;

    HAL_TRACE_DEBUG("Processing TCP SYN");
    if (cpu_rxhdr == NULL) {
        return;
    }

    tcp_flags = cpu_rxhdr->tcp_flags;
    if (tcp_flags & TCP_FLAG_SYN) {
        // Start TCP connection setup timer
        // if we have a session
        ctx.register_completion_handler(start_tcp_cxnsetup_timer);
    }
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

        if (ctx.flow_miss()) {
            process_tcp_syn(ctx);
        }
        if (ctx.protobuf_request()) {
            net_conntrack_extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->initiator_flow().flow_data());
            flowupd.flow_state.syn_ack_delta = ctx.sess_spec()->iflow_syn_ack_delta();
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
