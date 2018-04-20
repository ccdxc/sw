/*
 * exec.cc
 */

#include "core.hpp"

namespace hal {
namespace plugins {
namespace nat {

/*
 * update_flow
 *   Update flow from NAT spec
 */
static inline hal_ret_t
update_flow_from_nat_spec (fte::ctx_t& ctx)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    ipv6_addr_t addr;
    const session::FlowInfo *flow_info;

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR)
        flow_info = &ctx.sess_spec()->initiator_flow().flow_data().flow_info();
    else
        flow_info = &ctx.sess_spec()->responder_flow().flow_data().flow_info();

    auto nat_type = flow_info->nat_type();

    if (nat_type == session::NAT_TYPE_SNAT ||
        nat_type == session::NAT_TYPE_TWICE_NAT) {
        if (flow_info->has_nat_sip()) {
            if (flow_info->nat_sip().ip_af() == types::IP_AF_INET) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, sip,
                               flow_info->nat_sip().v4_addr());
            } else {
                memcpy(addr.addr8, flow_info->nat_sip().v6_addr().c_str(),
                       IP6_ADDR8_LEN);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, sip, addr);
            }
        }

        if (flow_info->nat_sport()) {
            if (ctx.key().proto == types::IPPROTO_TCP) {
                HEADER_SET_FLD(flowupd.header_rewrite, tcp, sport,
                               flow_info->nat_sport());
            } else {
                HEADER_SET_FLD(flowupd.header_rewrite, udp, sport,
                               flow_info->nat_sport());
            }
        }
    }

    if (nat_type == session::NAT_TYPE_DNAT ||
        nat_type == session::NAT_TYPE_TWICE_NAT) {
        if (flow_info->has_nat_dip()) {
            if (flow_info->nat_dip().ip_af() == types::IP_AF_INET) {
                HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dip,
                               flow_info->nat_dip().v4_addr());
            } else {
                memcpy(addr.addr8, flow_info->nat_dip().v6_addr().c_str(),
                       IP6_ADDR8_LEN);
                HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dip, addr);
            }
        }

        if (flow_info->nat_dport()) {
            if (ctx.key().proto == types::IPPROTO_TCP) {
                HEADER_SET_FLD(flowupd.header_rewrite, tcp, dport,
                               flow_info->nat_dport());
            } else {
                HEADER_SET_FLD(flowupd.header_rewrite, udp, dport,
                               flow_info->nat_dport());
            }
        }

        if (flow_info->nat_dmac()) {
            mac_addr_t dmac;
            MAC_UINT64_TO_ADDR(dmac, flow_info->nat_dmac());
            HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac,
                           *(ether_addr *)dmac);
        }
    }

    return ctx.update_flow(flowupd);
}

/*
 * nat_exec
 *   Entry point into the NAT feature
 */
fte::pipeline_action_t
nat_exec (fte::ctx_t &ctx)
{
    hal_ret_t ret;

    if (ctx.protobuf_request()) {
        ret = update_flow_from_nat_spec(ctx);
    } else {
        // TODO(goli) pick a pip from l4lb service
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace nat
} // namespace plugins
} // namespace hal
