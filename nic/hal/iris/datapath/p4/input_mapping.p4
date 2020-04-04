/*****************************************************************************/
/* Input mapping processing                                                  */
/*****************************************************************************/
action set_packet_type(mac_da) {
    modify_field(flow_lkp_metadata.pkt_type, PACKET_TYPE_UNICAST);
    if ((mac_da & 0x010000000000) == 0x010000000000) {
        modify_field(flow_lkp_metadata.pkt_type, PACKET_TYPE_MULTICAST);
    }
    if (mac_da == 0xFFFFFFFFFFFF) {
        modify_field(flow_lkp_metadata.pkt_type, PACKET_TYPE_BROADCAST);
    }
}

action tunneled_ipv4_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4);
    modify_field(flow_lkp_metadata.lkp_src, inner_ipv4.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, inner_ipv4.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, inner_ipv4.protocol);
    modify_field(flow_lkp_metadata.ipv4_flags, inner_ipv4.flags);
    modify_field(flow_lkp_metadata.ipv4_hlen, inner_ipv4.ihl);
    modify_field(flow_lkp_metadata.ip_ttl, inner_ipv4.ttl);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    modify_field(l3_metadata.ip_option_seen, l3_metadata.inner_ip_option_seen);
    modify_field(l3_metadata.ip_frag, l3_metadata.inner_ip_frag);
    modify_field(l4_metadata.tcp_data_len,
                 (inner_ipv4.totalLen - ((inner_ipv4.ihl + tcp.dataOffset) * 4)));
    if (roce_bth.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_sport, 0);
    }
    if (inner_ethernet.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_srcMacAddr, inner_ethernet.srcAddr);
        modify_field(flow_lkp_metadata.lkp_dstMacAddr, inner_ethernet.dstAddr);
        set_packet_type(inner_ethernet.dstAddr);
    } else {
        modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
        modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
        set_packet_type(ethernet.dstAddr);
    }

    if (tunnel_metadata.tunnel_type == INGRESS_TUNNEL_TYPE_MPLS_L3VPN) {
        modify_field(tunnel_metadata.tunnel_type, 0);
        modify_field(tunnel_metadata.tunnel_vni, 0);
    }
}

action tunneled_ipv6_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6);
    modify_field(flow_lkp_metadata.lkp_proto, l3_metadata.inner_ipv6_ulp);
    modify_field(flow_lkp_metadata.ip_ttl, inner_ipv6.hopLimit);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    modify_field(l3_metadata.ip_option_seen, l3_metadata.inner_ip_option_seen);
    modify_field(l4_metadata.tcp_data_len,
                 (inner_ipv6.payloadLen - (tcp.dataOffset) * 4));
    if (roce_bth.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_sport, 0);
    }
    if (inner_ethernet.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_srcMacAddr, inner_ethernet.srcAddr);
        modify_field(flow_lkp_metadata.lkp_dstMacAddr, inner_ethernet.dstAddr);
        set_packet_type(inner_ethernet.dstAddr);
    } else {
        modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
        modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
        set_packet_type(ethernet.dstAddr);
    }
}

action tunneled_non_ip_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC);
    modify_field(flow_lkp_metadata.lkp_src, inner_ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, inner_ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, inner_ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, inner_ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, 0);
    modify_field(flow_lkp_metadata.lkp_dport, inner_ethernet.etherType);
    modify_field(flow_lkp_metadata.lkp_sport, 0);
    modify_field(flow_lkp_metadata.ip_ttl, 0);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    set_packet_type(inner_ethernet.dstAddr);
}

action native_ipv4_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4);
    modify_field(flow_lkp_metadata.lkp_src, ipv4.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, ipv4.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, ipv4.protocol);
    modify_field(flow_lkp_metadata.ipv4_flags, ipv4.flags);
    modify_field(flow_lkp_metadata.ipv4_hlen, ipv4.ihl);
    modify_field(flow_lkp_metadata.ip_ttl, ipv4.ttl);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
    modify_field(l4_metadata.tcp_data_len,
                 (ipv4.totalLen - ((ipv4.ihl + tcp.dataOffset) * 4)));
    if (esp.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_proto, IP_PROTO_IPSEC_ESP);
    } else {
        if (ipv4.protocol == IP_PROTO_UDP) {
            if (roce_bth.valid == TRUE) {
                modify_field(flow_lkp_metadata.lkp_sport, 0);
            } else {
                modify_field(flow_lkp_metadata.lkp_sport, udp.srcPort);
            }
            modify_field(flow_lkp_metadata.lkp_dport, udp.dstPort);
        }
        if ((ipv4.protocol != IP_PROTO_TCP) and
            (ipv4.protocol != IP_PROTO_UDP) and
            (ipv4.protocol != IP_PROTO_ICMP)) {
            modify_field(flow_lkp_metadata.lkp_sport, 0);
            modify_field(flow_lkp_metadata.lkp_dport, 0);
        }
    }
    set_packet_type(ethernet.dstAddr);

    modify_field(tunnel_metadata.tunnel_type, 0);
    modify_field(tunnel_metadata.tunnel_vni, 0);

    // Just to make NCC happy. CRITICAL::Could not allocate memory for ingress tcam table 'input_mapping_native'.
    modify_field(scratch_metadata.lif, capri_intrinsic.lif);
}

action native_ipv6_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6);
    modify_field(flow_lkp_metadata.lkp_proto, l3_metadata.ipv6_ulp);
    modify_field(flow_lkp_metadata.ip_ttl, ipv6.hopLimit);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
    modify_field(l4_metadata.tcp_data_len,
                 (ipv6.payloadLen - (tcp.dataOffset) * 4));
    if (l3_metadata.ipv6_ulp == IP_PROTO_UDP) {
        if (roce_bth.valid == TRUE) {
            modify_field(flow_lkp_metadata.lkp_sport, 0);
        } else {
            modify_field(flow_lkp_metadata.lkp_sport, udp.srcPort);
        }
        modify_field(flow_lkp_metadata.lkp_dport, udp.dstPort);
    }
    if ((l3_metadata.ipv6_ulp!= IP_PROTO_TCP) and
        (l3_metadata.ipv6_ulp != IP_PROTO_UDP) and
        (l3_metadata.ipv6_ulp != IP_PROTO_ICMPV6)) {
        modify_field(flow_lkp_metadata.lkp_sport, 0);
        modify_field(flow_lkp_metadata.lkp_dport, 0);
    }
    set_packet_type(ethernet.dstAddr);

    modify_field(tunnel_metadata.tunnel_type, 0);
    modify_field(tunnel_metadata.tunnel_vni, 0);

    // Just to make NCC happy. CRITICAL::Could not allocate memory for ingress tcam table 'input_mapping_native'.
    modify_field(scratch_metadata.lif, capri_intrinsic.lif);
}

action native_ipv6_packet2() {
    if (tunnel_metadata.tunnel_terminate == FALSE) {
        modify_field(flow_lkp_metadata.lkp_src, ipv6.srcAddr);
        modify_field(flow_lkp_metadata.lkp_dst, ipv6.dstAddr);
    }
}

action native_non_ip_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC);
    modify_field(flow_lkp_metadata.lkp_src, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, 0);
    if (vlan_tag.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_dport, vlan_tag.etherType);
    } else {
        modify_field(flow_lkp_metadata.lkp_dport, ethernet.etherType);
    }
    modify_field(flow_lkp_metadata.lkp_dport, 0);
    modify_field(flow_lkp_metadata.ip_ttl, 0);
    set_packet_type(ethernet.dstAddr);

    modify_field(tunnel_metadata.tunnel_type, 0);
    modify_field(tunnel_metadata.tunnel_vni, 0);

    // Just to make NCC happy. CRITICAL::Could not allocate memory for ingress tcam table 'input_mapping_native'.
    modify_field(scratch_metadata.lif, capri_intrinsic.lif);
}

action input_mapping_miss() {
    modify_field(control_metadata.drop_reason, DROP_INPUT_MAPPING);
    drop_packet();
}

@pragma stage 0
table input_mapping_tunneled {
    reads {
        entry_inactive.input_mapping : ternary;
        tunnel_metadata.tunnel_type  : ternary;
        roce_bth.valid               : ternary;
        ipv4.valid                   : ternary;
        ipv6.valid                   : ternary;
        inner_ipv4.valid             : ternary;
        inner_ipv6.valid             : ternary;
        ipv4.dstAddr                 : ternary;
    }
    actions {
        tunneled_ipv4_packet;
        tunneled_ipv6_packet;
        tunneled_non_ip_packet;
        nop;
    }
    default_action : nop;
    size : INPUT_MAPPING_TABLE_SIZE;
}

@pragma stage 0
table input_mapping_native {
    reads {
        entry_inactive.input_mapping : ternary;
        tunnel_metadata.tunnel_type  : ternary;
        roce_bth.valid               : ternary;
        ipv4.valid                   : ternary;
        ipv6.valid                   : ternary;
        inner_ipv4.valid             : ternary;
        inner_ipv6.valid             : ternary;
        ipv4.dstAddr                 : ternary;
    }
    actions {
        native_ipv4_packet;
        native_ipv6_packet;
        native_non_ip_packet;
        input_mapping_miss;
        nop;
    }
    default_action : input_mapping_miss;
    size : INPUT_MAPPING_TABLE_SIZE;
}

@pragma stage 1
table input_mapping_native2 {
    actions {
        native_ipv6_packet2;
    }
    default_action : native_ipv6_packet2;
}

action input_properties(vrf, reg_mac_vrf, dir, flow_miss_qos_class_id,
                        flow_miss_idx, ipsg_enable, clear_promiscuous_repl,
                        l4_profile_idx, dst_lport, src_lport, rewrite_index,
                        tunnel_rewrite_index, bounce_vnid, mirror_on_drop_en,
                        mirror_on_drop_session_id, mdest_flow_miss_action,
                        if_label_check_en, if_label_check_fail_drop,
                        mseg_bm_bc_repls, mseg_bm_mc_repls, flow_learn,
                        src_if_label, uuc_fl_pe_sup_en, has_prom_host_lifs) {
    modify_field(flow_lkp_metadata.lkp_vrf, vrf);
    modify_field(flow_lkp_metadata.lkp_dir, dir);
    modify_field(control_metadata.mdest_flow_miss_action, mdest_flow_miss_action);
    modify_field(control_metadata.flow_miss_qos_class_id, flow_miss_qos_class_id);
    modify_field(capri_intrinsic.tm_replicate_ptr, flow_miss_idx);
    modify_field(control_metadata.ipsg_enable, ipsg_enable);
    modify_field(l4_metadata.profile_idx, l4_profile_idx);
    modify_field(control_metadata.src_lport, src_lport);
    modify_field(control_metadata.dst_lport, dst_lport);
    modify_field(rewrite_metadata.rewrite_index, rewrite_index);
    modify_field(rewrite_metadata.tunnel_rewrite_index, tunnel_rewrite_index);
    modify_field(rewrite_metadata.tunnel_vnid, bounce_vnid);
    modify_field(control_metadata.mirror_on_drop_en, mirror_on_drop_en);
    modify_field(control_metadata.mirror_on_drop_session_id,
                 mirror_on_drop_session_id);
    modify_field(control_metadata.clear_promiscuous_repl, clear_promiscuous_repl);
    modify_field(control_metadata.if_label_check_en, if_label_check_en);
    modify_field(control_metadata.if_label_check_fail_drop, if_label_check_fail_drop);
    modify_field(control_metadata.src_if_label, src_if_label);
    modify_field(control_metadata.mseg_bm_bc_repls, mseg_bm_bc_repls);
    modify_field(control_metadata.mseg_bm_mc_repls, mseg_bm_mc_repls);
    modify_field(control_metadata.flow_learn, flow_learn);
    modify_field(control_metadata.uuc_fl_pe_sup_en, uuc_fl_pe_sup_en);
    modify_field(control_metadata.has_prom_host_lifs, has_prom_host_lifs);
    if (flow_lkp_metadata.pkt_type == PACKET_TYPE_MULTICAST) {
        modify_field(flow_lkp_metadata.lkp_reg_mac_vrf, reg_mac_vrf);
    } else {
        modify_field(flow_lkp_metadata.lkp_reg_mac_vrf, vrf);
    }
}

@pragma stage 1
@pragma overflow_table input_properties
table input_properties_otcam {
    reads {
        entry_inactive.input_properties : ternary;
        capri_intrinsic.lif             : ternary;
        p4plus_to_p4.insert_vlan_tag    : ternary;
        vlan_tag.valid                  : ternary;
        vlan_tag.vid                    : ternary;
        tunnel_metadata.tunnel_type     : ternary;
        tunnel_metadata.tunnel_vni      : ternary;
    }
    actions {
        input_properties;
    }
    size : INPUT_PROPERTIES_OTCAM_TABLE_SIZE;
}

@pragma stage 1
@pragma hash_type 0
table input_properties {
    reads {
        entry_inactive.input_properties : exact;
        capri_intrinsic.lif             : exact;
        p4plus_to_p4.insert_vlan_tag    : exact;
        vlan_tag.valid                  : exact;
        vlan_tag.vid                    : exact;
        tunnel_metadata.tunnel_type     : exact;
        tunnel_metadata.tunnel_vni      : exact;
    }
    actions {
        input_properties;
    }
    size : INPUT_PROPERTIES_TABLE_SIZE;
}

action input_properties_mac_vlan(vrf, dir, mdest_flow_miss_action,
                                 flow_miss_qos_class_id,
                                 flow_miss_idx, ipsg_enable,
                                 l4_profile_idx, dst_lport, src_lport, src_if_label,
                                 rewrite_index,
                                 tunnel_rewrite_index, tunnel_vnid,
                                 tunnel_originate, mirror_on_drop_en,
                                 mirror_on_drop_session_id, skip_flow_update,
                                 ep_learn_en, clear_ingresss_mirror) {
    adjust_lkp_fields();
    modify_field(control_metadata.src_lif, capri_intrinsic.lif);

    // if table is a miss, return. do not perform rest of the actions.

    modify_field(control_metadata.clear_ingresss_mirror, clear_ingresss_mirror);
    if (clear_ingresss_mirror == TRUE) {
        // For loop back packets from network
        // return
    }

    modify_field(control_metadata.skip_flow_update, skip_flow_update);
    if (skip_flow_update == TRUE) {
        // return;
    }

    modify_field(control_metadata.ep_learn_en, ep_learn_en);
    // modify_field(rewrite_metadata.rewrite_index, rewrite_index);
    modify_field(flow_miss_metadata.tunnel_originate, tunnel_originate);

    input_properties(vrf, vrf, dir, flow_miss_qos_class_id,
                     flow_miss_idx, ipsg_enable,
                     0,  // clear_promiscuous_repl
                     l4_profile_idx, dst_lport, src_lport, rewrite_index,
                     tunnel_rewrite_index, tunnel_vnid, mirror_on_drop_en,
                     mirror_on_drop_session_id, mdest_flow_miss_action,
                     1,  // if_label_check_en
                     0,  // if_label_check_fail_drop
                     1,  // mseg_bm_bc_repls
                     1,  // mseg_bm_mc_repls
                     1,  // flow_learn
                     src_if_label,
                     1,  // uuc_fl_pe_sup_en
                     0); // has_prom_host_lifs
    modify_field(scratch_metadata.src_if_label, src_if_label);
}

action adjust_lkp_fields() {
    if (control_metadata.tm_iport == TM_PORT_DMA) {
        modify_field(flow_lkp_metadata.lkp_inst, p4plus_to_p4.lkp_inst);
        subtract(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_TXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TO_P4_HDR_SZ));
    } else {
        subtract(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size,
                 CAPRI_GLOBAL_INTRINSIC_HDR_SZ);
    }
    if (recirc_header.valid == TRUE) {
        modify_field(control_metadata.recirc_reason, recirc_header.reason);
        modify_field(qos_metadata.qos_class_id, capri_intrinsic.tm_iq);
        subtract(scratch_metadata.packet_len, scratch_metadata.packet_len,
                 P4_RECIRC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    }

    modify_field(capri_p4_intrinsic.packet_len, scratch_metadata.packet_len);
}

// this table will be programmed during enic-if create time frame only
// in pvlan mode, program (isolated-vlan, mac) and (user-vlan, mac)
// in useg mode, program (useg-vlan, mac) and (user-vlan, mac)
// NOTE: (user-vlan, mac) is only needed in end-host mode for dejavu
//       checks
// in direct-io mode, don't program any mode
@pragma stage 1
table input_properties_mac_vlan {
    reads {
        entry_inactive.input_mac_vlan : ternary;
        control_metadata.uplink       : ternary;
        p4plus_to_p4.insert_vlan_tag  : ternary;
        vlan_tag.valid                : ternary;
        vlan_tag.vid                  : ternary;
        ethernet.srcAddr              : ternary;
    }
    actions {
        input_properties_mac_vlan;
    }
    size : INPUT_PROPERTIES_MAC_VLAN_TABLE_SIZE;
}

/******************************************************************************/
/* LIF info                                                                   */
/******************************************************************************/
action lif_info(ingress_mirror_en, ingress_mirror_session_id) {
    if (ingress_mirror_en == TRUE) {
        modify_field(capri_intrinsic.tm_span_session, ingress_mirror_session_id);
    }
    modify_field(scratch_metadata.flag, ingress_mirror_en);
}

@pragma stage 0
table lif {
    reads {
        capri_intrinsic.lif : exact;
    }
    actions {
        lif_info;
    }
    size : LIF_TABLE_SIZE;
}

control process_input_mapping {
    apply(lif);
    apply(input_mapping_tunneled);
    apply(input_mapping_native);
    if (ipv6.valid == TRUE) {
        apply(input_mapping_native2);
    }
    apply(input_properties);
    apply(input_properties_otcam);
    apply(input_properties_mac_vlan);
}
