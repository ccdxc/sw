#include "nic/fte/fte_flow.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

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

    if (!memcmp(&dst, &src, sizeof(header_rewrite_info_t))) {
        return HAL_RET_ENTRY_EXISTS;
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


    // check the previous header type and merge into it if possible
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

hal::rewrite_actions_enum
flow_t::nat_rewrite_action(header_type_t l3_type, header_type_t l4_type,
                           session::NatType nat_type)
{
    static int proto_tcp = 0, proto_udp = 1, proto_other = 2;
    static int af_v4 = 0, af_v6 = 1;
    static hal::rewrite_actions_enum snat_actions[2][3] = {
        {hal::REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID, hal::REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID,
         hal::REWRITE_IPV4_NAT_SRC_REWRITE_ID},
        {hal::REWRITE_IPV6_NAT_SRC_TCP_REWRITE_ID, hal::REWRITE_IPV6_NAT_SRC_UDP_REWRITE_ID,
         hal::REWRITE_IPV6_NAT_SRC_REWRITE_ID}
    };
    static hal::rewrite_actions_enum dnat_actions[2][3] = {
        {hal::REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID, hal::REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID,
         hal::REWRITE_IPV4_NAT_DST_REWRITE_ID},
        {hal::REWRITE_IPV6_NAT_DST_TCP_REWRITE_ID, hal::REWRITE_IPV6_NAT_DST_UDP_REWRITE_ID,
         hal::REWRITE_IPV6_NAT_DST_REWRITE_ID}
    };

    static hal::rewrite_actions_enum twice_nat_actions[2][3] = {
        {hal::REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID, hal::REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID,
         hal::REWRITE_IPV4_TWICE_NAT_REWRITE_ID},
        {hal::REWRITE_IPV6_TWICE_NAT_TCP_REWRITE_ID, hal::REWRITE_IPV6_TWICE_NAT_UDP_REWRITE_ID,
         hal::REWRITE_IPV6_TWICE_NAT_REWRITE_ID}
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
        return hal::REWRITE_REWRITE_ID;
    }
}

hal_ret_t flow_t::build_rewrite_config(hal::flow_cfg_t &config,
                                       hal::flow_pgm_attrs_t &attrs,
                                       const header_rewrite_info_t &rewrite) const
{
    hal_ret_t ret = HAL_RET_OK;
    bool snat, dnat;
    hal::pd::pd_rw_entry_args_t rw_key{};

    attrs.rw_act = hal::REWRITE_NOP_ID;
    attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_NOP_ID;


    // flags
    attrs.ttl_dec = rewrite.flags.dec_ttl;

    // MAC rewrite
    if (rewrite.valid_flds.smac) {
        attrs.mac_sa_rewrite = true;
        uint8_t *mac_a = rw_key.mac_sa;
        struct ether_addr *eth_addr = (struct ether_addr *)mac_a;
        *eth_addr = rewrite.ether.smac;
    }
    if (rewrite.valid_flds.dmac) {
        attrs.mac_da_rewrite = true;
        uint8_t *mac_a = rw_key.mac_da;
        struct ether_addr *eth_addr = (struct ether_addr *)mac_a;
        *eth_addr = rewrite.ether.dmac;
    }

    //VLAN rewrite
    if (rewrite.valid_flds.vlan_id) {
        attrs.tnnl_vnid = rewrite.ether.vlan_id;
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_VLAN_ID;
    }

    // L3 rewrite
    switch(rewrite.valid_hdrs&FTE_L3_HEADERS) {
    case FTE_HEADER_ipv4:
        if (rewrite.valid_flds.sip) {
            config.nat_sip.af = IP_AF_IPV4;
            config.nat_sip.addr.v4_addr = rewrite.ipv4.sip;
        }
        if (rewrite.valid_flds.dip) {
            config.nat_dip.af = IP_AF_IPV4;
            config.nat_dip.addr.v4_addr = rewrite.ipv4.dip;
        }
        break;
    case FTE_HEADER_ipv6:
        if (rewrite.valid_flds.sip) {
            config.nat_sip.af = IP_AF_IPV6;
            config.nat_sip.addr.v6_addr = rewrite.ipv6.sip;
        }
        if (rewrite.valid_flds.dip) {
            config.nat_dip.af = IP_AF_IPV6;
            config.nat_dip.addr.v6_addr = rewrite.ipv6.dip;
        }
        break;
    }

    // L4 rewrite
    switch(rewrite.valid_hdrs&FTE_L4_HEADERS) {
    case FTE_HEADER_tcp:
        if (rewrite.valid_flds.sport) {
            config.nat_sport = rewrite.tcp.sport;
        }
        if (rewrite.valid_flds.dport) {
            config.nat_dport = rewrite.tcp.dport;
        }
        break;
    case FTE_HEADER_udp:
        if (rewrite.valid_flds.sport) {
            config.nat_sport = rewrite.udp.sport;
        }
        if (rewrite.valid_flds.dport) {
            config.nat_dport = rewrite.udp.dport;
        }
        break;
    }

    // If only l4 port is changed, use the IP from flow key
    if (!rewrite.valid_flds.sip && rewrite.valid_flds.sport) {
        config.nat_sip.af = (key_.flow_type == hal::FLOW_TYPE_V4) ? IP_AF_IPV4 : IP_AF_IPV6;
        config.nat_sip.addr = key_.sip;
    }

    if (!rewrite.valid_flds.dip && rewrite.valid_flds.dport) {
        config.nat_dip.af = (key_.flow_type == hal::FLOW_TYPE_V4) ? IP_AF_IPV4 : IP_AF_IPV6;
        config.nat_dip.addr = key_.dip;
    }

    snat = (rewrite.valid_flds.sip || rewrite.valid_flds.sport);
    dnat = (rewrite.valid_flds.dip || rewrite.valid_flds.dport);

    if (snat && dnat){
        attrs.nat_ip = config.nat_sip;
        attrs.nat_l4_port = config.nat_sport;
        config.nat_type = session::NAT_TYPE_TWICE_NAT;
    } else if (snat){
        attrs.nat_ip = config.nat_sip;
        attrs.nat_l4_port = config.nat_sport;
        config.nat_type = session::NAT_TYPE_SNAT;
    } else if (dnat){
        attrs.nat_ip = config.nat_dip;
        attrs.nat_l4_port = config.nat_dport;
        config.nat_type = session::NAT_TYPE_DNAT;
    } else {
        config.nat_type = session::NAT_TYPE_NONE;
    }

    // rewrite action/index
    attrs.rw_act = nat_rewrite_action(rewrite.valid_hdrs&FTE_L3_HEADERS,
                                      rewrite.valid_hdrs&FTE_L4_HEADERS,
                                      (session::NatType)config.nat_type);
    rw_key.rw_act = attrs.rw_act;

    hal::pd::pd_rw_entry_find_or_alloc_args_t r_args;
    hal::pd::pd_func_args_t          pd_func_args = {0};
    r_args.args = &rw_key;
    r_args.rw_idx = &attrs.rw_idx;
    // ret = hal::pd::pd_rw_entry_find_or_alloc(&rw_key, &attrs.rw_idx);
    pd_func_args.pd_rw_entry_find_or_alloc = &r_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_RWENTRY_FIND_OR_ALLOC, &pd_func_args);
    if (ret != HAL_RET_OK) {
        return ret;
    }


    //twice-nat idx
    // TODO(goli) delete the idx on freeing flow
    if (config.nat_type == session::NAT_TYPE_TWICE_NAT){
		hal::pd::pd_twice_nat_entry_args_t args;
		args.twice_nat_act = hal::pd::TWICE_NAT_TWICE_NAT_REWRITE_INFO_ID;
		args.nat_ip = config.nat_dip;
		args.nat_l4_port = config.nat_dport;
		hal::pd::pd_twice_nat_add_args_t t_args;
		t_args.args = &args;
		t_args.twice_nat_idx = &attrs.twice_nat_idx;
        pd_func_args.pd_twice_nat_add = &t_args;
		ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TWICE_NAT_ADD, &pd_func_args);
		// ret = pd_twice_nat_add(&args, &attrs.twice_nat_idx);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // tunnel rewrite action
    attrs.tnnl_rw_act = rewrite.valid_flds.vlan_id ? hal::TUNNEL_REWRITE_ENCAP_VLAN_ID :
        hal::TUNNEL_REWRITE_NOP_ID;

    return ret;
}

hal_ret_t flow_t::build_push_header_config(hal::flow_pgm_attrs_t &attrs,
                                           const header_push_info_t &header) const
{
    attrs.tunnel_orig = TRUE;

    switch (header.valid_hdrs&FTE_ENCAP_HEADERS) {
    case FTE_HEADER_vxlan:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_VXLAN_ID;
        attrs.tnnl_vnid = header.vxlan.vrf_id;
        break;
    case FTE_HEADER_erspan:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_ERSPAN_ID;
        break;
#ifdef PHASE2
    case FTE_HEADER_ipsec_esp:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_IPSEC_ID;
        break;
    case FTE_HEADER_vxlan_gpe:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_VXLAN_GPE_ID;
        attrs.tnnl_vnid = header.vxlan_gpe.vrf_id;
        break;
    case FTE_HEADER_geneve:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_GENV_ID;
        attrs.tnnl_vnid = header.geneve.vrf_id;
      break;
    case FTE_HEADER_nvgre:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_NVGRE_ID;
        attrs.tnnl_vnid = header.geneve.vrf_id;
        break;
    case FTE_HEADER_gre:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_GRE_ID;
        break;
    case FTE_HEADER_mpls:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_MPLS_ID;
        break;
    case FTE_HEADER_ip_in_ip:
        attrs.tnnl_rw_act = hal::TUNNEL_REWRITE_ENCAP_IP_ID;
        break;
#endif /* PHASE2 */
    default:
        HAL_TRACE_ERR("fte: invalid encap");
        return HAL_RET_ERR;
    }

    return HAL_RET_OK;
}

hal_ret_t flow_t::get_rewrite_config(const hal::flow_cfg_t &config,
                                     const hal::flow_pgm_attrs_t  &attrs,
                                     header_rewrite_info_t *rewrite)
{
    hal_ret_t ret = HAL_RET_OK;

    rewrite->flags.dec_ttl =  attrs.ttl_dec;
    if (attrs.mac_sa_rewrite) {
        rewrite->valid_flds.smac = true;
        // How to get the actual mac - its not stored anywhere!!
    }

    if (attrs.mac_da_rewrite) {
        rewrite->valid_flds.dmac = true;
        // How to get the actual mac - its not stored anywhere!!
    }

    // vlan
    if (attrs.tnnl_rw_act == hal::TUNNEL_REWRITE_ENCAP_VLAN_ID) {
        HEADER_SET_FLD(*rewrite, ether, vlan_id, attrs.tnnl_vnid)
    }

    // sip
    if (!ip_addr_is_zero(&config.nat_sip)) {
        if (config.nat_sip.af == IP_AF_IPV4) {
            HEADER_SET_FLD(*rewrite, ipv4, sip, config.nat_sip.addr.v4_addr);
        } else {
            HEADER_SET_FLD(*rewrite, ipv6, sip, config.nat_sip.addr.v6_addr);
        }
    }

    // dip
    if (!ip_addr_is_zero(&config.nat_dip)) {
        if (config.nat_dip.af == IP_AF_IPV4) {
            HEADER_SET_FLD(*rewrite, ipv4, dip, config.nat_dip.addr.v4_addr);
        } else {
            HEADER_SET_FLD(*rewrite, ipv6, dip, config.nat_dip.addr.v6_addr);
        }
    }

    // sport
    if (config.nat_sport) {
        if  (key_.proto == IP_PROTO_TCP) {
            HEADER_SET_FLD(*rewrite, tcp, sport, config.nat_sport);
        } else  if  (key_.proto == IP_PROTO_UDP) {
            HEADER_SET_FLD(*rewrite, udp, sport, config.nat_sport);
        }
    }

    // dport
    if (config.nat_dport) {
        if  (key_.proto == IP_PROTO_TCP) {
            HEADER_SET_FLD(*rewrite, tcp, dport, config.nat_dport);
        } else  if  (key_.proto == IP_PROTO_UDP) {
            HEADER_SET_FLD(*rewrite, udp, dport, config.nat_dport);
        }
    }

    return ret;
}

hal_ret_t flow_t::to_config(hal::flow_cfg_t &config, hal::flow_pgm_attrs_t &attrs) const
{
    hal_ret_t ret;

    config.key = key_;
    config.dir = direction_; 
    memcpy(config.l2_info.smac, l2_info_.smac, sizeof(config.l2_info.smac));
    memcpy(config.l2_info.dmac, l2_info_.dmac, sizeof(config.l2_info.dmac));
    config.l2_info.l2seg_id = l2_info_.l2seg_id;

    // Restore the old attrs
    if (valid_.attrs) {
        attrs = attrs_;
    }

    if (valid_.action) {
        config.action = action_;
        attrs.drop =  (action_ == session::FLOW_ACTION_DROP);
    }

    if (valid_.mcast_info) {

        // Evaluate which mcast ptr is ultimately applicable
        attrs.mcast_en = mcast_info_.mcast_en;
        attrs.mcast_ptr = 0;
        if (attrs.mcast_en) {
            if (mcast_info_.mcast_ptr && mcast_info_.proxy_mcast_ptr) {
                HAL_TRACE_ERR("fte: {} cannot set both mcast ptrs", __FUNCTION__);
                ret = HAL_RET_INVALID_OP;
                goto end;
            }

            if (mcast_info_.proxy_mcast_ptr)
                attrs.is_proxy_mcast = 1;

            attrs.mcast_ptr = mcast_info_.mcast_ptr ? mcast_info_.mcast_ptr :
                                                      mcast_info_.proxy_mcast_ptr;
        }
    }

    if (valid_.ingress_info) {
        if (ingress_info_.expected_sif) {
            attrs.expected_src_lif_en = 1;
            attrs.expected_src_lif = ingress_info_.hw_lif_id;
        }
    }

    if (valid_.qos_info) {
        attrs.qos_class_en = qos_info_.qos_class_en;
        attrs.qos_class_id = qos_info_.qos_class_id;
    }

    if (valid_.fwding) {
        attrs.lport = fwding_.lport;
        attrs.qid_en =  fwding_.qid_en;
        attrs.qtype = fwding_.qtype;
        attrs.qid = fwding_.qid;
    }

    if (valid_.mirror_info) {
        if (mirror_info_.proxy_ing_mirror_session)
            config.is_ing_proxy_mirror = 1;
        if (mirror_info_.proxy_egr_mirror_session)
            config.is_eg_proxy_mirror = 1;

        config.ing_mirror_session = mirror_info_.ing_mirror_session |
                                    mirror_info_.proxy_ing_mirror_session;
        config.eg_mirror_session = mirror_info_.egr_mirror_session |
                                   mirror_info_.proxy_egr_mirror_session;
    }

    if (valid_.export_info) {
        attrs.export_en = export_info_.export_en;
        attrs.export_id1 = export_info_.export_id1;
        attrs.export_id2 = export_info_.export_id2;
        attrs.export_id3 = export_info_.export_id3;
        attrs.export_id4 = export_info_.export_id4;
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
            ret = build_rewrite_config(config, attrs, entry->header_rewrite);
            break;
        }

        if (ret != HAL_RET_OK) {
            break;
        }
    }

end:
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

    if (flow.valid_mirror_info()) {
        merge_mirror_info(flow.mirror_info());
    }

    if (flow.valid_export_info()) {
        merge_export_info(flow.export_info());
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

void flow_t::from_config(const hal::flow_cfg_t &flow_cfg,
                         const hal::flow_pgm_attrs_t  &attrs,
                         const hal::session_t *session)
{
    header_update_t *entry;

    key_ = flow_cfg.key;
    valid_.key = 1;

    direction_ = (hal::flow_direction_t)flow_cfg.dir;
    valid_.direction = 1;
 
    l2_info_.l2seg_id = flow_cfg.l2_info.l2seg_id;
    memcpy(l2_info_.smac, flow_cfg.l2_info.smac, sizeof(l2_info_.smac));
    memcpy(l2_info_.dmac, flow_cfg.l2_info.dmac, sizeof(l2_info_.dmac)); 
    valid_.l2_info = 1;

    attrs_ = attrs;
    valid_.attrs = 1;

    is_proxy_enabled_ = attrs.is_proxy_en;

    action_ = (session::FlowAction)flow_cfg.action;

    if (attrs.mcast_en) {
        if (attrs.is_proxy_mcast)
            mcast_info_.proxy_mcast_ptr = attrs.mcast_ptr;
        else
            mcast_info_.mcast_ptr = attrs.mcast_ptr;
        mcast_info_.mcast_en = attrs.mcast_en;
    }

    if (attrs.expected_src_lif_en) {
        ingress_info_.hw_lif_id = attrs.expected_src_lif;
    }

    if (attrs.qos_class_en) {
        qos_info_.qos_class_en = attrs.qos_class_en;
        qos_info_.qos_class_id = attrs.qos_class_id;
    }

    if (attrs.lport || attrs.qid_en || attrs.qtype || attrs.qid) {
        fwding_.lport = attrs.lport;
        fwding_.qid_en = attrs.qid_en;
        fwding_.qtype = attrs.qtype;
        fwding_.qid = attrs.qid;
        valid_.fwding = true;
    }

    if (flow_cfg.is_ing_proxy_mirror)
        mirror_info_.proxy_ing_mirror_session = flow_cfg.ing_mirror_session;
    else
        mirror_info_.ing_mirror_session = flow_cfg.ing_mirror_session;

    if (flow_cfg.is_eg_proxy_mirror)
        mirror_info_.proxy_egr_mirror_session = flow_cfg.eg_mirror_session;
    else
        mirror_info_.egr_mirror_session = flow_cfg.eg_mirror_session;
   
    export_info_.export_id1 = (attrs.export_en & (1 << 0)) ? attrs.export_id1 :
                                                             export_info_.export_id1;
    export_info_.export_id2 = (attrs.export_en & (1 << 1)) ? attrs.export_id2 :
                                                             export_info_.export_id2;
    export_info_.export_id3 = (attrs.export_en & (1 << 2)) ? attrs.export_id3 :
                                                             export_info_.export_id3;
    export_info_.export_id4 = (attrs.export_en & (1 << 3)) ? attrs.export_id4 :
                                                             export_info_.export_id4;
    export_info_.export_en = attrs.export_en;

    if (session->idle_timeout != HAL_MAX_INACTIVTY_TIMEOUT) {
        aging_info_.idle_timeout = session->idle_timeout;
        valid_.aging_info = true;
    }

    if (session->sfw_rule_id) {
        sfw_info_.sfw_rule_id = session->sfw_rule_id;
        sfw_info_.sfw_action = session->sfw_action;
        sfw_info_.skip_sfw_reval = session->skip_sfw_reval;
    }

    // Header rewrite
    entry = &header_updates_[num_header_updates_++];
    entry->type = HEADER_REWRITE;
    get_rewrite_config(flow_cfg, attrs, &entry->header_rewrite);
}

}    // namespace fte
