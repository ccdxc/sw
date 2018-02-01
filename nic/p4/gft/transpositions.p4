/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
action hdr_transpositions_layer_00(hdr_bits, ethernet_dst, ethernet_src,
                                   ethernet_type, ctag, ip_src, ip_dst,
                                   ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_00) != 0) {
        add_header(ethernet_00);
        modify_field(ethernet_00.dstAddr, ethernet_dst);
        modify_field(ethernet_00.srcAddr, ethernet_src);
        modify_field(ethernet_00.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_00) != 0) {
        add_header(ipv4_00);
        modify_field(ipv4_00.srcAddr, ip_src);
        modify_field(ipv4_00.dstAddr, ip_dst);
        modify_field(ipv4_00.diffserv, ip_dscp);
        modify_field(ipv4_00.totalLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & TRANSPOSITIONS_ENCAP_LEN_MASK_00) >>
                      TRANSPOSITIONS_ENCAP_LEN_SHIFT_00));
        modify_field(ipv4_00.protocol, ip_proto);
        modify_field(ipv4_00.ttl, ip_ttl);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_00) != 0) {
        add_header(ipv6_00);
        modify_field(ipv6_00.srcAddr, ip_src);
        modify_field(ipv6_00.dstAddr, ip_dst);
        modify_field(ipv6_00.trafficClass, ip_dscp);
        modify_field(ipv6_00.payloadLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & TRANSPOSITIONS_ENCAP_LEN_MASK_00) >>
                      TRANSPOSITIONS_ENCAP_LEN_SHIFT_00));
        modify_field(ipv6_00.nextHdr, ip_proto);
        modify_field(ipv6_00.hopLimit, ip_ttl);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_CTAG_00) != 0) {
        modify_field(ethernet_00.etherType, ETHERTYPE_CTAG);
        add_header(ctag_00);
        modify_field(ctag_00.vid, ctag);
        modify_field(ctag_00.etherType, ethernet_type);
    }
}

action hdr_transpositions_layer_01(hdr_bits, ethernet_dst, ethernet_src,
                                   ethernet_type, ctag, ip_src, ip_dst,
                                   ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_01) != 0) {
        add_header(ethernet_01);
        modify_field(ethernet_01.dstAddr, ethernet_dst);
        modify_field(ethernet_01.srcAddr, ethernet_src);
        modify_field(ethernet_01.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_01) != 0) {
        add_header(ipv4_01);
        modify_field(ipv4_01.srcAddr, ip_src);
        modify_field(ipv4_01.dstAddr, ip_dst);
        modify_field(ipv4_01.diffserv, ip_dscp);
        modify_field(ipv4_01.totalLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & TRANSPOSITIONS_ENCAP_LEN_MASK_01) >>
                      TRANSPOSITIONS_ENCAP_LEN_SHIFT_01));
        modify_field(ipv4_01.protocol, ip_proto);
        modify_field(ipv4_01.ttl, ip_ttl);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_01) != 0) {
        add_header(ipv6_01);
        modify_field(ipv6_01.srcAddr, ip_src);
        modify_field(ipv6_01.dstAddr, ip_dst);
        modify_field(ipv6_01.trafficClass, ip_dscp);
        modify_field(ipv6_01.payloadLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & TRANSPOSITIONS_ENCAP_LEN_MASK_01) >>
                      TRANSPOSITIONS_ENCAP_LEN_SHIFT_01));
        modify_field(ipv6_01.nextHdr, ip_proto);
        modify_field(ipv6_01.hopLimit, ip_ttl);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_CTAG_01) != 0) {
        modify_field(ethernet_01.etherType, ETHERTYPE_CTAG);
        add_header(ctag_01);
        modify_field(ctag_01.vid, ctag);
        modify_field(ctag_01.etherType, ethernet_type);
    }
}

action hdr_transpositions_layer_1(hdr_bits, ethernet_dst, ethernet_src,
                                  ethernet_type, ctag, ip_src, ip_dst,
                                  ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET) != 0) {
        remove_header(ethernet_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_CTAG) != 0) {
        remove_header(ctag_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4) != 0) {
        remove_header(ipv4_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6) != 0) {
        remove_header(ipv6_1);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC) != 0) {
        modify_field(ethernet_1.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST) != 0) {
        modify_field(ethernet_1.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE) != 0) {
        modify_field(ethernet_1.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG) != 0) {
        modify_field(ethernet_1.etherType, ETHERTYPE_CTAG);
        add_header(ctag_1);
        modify_field(ctag_1.vid, ctag);
        modify_field(ctag_1.etherType, ethernet_type);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC) != 0) {
        modify_field(ipv4_1.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST) != 0) {
        modify_field(ipv4_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP) != 0) {
        modify_field(ipv4_1.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO) != 0) {
        modify_field(ipv4_1.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL) != 0) {
        modify_field(ipv4_1.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC) != 0) {
        modify_field(ipv6_1.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST) != 0) {
        modify_field(ipv6_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP) != 0) {
        modify_field(ipv6_1.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO) != 0) {
        modify_field(ipv6_1.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL) != 0) {
        modify_field(ipv6_1.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_2(hdr_bits, ethernet_dst, ethernet_src,
                                  ethernet_type, ctag, ip_src, ip_dst,
                                  ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET) != 0) {
        remove_header(ethernet_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_CTAG) != 0) {
        remove_header(ctag_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4) != 0) {
        remove_header(ipv4_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6) != 0) {
        remove_header(ipv6_2);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC) != 0) {
        modify_field(ethernet_2.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST) != 0) {
        modify_field(ethernet_2.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE) != 0) {
        modify_field(ethernet_2.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG) != 0) {
        modify_field(ethernet_2.etherType, ETHERTYPE_CTAG);
        add_header(ctag_2);
        modify_field(ctag_2.vid, ctag);
        modify_field(ctag_2.etherType, ethernet_type);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC) != 0) {
        modify_field(ipv4_2.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST) != 0) {
        modify_field(ipv4_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP) != 0) {
        modify_field(ipv4_2.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO) != 0) {
        modify_field(ipv4_2.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL) != 0) {
        modify_field(ipv4_2.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC) != 0) {
        modify_field(ipv6_2.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST) != 0) {
        modify_field(ipv6_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP) != 0) {
        modify_field(ipv6_2.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO) != 0) {
        modify_field(ipv6_2.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL) != 0) {
        modify_field(ipv6_2.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_3(hdr_bits, ethernet_dst, ethernet_src,
                                  ethernet_type, ctag, ip_src, ip_dst,
                                  ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET) != 0) {
        remove_header(ethernet_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_CTAG) != 0) {
        remove_header(ctag_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4) != 0) {
        remove_header(ipv4_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6) != 0) {
        remove_header(ipv6_3);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC) != 0) {
        modify_field(ethernet_3.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST) != 0) {
        modify_field(ethernet_3.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE) != 0) {
        modify_field(ethernet_3.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG) != 0) {
        modify_field(ethernet_3.etherType, ETHERTYPE_CTAG);
        add_header(ctag_3);
        modify_field(ctag_3.vid, ctag);
        modify_field(ctag_3.etherType, ethernet_type);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC) != 0) {
        modify_field(ipv4_3.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST) != 0) {
        modify_field(ipv4_3.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP) != 0) {
        modify_field(ipv4_3.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO) != 0) {
        modify_field(ipv4_3.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL) != 0) {
        modify_field(ipv4_3.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC) != 0) {
        modify_field(ipv6_3.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST) != 0) {
        modify_field(ipv6_3.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP) != 0) {
        modify_field(ipv6_3.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO) != 0) {
        modify_field(ipv6_3.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL) != 0) {
        modify_field(ipv6_3.hopLimit, ip_ttl);
    }
}

action rx_hdr_transpositions(hdr1_bits, hdr2_bits, hdr3_bits,
                             ethernet_dst, ethernet_src, ethernet_type,
                             ctag, ip_src, ip_dst, ip_dscp, ip_ttl,
                             ip_proto) {
    hdr_transpositions_layer_1(hdr1_bits, ethernet_dst, ethernet_src,
                               ethernet_type, ctag, ip_src, ip_dst,
                               ip_dscp, ip_ttl, ip_proto);
    hdr_transpositions_layer_2(hdr2_bits, ethernet_dst, ethernet_src,
                               ethernet_type, ctag, ip_src, ip_dst,
                               ip_dscp, ip_ttl, ip_proto);
    hdr_transpositions_layer_3(hdr3_bits, ethernet_dst, ethernet_src,
                               ethernet_type, ctag, ip_src, ip_dst,
                               ip_dscp, ip_ttl, ip_proto);

    modify_field(scratch_metadata.hdr_bits, hdr1_bits);
    modify_field(scratch_metadata.hdr_bits, hdr2_bits);
    modify_field(scratch_metadata.hdr_bits, hdr3_bits);
}

action l4_hdr_transpositions_layer_00(hdr_bits, encap_len, tenant_id,
                                      l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_00) != 0) {
        add_header(vxlan_00);
        modify_field(vxlan_00.flags, 0x8);
        modify_field(vxlan_00.vni, tenant_id);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_00) != 0) {
        add_header(udp_00);
        modify_field(udp_00.srcPort, l4_sport);
        modify_field(udp_00.dstPort, l4_dport);
        modify_field(udp_00.len, capri_p4_intrinsic.packet_len + encap_len);
    }
}

action l4_hdr_transpositions_layer_01(hdr_bits, encap_len, tenant_id,
                                      l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_01) != 0) {
        add_header(vxlan_01);
        modify_field(vxlan_01.flags, 0x8);
        modify_field(vxlan_01.vni, tenant_id);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_01) != 0) {
        add_header(udp_01);
        modify_field(udp_01.srcPort, l4_sport);
        modify_field(udp_01.dstPort, l4_dport);
        modify_field(udp_01.len, capri_p4_intrinsic.packet_len + encap_len);
    }
}

action l4_hdr_transpositions_layer_1(hdr_bits, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_POP_VXLAN_1) != 0) {
        remove_header(vxlan_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_UDP_1) != 0) {
        remove_header(udp_1);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_SPORT_1) != 0) {
        modify_field(udp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_DPORT_1) != 0) {
        modify_field(udp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_SPORT_1) != 0) {
        modify_field(tcp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_DPORT_1) != 0) {
        modify_field(tcp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_TYPE_1) != 0) {
        modify_field(icmp_1.icmp_type, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_1) != 0) {
        modify_field(icmp_1.icmp_code, l4_dport);
    }
}

action l4_hdr_transpositions_layer_2(hdr_bits, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_POP_VXLAN_2) != 0) {
        remove_header(vxlan_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_UDP_2) != 0) {
        remove_header(udp_2);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_SPORT_2) != 0) {
        modify_field(udp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_DPORT_2) != 0) {
        modify_field(udp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_SPORT_2) != 0) {
        modify_field(tcp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_DPORT_2) != 0) {
        modify_field(tcp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_TYPE_2) != 0) {
        modify_field(icmp_2.icmp_type, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_2) != 0) {
        modify_field(icmp_2.icmp_code, l4_dport);
    }
}

action l4_hdr_transpositions_layer_3(hdr_bits, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_POP_VXLAN_3) != 0) {
        remove_header(vxlan_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_UDP_3) != 0) {
        remove_header(udp_3);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_SPORT_3) != 0) {
        modify_field(udp_3.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_DPORT_3) != 0) {
        modify_field(udp_3.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_SPORT_3) != 0) {
        modify_field(tcp_3.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_DPORT_3) != 0) {
        modify_field(tcp_3.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_TYPE_3) != 0) {
        modify_field(icmp_3.icmp_type, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_3) != 0) {
        modify_field(icmp_3.icmp_code, l4_dport);
    }
}

action rx_l4_hdr_transpositions(hdr_bits, l4_sport_1, l4_dport_1, l4_sport_2,
                                l4_dport_2, l4_sport_3, l4_dport_3,
                                in_pkts, in_bytes) {
    l4_hdr_transpositions_layer_1(hdr_bits, l4_sport_1, l4_dport_1);
    l4_hdr_transpositions_layer_2(hdr_bits, l4_sport_2, l4_dport_2);
    l4_hdr_transpositions_layer_3(hdr_bits, l4_sport_3, l4_dport_3);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
    modify_field(scratch_metadata.num_packets, in_pkts + 1);
    modify_field(scratch_metadata.num_bytes,
                 in_bytes + capri_p4_intrinsic.packet_len);
}

@pragma stage 4
@pragma hbm_table
table rx_hdr_transpositions0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
table rx_hdr_transpositions1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
table rx_hdr_transpositions2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 4
@pragma hbm_table
table rx_hdr_transpositions3 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_l4_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

control rx_transpositions {
    apply(rx_hdr_transpositions0);
    apply(rx_hdr_transpositions1);
    apply(rx_hdr_transpositions2);
    apply(rx_hdr_transpositions3);
}

/******************************************************************************/
/* Tx pipeline                                                                */
/******************************************************************************/
action tx_hdr_transpositions(hdr0_bits, hdr1_bits, ethernet_dst, ethernet_src,
                             ethernet_type, ctag, ip_src, ip_dst,
                             ip_dscp, ip_ttl, ip_proto) {
    hdr_transpositions_layer_00(hdr0_bits, ethernet_dst, ethernet_src,
                                ethernet_type, ctag, ip_src, ip_dst,
                                ip_dscp, ip_ttl, ip_proto);
    hdr_transpositions_layer_01(hdr0_bits, ethernet_dst, ethernet_src,
                                ethernet_type, ctag, ip_src, ip_dst,
                                ip_dscp, ip_ttl, ip_proto);
    hdr_transpositions_layer_1(hdr1_bits, ethernet_dst, ethernet_src,
                               ethernet_type, ctag, ip_src, ip_dst,
                               ip_dscp, ip_ttl, ip_proto);
    modify_field(scratch_metadata.hdr_bits, hdr0_bits);
    modify_field(scratch_metadata.hdr_bits, hdr1_bits);
}

action tx_l4_hdr_transpositions(hdr0_bits, hdr_bits, encap_len_00,
                                tenant_id_00, l4_sport_00, l4_dport_00,
                                encap_len_01, tenant_id_01, l4_sport_01,
                                l4_dport_01, l4_sport_1, l4_dport_1,
                                in_pkts, in_bytes) {
    l4_hdr_transpositions_layer_00(hdr0_bits, encap_len_00, tenant_id_00,
                                   l4_sport_00, l4_dport_00);
    l4_hdr_transpositions_layer_01(hdr0_bits, encap_len_01, tenant_id_01,
                                   l4_sport_01, l4_dport_01);
    l4_hdr_transpositions_layer_1(hdr_bits, l4_sport_1, l4_dport_1);

    modify_field(scratch_metadata.hdr_bits, hdr0_bits);
    modify_field(scratch_metadata.hdr_bits, hdr_bits);
    modify_field(scratch_metadata.num_packets, in_pkts + 1);
    modify_field(scratch_metadata.num_bytes,
                 in_bytes + capri_p4_intrinsic.packet_len);
}

@pragma stage 3
@pragma hbm_table
table tx_hdr_transpositions0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
table tx_hdr_transpositions1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
table tx_hdr_transpositions2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 3
@pragma hbm_table
table tx_hdr_transpositions3 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_l4_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

control tx_transpositions {
    apply(tx_hdr_transpositions0);
    apply(tx_hdr_transpositions1);
    apply(tx_hdr_transpositions2);
    apply(tx_hdr_transpositions3);
}
