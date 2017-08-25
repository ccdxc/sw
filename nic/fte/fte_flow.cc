#include "fte_flow.hpp"
#include "fte_ctx.hpp"
#include <session.hpp>
#include <defines.h>

namespace fte {

// fte flow api
// merge header_rewrite op with another header_rewrite
hal_ret_t
flow_t::merge_header_rewrite(header_rewrite_info_t &dst,
                             const  header_rewrite_info_t &src)
{
    if (src.flags.dec_ttl) {
        dst.flags.dec_ttl = 1;
    }
    
    // L2 header fields
    switch(src.valid_hdrs&FTE_L2_HEADERS) {
    case FTE_HEADER_ether:
        HEADER_COPY_FLD(dst, src, ether, smac);
        HEADER_COPY_FLD(dst, src, ether, dmac);
        HEADER_COPY_FLD(dst, src, ether, vlan_id);
        HEADER_COPY_FLD(dst, src, ether, dot1p);
        break;
    default:
        HAL_ASSERT_RETURN(0, HAL_RET_INVALID_ARG);
    }
    
    // l3 header fields
    switch (src.valid_hdrs&FTE_L3_HEADERS) {
    case FTE_HEADER_ipv4:
        HEADER_COPY_FLD(dst, src, ipv4, sip);
        HEADER_COPY_FLD(dst, src, ipv4, dip);
        HEADER_COPY_FLD(dst, src, ipv4, ttl);
        HEADER_COPY_FLD(dst, src, ipv4, dscp);
        break;
    case FTE_HEADER_ipv6:
        HEADER_COPY_FLD(dst, src, ipv6, sip);
        HEADER_COPY_FLD(dst, src, ipv6, dip);
        HEADER_COPY_FLD(dst, src, ipv6, ttl);
        HEADER_COPY_FLD(dst, src, ipv6, dscp);
        break;
    default:
        HAL_ASSERT_RETURN(0, HAL_RET_INVALID_ARG);
    }
    
    // l4 header fields
    switch (src.valid_hdrs&FTE_L4_HEADERS) {
    case FTE_HEADER_tcp:
        HEADER_COPY_FLD(dst, src, tcp, sport);
        HEADER_COPY_FLD(dst, src, tcp, dport);
        break;
    case FTE_HEADER_udp:
        HEADER_COPY_FLD(dst, src, udp, sport);
        HEADER_COPY_FLD(dst, src, udp, dport);
        break;
    default:
        HAL_ASSERT_RETURN(0, HAL_RET_INVALID_ARG);
    }

    return HAL_RET_OK;
}

// merger header_rewrite op with  header_push
hal_ret_t
flow_t::merge_header_rewrite_with_push(header_push_info_t &dst,
                                       const  header_rewrite_info_t &src)
{
    // L2 header fields
    switch(src.valid_hdrs&FTE_L2_HEADERS) {
    case FTE_HEADER_ether:
        HEADER_COPY_FLD(dst, src, ether, smac);
        HEADER_COPY_FLD(dst, src, ether, dmac);
        HEADER_COPY_FLD(dst, src, ether, vlan_id);
        break;
    default:
        HAL_ASSERT_RETURN(0, HAL_RET_INVALID_ARG);
    }

    // l3 header fields
    switch (src.valid_hdrs&FTE_L3_HEADERS) {
    case FTE_HEADER_ipv4:
        HEADER_COPY_FLD(dst, src, ipv4, sip);
        HEADER_COPY_FLD(dst, src, ipv4, dip);
        break;
    case FTE_HEADER_ipv6:
        HEADER_COPY_FLD(dst, src, ipv6, sip);
        HEADER_COPY_FLD(dst, src, ipv6, dip);
        break;
    default:
        HAL_ASSERT_RETURN(0, HAL_RET_INVALID_ARG);
    }

    return HAL_RET_OK;
}

hal_ret_t flow_t::header_pop(const header_pop_info_t &header_pop)
{
    if (num_header_updates_ >= MAX_HEADER_UPDATES) {
        HAL_TRACE_ERR("header updates exceeded");
        return HAL_RET_ERR;
    }

    header_update_t *entry = &header_updates_[num_header_updates_++];
    entry->type = HEADER_POP;
    entry->header_pop = header_pop;

    return HAL_RET_OK;
}

hal_ret_t flow_t::header_push(const header_push_info_t &header_push)
{
    if (!valid_tunnel_headers(header_push.valid_hdrs)) {
        return HAL_RET_INVALID_ARG;
    }

    if (num_header_updates_ >= MAX_HEADER_UPDATES) {
        HAL_TRACE_ERR("header updates exceeded");
        return HAL_RET_ERR;
    }

    header_update_t *entry = &header_updates_[num_header_updates_++];
    entry->type = HEADER_PUSH;
    entry->header_push = header_push;
    return HAL_RET_OK;
}

hal_ret_t flow_t::header_rewrite(const header_rewrite_info_t &header_rewrite)
{
    header_update_t *entry;
    hal_ret_t ret = HAL_RET_OK;

    if (!valid_headers(header_rewrite.valid_hdrs)) {
        return HAL_RET_INVALID_ARG;
    }

    if (num_header_updates_ == 0) {
        // No previous entries, allocate new one
        entry = &header_updates_[num_header_updates_++];
        entry->type = HEADER_REWRITE; 
        entry->header_rewrite = header_rewrite;
        return HAL_RET_OK;
    }


    // Check the previous header type and merge into it if possible
    entry = &header_updates_[num_header_updates_-1];

    switch (entry->type) {
    case HEADER_POP:
        // Allocate new update entry
        if (num_header_updates_ >= MAX_HEADER_UPDATES) {
            HAL_TRACE_ERR("header updates exceeded");
            ret = HAL_RET_ERR;
            break;
        }

        entry = &header_updates_[num_header_updates_++];
        entry->type = HEADER_REWRITE;
        entry->header_rewrite = header_rewrite;
        break;
    case HEADER_REWRITE:
        ret = merge_header_rewrite(entry->header_rewrite, header_rewrite);
        break;

    case HEADER_PUSH:
        ret = merge_header_rewrite_with_push(entry->header_push, header_rewrite);
        break;
    }

    return ret;
}


rewrite_actions_enum
flow_t::nat_rewrite_action(header_type_t l3_type, header_type_t l4_type, int nat_type)
{
    // index1: 0(ipv4), 1(ipv6)
    // index2: 0(snat), 1(dnat), 2(twice_nat)
    // index3: 0(tcp), 1(udp), 2(none)
    static rewrite_actions_enum actions[2][3][3] = {
        {{REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID, REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID, REWRITE_IPV4_NAT_SRC_REWRITE_ID},
         {REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID, REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID, REWRITE_IPV4_NAT_DST_REWRITE_ID},
         {REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID, REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID, REWRITE_IPV4_TWICE_NAT_REWRITE_ID}},
        {{REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID, REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID, REWRITE_IPV4_NAT_SRC_REWRITE_ID},
         {REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID, REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID, REWRITE_IPV4_NAT_DST_REWRITE_ID},
         {REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID, REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID, REWRITE_IPV4_TWICE_NAT_REWRITE_ID}}
    };
    
    int index1, index2, index3;
    switch(l3_type) {
    case FTE_HEADER_ipv4: index1 = 0; break;
    case FTE_HEADER_ipv6: index1 = 1; break;
    default: return REWRITE_NOP_ID;
    }

    switch (nat_type) {
    case hal::NAT_TYPE_SNAT: index2 = 0; break; 
    case hal::NAT_TYPE_DNAT: index2 = 1; break;
    case hal::NAT_TYPE_TWICE_NAT: index2 = 2; break;
    default: return REWRITE_NOP_ID;
    }

    switch (l4_type) {
    case FTE_HEADER_tcp: index3 = 0; break;
    case FTE_HEADER_udp: index3 = 1; break;
    default: index3 = 2; break;
    }

    return actions[index1][index2][index3];
}

hal_ret_t flow_t::build_rewrite_config(hal::flow_cfg_t &flow_cfg,
                                       const header_rewrite_info_t &rewrite)
{
       // flags
    flow_cfg.ttl_dec = rewrite.flags.dec_ttl;

    // MAC rewrite
    flow_cfg.mac_sa_rewrite = rewrite.valid_flds.smac;
    flow_cfg.mac_da_rewrite = rewrite.valid_flds.dmac;
    if (flow_cfg.mac_sa_rewrite || flow_cfg.mac_da_rewrite) {
        flow_cfg.rw_act = REWRITE_REWRITE_ID;
        // TODO(goli)update rewrite table with sa da
    }

    //VLAN rewrite
    if (rewrite.valid_flds.vlan_id) {
        flow_cfg.tnnl_vnid = rewrite.ether.vlan_id;
        flow_cfg.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VLAN_ID;
    }

    // NAT rewrite
    bool snat = rewrite.valid_flds.sip || rewrite.valid_flds.sport;
    bool dnat = rewrite.valid_flds.dip || rewrite.valid_flds.dport;

    if (snat && dnat){
        flow_cfg.nat_type = hal::NAT_TYPE_TWICE_NAT;
    } else if (snat){
        flow_cfg.nat_type = hal::NAT_TYPE_SNAT;
    } else if (dnat) {
        flow_cfg.nat_type = hal::NAT_TYPE_DNAT;
    } else {
        flow_cfg.nat_type = hal::NAT_TYPE_NONE;
    }

    if (flow_cfg.nat_type != hal::NAT_TYPE_NONE) {
        flow_cfg.rw_act = nat_rewrite_action(rewrite.valid_hdrs&FTE_L3_HEADERS,
                                              rewrite.valid_hdrs&FTE_L4_HEADERS,
                                              flow_cfg.nat_type);
    }

    // L3 rewrite
    switch(rewrite.valid_hdrs&FTE_L3_HEADERS) {
    case FTE_HEADER_ipv4:
        if (rewrite.valid_flds.sip) {
            flow_cfg.nat_sip.af = IP_AF_IPV4;
            flow_cfg.nat_sip.addr.v4_addr = rewrite.ipv4.sip;
        }
        if (rewrite.valid_flds.dip) {
            flow_cfg.nat_dip.af = IP_AF_IPV4;
            flow_cfg.nat_dip.addr.v4_addr = rewrite.ipv4.dip;
        }
        break;
    case FTE_HEADER_ipv6:
        if (rewrite.valid_flds.sip) {
            flow_cfg.nat_sip.af = IP_AF_IPV6;
            flow_cfg.nat_sip.addr.v6_addr = rewrite.ipv6.sip;
        }
        if (rewrite.valid_flds.dip) {
            flow_cfg.nat_dip.af = IP_AF_IPV6;
            flow_cfg.nat_dip.addr.v6_addr = rewrite.ipv6.dip;
        }
        break;
    }

    // L4 rewrite
    switch(rewrite.valid_hdrs&FTE_L4_HEADERS) {
    case FTE_HEADER_tcp:
        if (rewrite.valid_flds.sport) {
            flow_cfg.nat_sport = rewrite.tcp.sport;
        }
        if (rewrite.valid_flds.dport) {
            flow_cfg.nat_sport = rewrite.tcp.dport;
        }
        break;
    case FTE_HEADER_udp:
        if (rewrite.valid_flds.sport) {
            flow_cfg.nat_sport = rewrite.udp.sport;
        }
        if (rewrite.valid_flds.dport) {
            flow_cfg.nat_sport = rewrite.udp.dport;
        }
        break;
    }

    //TODO(goli)need to create rewrite table entry with appropraite action (l3 or nat_xxx)

    return HAL_RET_OK;
}

hal_ret_t flow_t::to_config(hal::flow_cfg_t &flow_cfg) const
{
    hal_ret_t ret;
    flow_cfg.key = key_;

    flow_cfg.action = action_.deny ? session::FLOW_ACTION_DROP : session::FLOW_ACTION_ALLOW;

    // conn_track info
    flow_cfg.state = conn_track_.state;

    // fwding info
    flow_cfg.lport = fwding_.lport;
    flow_cfg.qid_en = fwding_.qid_en;
    flow_cfg.qtype = fwding_.qtype;
    flow_cfg.qid = fwding_.qid;

    // header manipulations
    for (int i = 0; i < num_header_updates_; i++) {
        const header_update_t *entry = &header_updates_[i];
        switch (entry->type) {
        case HEADER_POP:
            // TODO(goli) not supportef yet
            ret = HAL_RET_INVALID_OP;
            break;

        case HEADER_PUSH:
            // TODO(goli) not supportef yet
            ret = HAL_RET_INVALID_OP;
            break;

        case HEADER_REWRITE:
            ret = build_rewrite_config(flow_cfg, entry->header_rewrite);
            break;
        }

        if (ret != HAL_RET_OK) {
            break;
        }
    }

    return ret;
}

hal_ret_t flow_t::merge_flow(const flow_t &flow) 
{
    hal_ret_t ret = HAL_RET_OK;

    if (flow.valid_action()) {
        set_action(flow.action());
    }

    if (flow.valid_conn_track()) {
        set_conn_track(flow.conn_track());
    }

    if (flow.valid_fwding()) {
        set_fwding(flow.fwding());
    }

    for(int i = 0; i < flow.num_header_updates_; i++) {
        const header_update_t *entry = &flow.header_updates_[i];
        switch (entry->type) {
        case HEADER_POP:
            ret = header_pop(entry->header_pop);
            break;

        case HEADER_PUSH:
            ret = header_push(entry->header_push);
            break;

        case HEADER_REWRITE:
            ret = header_rewrite(entry->header_rewrite);
            break;
        }

        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    return ret;
}

void flow_t::from_config(const hal::flow_cfg_t &flow_cfg)
{
    // TODO(goli)
}

} // namespace fte
