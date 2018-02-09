//{C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "nic/fte/fte.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/p4/nw/include/defines.h"

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

static void
process_tcp_close(fte::ctx_t& ctx) 
{
    // Start a timer to cleanup session if this is the first FIN/RST received
    // Timer is started in order to wait for either side to close the TCP 
    // connection
    if (ctx.existing_session() && ctx.session()->tcp_close_timer == NULL) {
        hal::tcp_close_timer_schedule(ctx.session()); 
    }
}

fte::pipeline_action_t
conntrack_exec(fte::ctx_t& ctx)
{
    hal_ret_t       ret;

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FLOW_STATE};

    if (ctx.tcp_close()) {
        process_tcp_close(ctx);
        return fte::PIPELINE_CONTINUE; 
    }

    if (!net_conntrack_configured(ctx)) {
        return fte::PIPELINE_CONTINUE;
    }

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
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
