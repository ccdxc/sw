#include "net_plugin.hpp"

namespace hal {
namespace net {

static inline hal_ret_t
update_flow_from_nat_spec(fte::ctx_t& ctx, hal::flow_role_t role,
                          const session::FlowInfo& flow_info)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    ipv6_addr_t addr;

    auto nat_type = flow_info.nat_type();
    if (nat_type == session::NAT_TYPE_SNAT || nat_type == session::NAT_TYPE_TWICE_NAT) {
        if (flow_info.has_nat_sip()) {
            if (flow_info.nat_sip().ip_af() == types::IP_AF_INET) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, sip, flow_info.nat_sip().v4_addr());
            } else {
                memcpy(addr.addr8, flow_info.nat_sip().v6_addr().c_str(), IP6_ADDR8_LEN);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, sip, addr);
            }
        }

        if (flow_info.nat_sport()) {
            if (ctx.key().proto == types::IPPROTO_TCP) {
                HEADER_SET_FLD(flowupd.header_rewrite, tcp, sport, flow_info.nat_sport());
            } else {
                HEADER_SET_FLD(flowupd.header_rewrite, udp, sport, flow_info.nat_sport());
            }
        }
    }

    if (nat_type == session::NAT_TYPE_DNAT || nat_type == session::NAT_TYPE_TWICE_NAT) {
        if (flow_info.has_nat_dip()) {
            if (flow_info.nat_dip().ip_af() == types::IP_AF_INET) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dip, flow_info.nat_dip().v4_addr());
            } else {
                memcpy(addr.addr8, flow_info.nat_dip().v6_addr().c_str(), IP6_ADDR8_LEN);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dip, addr);
            }
        }

        if (flow_info.nat_dport()) {
            if (ctx.key().proto == types::IPPROTO_TCP) {
                HEADER_SET_FLD(flowupd.header_rewrite, tcp, dport, flow_info.nat_dport());
            } else {
                HEADER_SET_FLD(flowupd.header_rewrite, udp, dport, flow_info.nat_dport());
            }
        }
    }

    return ctx.update_flow(role, flowupd);
}

fte::pipeline_action_t
lb_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking lb feature");

    if (ctx.protobuf_request()) {
        update_flow_from_nat_spec(ctx, hal::FLOW_ROLE_INITIATOR,
                                  ctx.sess_spec()->initiator_flow().flow_data().flow_info());
        
        update_flow_from_nat_spec(ctx, hal::FLOW_ROLE_RESPONDER,
                                  ctx.sess_spec()->initiator_flow().flow_data().flow_info());
        return fte::PIPELINE_CONTINUE;
    }

    // TODO(goli) LB policy check
    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
