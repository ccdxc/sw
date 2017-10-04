#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace net {

static inline session::FlowAction
pol_check_sg_policy(fte::ctx_t &ctx)
{
    if (ctx.protobuf_request()) {
        return ctx.sess_spec()->initiator_flow().flow_data().flow_info().flow_action();
    }

    if (hal::is_forwarding_mode_host_pinned() &&
        ctx.sep()->ep_flags & EP_FLAGS_REMOTE &&
        ctx.dep()->ep_flags & EP_FLAGS_REMOTE) {
        return session::FLOW_ACTION_DROP;
    }

    // TODO(goli) check Security policy
    return session::FLOW_ACTION_ALLOW;
}

//------------------------------------------------------------------------------
// extract all the TCP related state from session spec
//------------------------------------------------------------------------------
static hal_ret_t
extract_session_state_from_spec (fte::flow_state_t *flow_state,
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
conn_tracking_configured(fte::ctx_t &ctx)
{

    if (ctx.protobuf_request()) {
        return ctx.sess_spec()->conn_track_en();
    }

    if (ctx.key().proto != types::IPPROTO_TCP) {
        return false;
    }

    // lookup Security profile
    if (ctx.tenant()->nwsec_profile_handle  != HAL_HANDLE_INVALID) {
        hal::nwsec_profile_t  *nwsec_prof =
            nwsec_profile_lookup_by_handle(ctx.tenant()->nwsec_profile_handle);
        if (nwsec_prof != NULL) {
            return nwsec_prof->cnxn_tracking_en;
        }
    }
    return false;
}

fte::pipeline_action_t
dfw_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;


    // security policy action
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};
    flowupd.action = pol_check_sg_policy(ctx);
    ctx.update_flow(flowupd);

    // connection tracking
    if (!conn_tracking_configured(ctx)) {
        return fte::PIPELINE_CONTINUE;
    }

    //iflow
    flowupd = {type: fte::FLOWUPD_FLOW_STATE};

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        if (ctx.protobuf_request()) {
            extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->initiator_flow().flow_data());
            flowupd.flow_state.syn_ack_delta = ctx.sess_spec()->iflow_syn_ack_delta();
        } else {
            const fte::cpu_rxhdr_t *rxhdr = ctx.cpu_rxhdr();
            flowupd.flow_state.state = session::FLOW_TCP_STATE_SYN_RCVD;
            // Expectation is to program the next expected seq num.
            flowupd.flow_state.tcp_seq_num = ntohl(rxhdr->tcp_seq_num) + 1;
            flowupd.flow_state.tcp_ack_num = ntohl(rxhdr->tcp_ack_num);
            flowupd.flow_state.tcp_win_sz = ntohs(rxhdr->tcp_window);
            flowupd.flow_state.tcp_win_scale = rxhdr->tcp_ws;
            flowupd.flow_state.tcp_mss = ntohs(rxhdr->tcp_mss);
            
        }
    } else {
        if (ctx.protobuf_request()) {
            extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->responder_flow().flow_data());
        } else {
            flowupd.flow_state.state = session::FLOW_TCP_STATE_INIT;
            flowupd.flow_state.tcp_win_sz = 512; //This is to allow the SYN Packet to go through.
        }
    }

    ret = ctx.update_flow(flowupd); 
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
