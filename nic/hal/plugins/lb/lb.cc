#include "nic/fte/fte.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"

namespace hal {
namespace lb {

static inline hal_ret_t
update_flow_from_nat_spec(fte::ctx_t& ctx, const session::FlowInfo& flow_info)
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

        if (flow_info.nat_dmac()) {
            mac_addr_t dmac;
            MAC_UINT64_TO_ADDR(dmac, flow_info.nat_dmac());
            HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac, *(ether_addr *)dmac);
        }
    }

    return ctx.update_flow(flowupd);
}

l4lb_service_entry_t *
lookup_l4lb_service(const hal::flow_key_t &key)
{
    hal::l4lb_key_t              l4lb_key{};
    hal::l4lb_service_entry_t    *l4lb = NULL;

    switch (key.flow_type) {
    case  hal::FLOW_TYPE_L2:
        return NULL;
    case hal::FLOW_TYPE_V4:
        l4lb_key.service_ip.af = IP_AF_IPV4;
        break;
    case hal::FLOW_TYPE_V6:
        l4lb_key.service_ip.af = IP_AF_IPV6;
        break;
    }

    l4lb_key.vrf_id = key.dvrf_id;
    l4lb_key.proto = key.proto;
    l4lb_key.service_ip.addr = key.dip;
    l4lb_key.service_port = key.dport;
    l4lb = find_l4lb_by_key(&l4lb_key);
    if (l4lb == NULL){
        // lookup with 0 port
        l4lb_key.service_port = 0;
        l4lb = find_l4lb_by_key(&l4lb_key);
    }

    return l4lb;
}

static inline hal_ret_t
rewrite_vmac(fte::ctx_t& ctx, hal::ep_t *odep)
{
    l4lb_service_entry_t  *l4lb;

    // Rewrite dmac(iflow)/smac(rflow) for lb
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        // iflow - rewrite dmac to pip's mac
        // TODO(goli) - if dep is not known, we need to get the mac from lb spec
        if (ctx.dep() && odep != ctx.dep()) {
            HEADER_SET_FLD(flowupd.header_rewrite, ether, dmac, 
                           *(ether_addr *)hal::ep_get_mac_addr(ctx.dep()));
        }
    } else {
        // rflow - rewrite smac to vip's mac
        l4lb = lookup_l4lb_service(ctx.key());
        if (l4lb) {
            HEADER_SET_FLD(flowupd.header_rewrite, ether, smac,
                           *(ether_addr *)l4lb->serv_mac_addr);
        }
    }

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
lb_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    hal::ep_t *dep = ctx.dep();

    if (ctx.protobuf_request()) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR){
            ret = update_flow_from_nat_spec(ctx, 
                     ctx.sess_spec()->initiator_flow().flow_data().flow_info());
        } else {
            ret = update_flow_from_nat_spec(ctx, 
                     ctx.sess_spec()->responder_flow().flow_data().flow_info());
        }
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END; 
        }
    } else {
        // TOTO(goli) pick a pip from l4lb service
    }

    ret = rewrite_vmac(ctx, dep);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
