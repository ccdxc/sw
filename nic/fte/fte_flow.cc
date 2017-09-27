#include "nic/include/pd_api.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/p4/nw/include/defines.h"

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
    }

    return HAL_RET_OK;
}

hal_ret_t flow_t::header_pop(const header_pop_info_t &header_pop)
{
    if (num_header_updates_ >= MAX_HEADER_UPDATES) {
        HAL_TRACE_ERR("fte: header updates exceeded");
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
        HAL_TRACE_ERR("fte: header updates exceeded");
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
            HAL_TRACE_ERR("fte: header updates exceeded");
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
flow_t::nat_rewrite_action(header_type_t l3_type, header_type_t l4_type,
                           session::NatType nat_type)
{
    static int proto_tcp = 0, proto_udp = 1, proto_other = 2;
    static int af_v4 = 0, af_v6 = 1;
    static rewrite_actions_enum snat_actions[2][3] = {
        {REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID, REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID,
         REWRITE_IPV4_NAT_SRC_REWRITE_ID},
        {REWRITE_IPV6_NAT_SRC_TCP_REWRITE_ID, REWRITE_IPV6_NAT_SRC_UDP_REWRITE_ID,
         REWRITE_IPV6_NAT_SRC_REWRITE_ID}
    };
    static rewrite_actions_enum dnat_actions[2][3] = {
        {REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID, REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID,
         REWRITE_IPV4_NAT_DST_REWRITE_ID},
        {REWRITE_IPV6_NAT_DST_TCP_REWRITE_ID, REWRITE_IPV6_NAT_DST_UDP_REWRITE_ID,
         REWRITE_IPV6_NAT_DST_REWRITE_ID}
    };

    static rewrite_actions_enum twice_nat_actions[2][3] = {
        {REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID, REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID,
         REWRITE_IPV4_TWICE_NAT_REWRITE_ID},
        {REWRITE_IPV6_TWICE_NAT_TCP_REWRITE_ID, REWRITE_IPV6_TWICE_NAT_UDP_REWRITE_ID,
         REWRITE_IPV6_TWICE_NAT_REWRITE_ID}
    };

    int proto = (l4_type == FTE_HEADER_tcp) ? proto_tcp :
        (l4_type == FTE_HEADER_udp) ? proto_udp : proto_other;
    int af = (l3_type == FTE_HEADER_ipv4) ? af_v4: af_v6;

    switch(nat_type) {
    case session::NAT_TYPE_TWICE_NAT:
        return twice_nat_actions[af][proto];
    case session::NAT_TYPE_SNAT:
        return snat_actions[af][proto];
    case session::NAT_TYPE_DNAT:
        return dnat_actions[af][proto];
    default:
        return REWRITE_REWRITE_ID;        
    }
}

hal_ret_t flow_t::build_rewrite_config(hal::flow_pgm_attrs_t &attrs,
                                       const header_rewrite_info_t &rewrite)
{
    hal_ret_t ret = HAL_RET_OK;
    bool snat, dnat;
    session::NatType nat_type;
    ip_addr_t nat_sip{}, nat_dip{};
    uint16_t nat_sport{}, nat_dport{};
    hal::pd::pd_rw_entry_args_t rw_key{};

    attrs.rw_act = REWRITE_NOP_ID;
    attrs.tnnl_rw_act = TUNNEL_REWRITE_NOP_ID;


    // flags
    attrs.ttl_dec = rewrite.flags.dec_ttl;

    // MAC rewrite
    if (rewrite.valid_flds.smac) {
        attrs.mac_sa_rewrite = true;
        *(struct ether_addr *)rw_key.mac_sa = rewrite.ether.smac;
    }
    if (rewrite.valid_flds.dmac) {
        attrs.mac_da_rewrite = true;
        *(struct ether_addr *)rw_key.mac_da = rewrite.ether.dmac;
    }

    //VLAN rewrite
    if (rewrite.valid_flds.vlan_id) {
        attrs.tnnl_vnid = rewrite.ether.vlan_id;
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VLAN_ID;
    }
    if (rewrite.valid_flds.dot1p) {
        attrs.dot1p = rewrite.ether.dot1p;
        attrs.dot1p_en = true;
    }

    // L3 rewrite
    switch(rewrite.valid_hdrs&FTE_L3_HEADERS) {
    case FTE_HEADER_ipv4:
        if (rewrite.valid_flds.sip) {
            nat_sip.af = IP_AF_IPV4;
            nat_sip.addr.v4_addr = rewrite.ipv4.sip;
        }
        if (rewrite.valid_flds.dip) {
            nat_dip.af = IP_AF_IPV4;
            nat_dip.addr.v4_addr = rewrite.ipv4.dip;
        }
        if (rewrite.valid_flds.dscp) {
            attrs.dscp_en = true;
            attrs.dscp = rewrite.ipv4.dscp;
        }
        break;
    case FTE_HEADER_ipv6:
        if (rewrite.valid_flds.sip) {
            nat_sip.af = IP_AF_IPV6;
            nat_sip.addr.v6_addr = rewrite.ipv6.sip;
        }
        if (rewrite.valid_flds.dip) {
            nat_dip.af = IP_AF_IPV6;
            nat_dip.addr.v6_addr = rewrite.ipv6.dip;
        }
        break;
    }

    // L4 rewrite
    switch(rewrite.valid_hdrs&FTE_L4_HEADERS) {
    case FTE_HEADER_tcp:
        if (rewrite.valid_flds.sport) {
            nat_sport = rewrite.tcp.sport;
        }
        if (rewrite.valid_flds.dport) {
            nat_dport = rewrite.tcp.dport;
        }
        break;
    case FTE_HEADER_udp:
        if (rewrite.valid_flds.sport) {
            nat_sport = rewrite.udp.sport;
        }
        if (rewrite.valid_flds.dport) {
            nat_dport = rewrite.udp.dport;
        }
        break;
    }

    snat = (rewrite.valid_flds.sip || rewrite.valid_flds.sport);
    dnat = (rewrite.valid_flds.dip || rewrite.valid_flds.dport);
    if (snat && dnat){
        // TODO(goli)we need to flip these if role is responder
        attrs.nat_dip = nat_dip;
        attrs.nat_dport = attrs.nat_l4_port = nat_dport;
        attrs.nat_sip = nat_sip;
        attrs.nat_sport = nat_sport;
        nat_type = session::NAT_TYPE_TWICE_NAT;
    } else if (snat){
        attrs.nat_sip = nat_sip;
        attrs.nat_sport = attrs.nat_l4_port = nat_sport;
        nat_type = session::NAT_TYPE_SNAT;
    } else if (dnat){
        attrs.nat_dip = nat_dip;
        attrs.nat_dport = attrs.nat_l4_port = nat_dport;
        nat_type = session::NAT_TYPE_DNAT;
    } else {
        nat_type = session::NAT_TYPE_NONE;
    }

    attrs.nat_type = nat_type;

    // rewrite action/index
    attrs.rw_act = nat_rewrite_action(rewrite.valid_hdrs&FTE_L3_HEADERS,
                                      rewrite.valid_hdrs&FTE_L4_HEADERS,
                                      nat_type);
    rw_key.rw_act = attrs.rw_act;

    ret = hal::pd::pd_rw_entry_find_or_alloc(&rw_key, &attrs.rw_idx);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // tunnel rewrite action
    attrs.tnnl_rw_act = rewrite.valid_flds.vlan_id ? TUNNEL_REWRITE_ENCAP_VLAN_ID :
        TUNNEL_REWRITE_NOP_ID;

    return ret;
}

hal_ret_t flow_t::build_push_header_config(hal::flow_pgm_attrs_t &attrs,
                                           const header_push_info_t &header)
{
    attrs.tunnel_orig = TRUE;

    switch (header.valid_hdrs&FTE_ENCAP_HEADERS) {
    case FTE_HEADER_vxlan:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VXLAN_ID;
        attrs.tnnl_vnid = header.vxlan.tenant_id;
        break;
    case FTE_HEADER_erspan:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_ERSPAN_ID;
        break;
    case FTE_HEADER_ipsec_esp:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_IPSEC_ID;
        break;
#ifdef PHASE2
    case FTE_HEADER_vxlan_gpe:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_VXLAN_GPE_ID;
        attrs.tnnl_vnid = header.vxlan_gpe.tenant_id;
        break;
    case FTE_HEADER_geneve:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_GENV_ID;
        attrs.tnnl_vnid = header.geneve.tenant_id;
      break;
    case FTE_HEADER_nvgre:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_NVGRE_ID;
        attrs.tnnl_vnid = header.geneve.tenant_id;
        break;
    case FTE_HEADER_gre:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_GRE_ID;
        break;
    case FTE_HEADER_mpls:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_MPLS_ID;
        break;
    case FTE_HEADER_ip_in_ip:
        attrs.tnnl_rw_act = TUNNEL_REWRITE_ENCAP_IP_ID;
        break;
#endif /* PHASE2 */
    default:
        HAL_TRACE_ERR("fte: invalid encap");
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

hal_ret_t flow_t::to_config(hal::flow_cfg_t &config, hal::flow_pgm_attrs_t &attrs) const
{
    hal_ret_t ret;

    config.key = key_;

    if (valid_.action) {
        config.action = action_;
        attrs.drop =  (action_ == session::FLOW_ACTION_DROP);
    }

    if (valid_.fwding) {
        attrs.lport = fwding_.lport;
        attrs.qid_en =  fwding_.qid_en;
        attrs.qtype = fwding_.qtype;
        attrs.qid = fwding_.qid;
    }

    // header manipulations
    for (int i = 0; i < num_header_updates_; i++) {
        const header_update_t *entry = &header_updates_[i];
        switch (entry->type) {
        case HEADER_POP:
            // TODO(goli) not supportef yet
            ret = HAL_RET_INVALID_OP;
            break;

        case HEADER_PUSH:
            ret = build_push_header_config(attrs, entry->header_push);
            break;

        case HEADER_REWRITE:
            ret = build_rewrite_config(attrs, entry->header_rewrite);
            break;
        }

        if (ret != HAL_RET_OK) {
            break;
        }
    }

    // Fill the config
    config.nat_type = attrs.nat_type;
    config.nat_sip = attrs.nat_sip;
    config.nat_dip = attrs.nat_dip;
    config.nat_sport = attrs.nat_sport;
    config.nat_dport = attrs.nat_dport;
    config.eg_qos_action.marking_action.pcp_rewrite_en = attrs.dot1p_en;
    config.eg_qos_action.marking_action.pcp = attrs.dot1p;
    config.eg_qos_action.marking_action.dscp_rewrite_en = attrs.dscp_en;
    config.eg_qos_action.marking_action.dscp = attrs.dscp;
    return ret;
}

hal_ret_t flow_t::merge_flow(const flow_t &flow) 
{
    hal_ret_t ret = HAL_RET_OK;

    if (flow.valid_action()) {
        set_action(flow.action());
    }

    if (flow.valid_flow_state()) {
        set_flow_state(flow.flow_state());
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

void flow_t::from_config(const hal::flow_cfg_t &flow_cfg, const hal::flow_pgm_attrs_t  &attrs)
{
    // TODO(goli)
}

} // namespace fte
