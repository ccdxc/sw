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
    modify_field(flow_lkp_metadata.ipv4_frag_offset, inner_ipv4.fragOffset);
    modify_field(flow_lkp_metadata.ipv4_hlen, inner_ipv4.ihl);
    modify_field(flow_lkp_metadata.ip_ttl, inner_ipv4.ttl);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    modify_field(l3_metadata.ip_option_seen, l3_metadata.inner_ip_option_seen);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, inner_ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, inner_ethernet.dstAddr);
    if (inner_ipv4.protocol == IP_PROTO_TCP) {
        modify_field(l4_metadata.tcp_data_len,
                     (inner_ipv4.totalLen -
                      ((inner_ipv4.ihl + tcp.dataOffset) * 4)));
    }
    set_packet_type(inner_ethernet.dstAddr);
    validate_tunneled_packet();
}

action tunneled_ipv6_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6);
    modify_field(flow_lkp_metadata.lkp_proto, inner_ipv6.nextHdr);
    modify_field(flow_lkp_metadata.ip_ttl, inner_ipv6.hopLimit);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, inner_ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, inner_ethernet.dstAddr);
    if (inner_ipv6.nextHdr == IP_PROTO_TCP) {
        modify_field(l4_metadata.tcp_data_len,
                     (inner_ipv6.payloadLen - (tcp.dataOffset) * 4));
    }
    set_packet_type(inner_ethernet.dstAddr);
    validate_tunneled_packet();
}

action tunneled_non_ip_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC);
    modify_field(flow_lkp_metadata.lkp_src, inner_ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, inner_ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, 0);
    modify_field(flow_lkp_metadata.lkp_sport, inner_ethernet.etherType);
    modify_field(flow_lkp_metadata.lkp_dport, 0);
    modify_field(flow_lkp_metadata.ip_ttl, 0);
    modify_field(tunnel_metadata.tunnel_terminate, TRUE);
    set_packet_type(inner_ethernet.dstAddr);
    validate_tunneled_packet();
}

action tunneled_vm_bounce_packet() {
    // if OAM bit is set, packet to destined to a VM otherwise packet is from
    // a VM
    if (vxlan_gpe.flags == 0x08) {
        modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_TO_VM_BOUNCE);
        modify_field(flow_lkp_metadata.lkp_dst, inner_ethernet.dstAddr);
    } else {
        modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_FROM_VM_BOUNCE);
        modify_field(flow_lkp_metadata.lkp_src, inner_ethernet.srcAddr);
    }
}

action native_ipv4_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV4);
    modify_field(flow_lkp_metadata.lkp_src, ipv4.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, ipv4.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, ipv4.protocol);
    modify_field(flow_lkp_metadata.ipv4_flags, ipv4.flags);
    modify_field(flow_lkp_metadata.ipv4_frag_offset, ipv4.fragOffset);
    modify_field(flow_lkp_metadata.ipv4_hlen, ipv4.ihl);
    modify_field(flow_lkp_metadata.ip_ttl, ipv4.ttl);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
    if (ipv4.protocol == IP_PROTO_TCP) {
        modify_field(l4_metadata.tcp_data_len,
                     (ipv4.totalLen - ((ipv4.ihl + tcp.dataOffset) * 4)));
    }
    if (ipv4.protocol == IP_PROTO_UDP) {
        modify_field(flow_lkp_metadata.lkp_sport, udp.srcPort);
        modify_field(flow_lkp_metadata.lkp_dport, udp.dstPort);
    }
    set_packet_type(ethernet.dstAddr);
    validate_native_packet();
}

action native_ipv6_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_IPV6);
    modify_field(flow_lkp_metadata.lkp_src, ipv6.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, ipv6.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, ipv6.nextHdr);
    modify_field(flow_lkp_metadata.ip_ttl, ipv6.hopLimit);
    modify_field(flow_lkp_metadata.lkp_srcMacAddr, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dstMacAddr, ethernet.dstAddr);
    if (ipv6.nextHdr == IP_PROTO_TCP) {
        modify_field(l4_metadata.tcp_data_len,
                     (ipv6.payloadLen - (tcp.dataOffset) * 4));
    }
    if (ipv6.nextHdr == IP_PROTO_UDP) {
        modify_field(flow_lkp_metadata.lkp_sport, udp.srcPort);
        modify_field(flow_lkp_metadata.lkp_dport, udp.dstPort);
    }
    set_packet_type(ethernet.dstAddr);
    validate_native_packet();
}

action native_non_ip_packet() {
    modify_field(flow_lkp_metadata.lkp_type, FLOW_KEY_LOOKUP_TYPE_MAC);
    modify_field(flow_lkp_metadata.lkp_src, ethernet.srcAddr);
    modify_field(flow_lkp_metadata.lkp_dst, ethernet.dstAddr);
    modify_field(flow_lkp_metadata.lkp_proto, 0);
    if (vlan_tag.valid == TRUE) {
        modify_field(flow_lkp_metadata.lkp_sport, vlan_tag.etherType);
    } else {
        modify_field(flow_lkp_metadata.lkp_sport, ethernet.etherType);
    }
    modify_field(flow_lkp_metadata.lkp_sport, ethernet.etherType);
    modify_field(flow_lkp_metadata.lkp_dport, 0);
    modify_field(flow_lkp_metadata.ip_ttl, 0);
    set_packet_type(ethernet.dstAddr);
    validate_native_packet();
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
        mpls[0].valid                : ternary;
        ipv4.valid                   : ternary;
        ipv6.valid                   : ternary;
        inner_ipv4.valid             : ternary;
        inner_ipv6.valid             : ternary;
        ipv4.dstAddr                 : ternary;
        ipv6.dstAddr                 : ternary;
    }
    actions {
        tunneled_ipv4_packet;
        tunneled_ipv6_packet;
        tunneled_non_ip_packet;
        tunneled_vm_bounce_packet;
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
        mpls[0].valid                : ternary;
        ipv4.valid                   : ternary;
        ipv6.valid                   : ternary;
        inner_ipv4.valid             : ternary;
        inner_ipv6.valid             : ternary;
        ipv4.dstAddr                 : ternary;
        ipv6.dstAddr                 : ternary;
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

action input_properties(vrf, dir, flow_miss_action, flow_miss_idx,
                        ipsg_enable, dscp, l4_profile_idx, src_lport,
                        flow_miss_tm_oqueue, dst_lport) {
    modify_field(flow_lkp_metadata.lkp_vrf, vrf);
    modify_field(flow_lkp_metadata.lkp_dir, dir);
    modify_field(control_metadata.flow_miss_action, flow_miss_action);
    modify_field(control_metadata.flow_miss_tm_oqueue, flow_miss_tm_oqueue);
    modify_field(control_metadata.flow_miss_idx, flow_miss_idx);
    modify_field(control_metadata.ipsg_enable, ipsg_enable);
    modify_field(qos_metadata.dscp, dscp);
    modify_field(l4_metadata.profile_idx, l4_profile_idx);
    modify_field(control_metadata.src_lif, capri_intrinsic.lif);
    modify_field(control_metadata.src_lport, src_lport);
    modify_field(control_metadata.dst_lport, dst_lport);

    // write nic mode (table constant)
    modify_field(control_metadata.nic_mode, scratch_metadata.flag);
}

// this table will only be programmed for uplinks and not for southbound enics
@pragma stage 0
@pragma overflow_table input_properties
table input_properties_otcam {
    reads {
        entry_inactive.input_properties : ternary;
        capri_intrinsic.lif             : ternary;
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

// this table will only be programmed for uplinks and not for southbound enics
@pragma stage 0
@pragma hash_type 0
table input_properties {
    reads {
        entry_inactive.input_properties : exact;
        capri_intrinsic.lif             : exact;
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

// For the case where packets from VMs are coming with Micro-VLAN and
// packets going to Uplink are going out with User-VLAN, we will need
// to insert two entries into this table so that one entry with
// Micro-VLAN derives the input_properties and the other entry with
// User-VLAN will be used for dejavu check.
action input_properties_mac_vlan(src_lif, src_lif_check_en,
                                 vrf, dir, flow_miss_action,flow_miss_idx,
                                 ipsg_enable, dscp, l4_profile_idx, src_lport,
                                 flow_miss_tm_oqueue, dst_lport) {
    adjust_lkp_fields();

    // if table is a miss, return. do not perform rest of the actions.

    // dejavu check
    if ((src_lif_check_en == TRUE) and (src_lif != capri_intrinsic.lif)) {
        modify_field(control_metadata.drop_reason, DROP_INPUT_MAPPING_DEJAVU);
        drop_packet();
    }

    modify_field(control_metadata.src_lif, src_lif);
    input_properties(vrf, dir, flow_miss_action, flow_miss_idx,
                     ipsg_enable, dscp, l4_profile_idx, src_lport,
                     flow_miss_tm_oqueue, dst_lport);

    // dummy ops to keep compiler happy
    modify_field(scratch_metadata.src_lif_check_en, src_lif_check_en);
}

action adjust_lkp_fields() {
    if (capri_intrinsic.tm_iport == TM_PORT_DMA) {
        modify_field(flow_lkp_metadata.lkp_inst,
                     (p4plus_to_p4.flags & P4PLUS_TO_P4_FLAGS_LKP_INST));
        subtract(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size,
                 (CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_TXDMA_INTRINSIC_HDR_SZ +
                  P4PLUS_TO_P4_HDR_SZ));
    } else {
        subtract(scratch_metadata.packet_len, capri_p4_intrinsic.frame_size,
                 CAPRI_GLOBAL_INTRINSIC_HDR_SZ);
    }
    if (recirc_header.valid == TRUE) {
        modify_field(control_metadata.recirc_reason, recirc_header.reason);
        subtract(scratch_metadata.packet_len, scratch_metadata.packet_len,
                 P4_RECIRC_HDR_SZ);
    }

    modify_field(control_metadata.packet_len, scratch_metadata.packet_len);
    modify_field(capri_p4_intrinsic.packet_len, scratch_metadata.packet_len);
}

// this table will be programmed during enic-if create time frame only
// in pvlan mode, program (isolated-vlan, mac) and (user-vlan, mac)
// in useg mode, program (useg-vlan, mac) and (user-vlan, mac)
// NOTE: (user-vlan, mac) is only needed in end-host mode for dejavu
//       checks
// in direct-io mode, don't program any mode
@pragma stage 0
table input_properties_mac_vlan {
    reads {
        entry_inactive.input_mac_vlan : ternary;
        control_metadata.uplink       : ternary;
        vlan_tag.valid                : ternary;
        vlan_tag.vid                  : ternary;
        p4plus_to_p4.valid            : ternary;
        p4plus_to_p4.vlan_tag         : ternary;
        ethernet.srcAddr              : ternary;
    }
    actions {
        input_properties_mac_vlan;
    }
    size : INPUT_PROPERTIES_MAC_VLAN_TABLE_SIZE;
}

control process_input_mapping {
    apply(input_mapping_tunneled);
    apply(input_mapping_native);
    apply(input_properties);
    apply(input_properties_otcam);
    apply(input_properties_mac_vlan);
}
