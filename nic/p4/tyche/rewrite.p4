/*****************************************************************************/
/* Common actions                                                            */
/*****************************************************************************/
action rewrite_layer_0(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & REWRITE_PUSH_ETHERNET_0) != 0) {
        add_header(ethernet_0);
        modify_field(ethernet_0.dstAddr, ethernet_dst);
        modify_field(ethernet_0.srcAddr, ethernet_src);
        modify_field(ethernet_0.etherType, ethernet_type);
    }

    if ((hdr_bits & REWRITE_PUSH_IPV4_0) != 0) {
        add_header(ipv4_0);
        modify_field(ipv4_0.srcAddr, ip_src);
        modify_field(ipv4_0.dstAddr, ip_dst);
        modify_field(ipv4_0.diffserv, ip_dscp);
        modify_field(ipv4_0.totalLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & REWRITE_ENCAP_LEN_MASK_0) >>
                      REWRITE_ENCAP_LEN_SHIFT_0));
        modify_field(ipv4_0.protocol, ip_proto);
        modify_field(ipv4_0.ttl, ip_ttl);
    }

    if ((hdr_bits & REWRITE_PUSH_IPV6_0) != 0) {
        add_header(ipv6_0);
        modify_field(ipv6_0.srcAddr, ip_src);
        modify_field(ipv6_0.dstAddr, ip_dst);
        modify_field(ipv6_0.trafficClass, ip_dscp);
        modify_field(ipv6_0.payloadLen, capri_p4_intrinsic.packet_len +
                     ((hdr_bits & REWRITE_ENCAP_LEN_MASK_0) >>
                      REWRITE_ENCAP_LEN_SHIFT_0));
        modify_field(ipv6_0.nextHdr, ip_proto);
        modify_field(ipv6_0.hopLimit, ip_ttl);
    }

    if ((hdr_bits & REWRITE_PUSH_CTAG_0) != 0) {
        modify_field(ethernet_0.etherType, ETHERTYPE_CTAG);
        add_header(ctag_0);
        modify_field(ctag_0.vid, ctag);
        modify_field(ctag_0.etherType, ethernet_type);
    }
}

action rewrite_layer_1(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & REWRITE_POP_ETHERNET) != 0) {
        remove_header(ethernet_1);
    }
    if ((hdr_bits & REWRITE_POP_CTAG) != 0) {
        remove_header(ctag_1);
    }
    if ((hdr_bits & REWRITE_POP_IPV4) != 0) {
        remove_header(ipv4_1);
    }
    if ((hdr_bits & REWRITE_POP_IPV6) != 0) {
        remove_header(ipv6_1);
    }

    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_SRC) != 0) {
        modify_field(ethernet_1.srcAddr, ethernet_src);
    }
    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_DST) != 0) {
        modify_field(ethernet_1.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_TYPE) != 0) {
        modify_field(ethernet_1.etherType, ethernet_type);
    }
    if ((hdr_bits & REWRITE_MODIFY_CTAG) != 0) {
        modify_field(ethernet_1.etherType, ETHERTYPE_CTAG);
        add_header(ctag_1);
        modify_field(ctag_1.vid, ctag);
        modify_field(ctag_1.etherType, ethernet_type);
    }

    if (ipv4_1.valid == TRUE) {
        if ((hdr_bits & REWRITE_MODIFY_IP_SRC) != 0) {
            modify_field(ipv4_1.srcAddr, ip_src);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DST) != 0) {
            modify_field(ipv4_1.dstAddr, ip_dst);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DSCP) != 0) {
            modify_field(ipv4_1.diffserv, ip_dscp);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_PROTO) != 0) {
            modify_field(ipv4_1.protocol, ip_proto);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_TTL) != 0) {
            modify_field(ipv4_1.ttl, ip_ttl);
        }
    }

    if (ipv6_1.valid == TRUE) {
        if ((hdr_bits & REWRITE_MODIFY_IP_SRC) != 0) {
            modify_field(ipv6_1.srcAddr, ip_src);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DST) != 0) {
            modify_field(ipv6_1.dstAddr, ip_dst);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DSCP) != 0) {
            modify_field(ipv6_1.trafficClass, ip_dscp);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_PROTO) != 0) {
            modify_field(ipv6_1.nextHdr, ip_proto);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_TTL) != 0) {
            modify_field(ipv6_1.hopLimit, ip_ttl);
        }
    }
}

action rewrite_layer_2(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto) {
    if ((hdr_bits & REWRITE_POP_ETHERNET) != 0) {
        remove_header(ethernet_2);
    }
    if ((hdr_bits & REWRITE_POP_CTAG) != 0) {
        remove_header(ctag_2);
    }
    if ((hdr_bits & REWRITE_POP_IPV4) != 0) {
        remove_header(ipv4_2);
    }
    if ((hdr_bits & REWRITE_POP_IPV6) != 0) {
        remove_header(ipv6_2);
    }

    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_SRC) != 0) {
        modify_field(ethernet_2.srcAddr, ethernet_src);
    }
    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_DST) != 0) {
        modify_field(ethernet_2.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & REWRITE_MODIFY_ETHERNET_TYPE) != 0) {
        modify_field(ethernet_2.etherType, ethernet_type);
    }
    if ((hdr_bits & REWRITE_MODIFY_CTAG) != 0) {
        modify_field(ethernet_2.etherType, ETHERTYPE_CTAG);
        add_header(ctag_2);
        modify_field(ctag_2.vid, ctag);
        modify_field(ctag_2.etherType, ethernet_type);
    }

    if (ipv4_2.valid == TRUE) {
        if ((hdr_bits & REWRITE_MODIFY_IP_SRC) != 0) {
            modify_field(ipv4_2.srcAddr, ip_src);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DST) != 0) {
            modify_field(ipv4_2.dstAddr, ip_dst);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DSCP) != 0) {
            modify_field(ipv4_2.diffserv, ip_dscp);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_PROTO) != 0) {
            modify_field(ipv4_2.protocol, ip_proto);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_TTL) != 0) {
            modify_field(ipv4_2.ttl, ip_ttl);
        }
    }

    if (ipv6_2.valid == TRUE) {
        if ((hdr_bits & REWRITE_MODIFY_IP_SRC) != 0) {
            modify_field(ipv6_2.srcAddr, ip_src);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DST) != 0) {
            modify_field(ipv6_2.dstAddr, ip_dst);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_DSCP) != 0) {
            modify_field(ipv6_2.trafficClass, ip_dscp);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_PROTO) != 0) {
            modify_field(ipv6_2.nextHdr, ip_proto);
        }
        if ((hdr_bits & REWRITE_MODIFY_IP_TTL) != 0) {
            modify_field(ipv6_2.hopLimit, ip_ttl);
        }
    }
}

action l4_rewrite_layer_0(hdr_bits, encap_len, tenant_id, l4_sport, l4_dport,
                          gre_proto, mpls0_valid, mpls1_valid, mpls2_valid,
                          mpls0_label, mpls0_exp, mpls0_bos, mpls0_ttl,
                          mpls1_label, mpls1_exp, mpls1_bos, mpls1_ttl,
                          mpls2_label, mpls2_exp, mpls2_bos, mpls2_ttl) {
    if ((hdr_bits & REWRITE_PUSH_VXLAN_0) != 0) {
        add_header(vxlan_0);
        modify_field(vxlan_0.flags, 0x8);
        modify_field(vxlan_0.vni, tenant_id);
    }
    if ((hdr_bits & REWRITE_PUSH_UDP_0) != 0) {
        add_header(udp_0);
        modify_field(udp_0.srcPort, l4_sport);
        modify_field(udp_0.dstPort, l4_dport);
        modify_field(udp_0.len, capri_p4_intrinsic.packet_len + encap_len);
    }
    if ((hdr_bits & REWRITE_PUSH_GRE_0) != 0) {
        add_header(gre_0);
        modify_field(gre_0.proto, gre_proto);
    }
    if ((hdr_bits & REWRITE_PUSH_MPLS_0) != 0) {
        if (mpls0_valid == TRUE) {
            add_header(mpls[0]);
            modify_field(mpls[0].label, mpls0_label);
            modify_field(mpls[0].exp, mpls0_exp);
            modify_field(mpls[0].bos, mpls0_bos);
            modify_field(mpls[0].ttl, mpls0_ttl);
        }
        if (mpls1_valid == TRUE) {
            add_header(mpls[1]);
            modify_field(mpls[1].label, mpls1_label);
            modify_field(mpls[1].exp, mpls1_exp);
            modify_field(mpls[1].bos, mpls1_bos);
            modify_field(mpls[1].ttl, mpls1_ttl);
        }
        if (mpls2_valid == TRUE) {
            add_header(mpls[2]);
            modify_field(mpls[2].label, mpls2_label);
            modify_field(mpls[2].exp, mpls2_exp);
            modify_field(mpls[2].bos, mpls2_bos);
            modify_field(mpls[2].ttl, mpls2_ttl);
        }
    }
}

action l4_rewrite_layer_1(hdr_bits, l4_sport, l4_dport) {
    if ((hdr_bits & REWRITE_POP_VXLAN_1) != 0) {
        remove_header(vxlan_1);
    }
    if ((hdr_bits & REWRITE_POP_UDP_1) != 0) {
        remove_header(udp_1);
    }
    if ((hdr_bits & REWRITE_POP_GRE_1) != 0) {
        remove_header(gre_1);
    }
    if ((hdr_bits & REWRITE_POP_MPLS_1) != 0) {
        remove_header(mpls[0]);
        remove_header(mpls[1]);
        remove_header(mpls[2]);
    }

    if ((hdr_bits & REWRITE_MODIFY_UDP_SPORT_1) != 0) {
        modify_field(udp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_UDP_DPORT_1) != 0) {
        modify_field(udp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_MODIFY_TCP_SPORT_1) != 0) {
        modify_field(tcp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_TCP_DPORT_1) != 0) {
        modify_field(tcp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_MODIFY_ICMP_TYPE_1) != 0) {
        modify_field(icmp_1.icmp_type, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_ICMP_CODE_1) != 0) {
        modify_field(icmp_1.icmp_code, l4_dport);
    }
}

action l4_rewrite_layer_2(hdr_bits, l4_sport, l4_dport) {
    if ((hdr_bits & REWRITE_MODIFY_UDP_SPORT_2) != 0) {
        modify_field(udp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_UDP_DPORT_2) != 0) {
        modify_field(udp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_MODIFY_TCP_SPORT_2) != 0) {
        modify_field(tcp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_TCP_DPORT_2) != 0) {
        modify_field(tcp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_MODIFY_ICMP_TYPE_2) != 0) {
        modify_field(icmp_2.icmp_type, l4_sport);
    }
    if ((hdr_bits & REWRITE_MODIFY_ICMP_CODE_2) != 0) {
        modify_field(icmp_2.icmp_code, l4_dport);
    }
}

/*****************************************************************************/
/* Rx pipeline                                                               */
/*****************************************************************************/
action rx_rewrite(hdr1_bits, hdr2_bits, ethernet_src, ethernet_dst,
                  ethernet_type, ctag, ip_src, ip_dst, ip_dscp, ip_ttl,
                  ip_proto) {
    rewrite_layer_1(hdr1_bits, ethernet_src, ethernet_dst, ethernet_type,
                    ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto);
    rewrite_layer_2(hdr2_bits, ethernet_src, ethernet_dst, ethernet_type,
                    ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto);

    modify_field(scratch_metadata.hdr_bits, hdr1_bits);
    modify_field(scratch_metadata.hdr_bits, hdr2_bits);
}

action rx_l4_rewrite(hdr_bits, l4_sport_1, l4_dport_1, l4_sport_2, l4_dport_2,
                     in_packets, in_bytes) {
    l4_rewrite_layer_1(hdr_bits, l4_sport_1, l4_dport_1);
    l4_rewrite_layer_2(hdr_bits, l4_sport_2, l4_dport_2);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
    modify_field(scratch_metadata.in_packets, in_packets);
    modify_field(scratch_metadata.in_bytes, in_bytes);
}

@pragma stage 5
@pragma hbm_table
table rx_rewrite0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table rx_rewrite1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table rx_rewrite2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rx_l4_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

control rx_rewrites {
    apply(rx_rewrite0);
    apply(rx_rewrite1);
    apply(rx_rewrite2);
}

/*****************************************************************************/
/* Tx pipeline                                                               */
/*****************************************************************************/
action tx_rewrite(hdr0_bits, hdr1_bits, ethernet_dst, ethernet_src,
                  ethernet_type, ctag, ip_src, ip_dst,
                  ip_dscp, ip_ttl, ip_proto) {
    rewrite_layer_0(hdr0_bits, ethernet_dst, ethernet_src, ethernet_type,
                    ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto);
    rewrite_layer_1(hdr1_bits, ethernet_dst, ethernet_src, ethernet_type,
                    ctag, ip_src, ip_dst, ip_dscp, ip_ttl, ip_proto);
    modify_field(scratch_metadata.hdr_bits, hdr0_bits);
    modify_field(scratch_metadata.hdr_bits, hdr1_bits);
}

action tx_l4_rewrite(hdr0_bits, hdr1_bits, encap_len_0, tenant_id_0, l4_sport_0,
                     l4_dport_0, l4_sport_1, l4_dport_1, gre_proto_0,
                     mpls0_valid, mpls1_valid, mpls2_valid,
                     mpls0_label, mpls0_exp, mpls0_bos, mpls0_ttl,
                     mpls1_label, mpls1_exp, mpls1_bos, mpls1_ttl,
                     mpls2_label, mpls2_exp, mpls2_bos, mpls2_ttl,
                     in_pkts, in_bytes) {
    l4_rewrite_layer_0(hdr0_bits, encap_len_0, tenant_id_0,
                       l4_sport_0, l4_dport_0, gre_proto_0,
                       mpls0_valid, mpls1_valid, mpls2_valid,
                       mpls0_label, mpls0_exp, mpls0_bos, mpls0_ttl,
                       mpls1_label, mpls1_exp, mpls1_bos, mpls1_ttl,
                       mpls2_label, mpls2_exp, mpls2_bos, mpls2_ttl);
    l4_rewrite_layer_1(hdr1_bits, l4_sport_1, l4_dport_1);

    modify_field(scratch_metadata.hdr_bits, hdr0_bits);
    modify_field(scratch_metadata.hdr_bits, hdr1_bits);
    modify_field(scratch_metadata.flag, mpls0_valid);
    modify_field(scratch_metadata.flag, mpls1_valid);
    modify_field(scratch_metadata.flag, mpls2_valid);
    modify_field(scratch_metadata.in_packets, in_pkts + 1);
    modify_field(scratch_metadata.in_bytes,
                 in_bytes + capri_p4_intrinsic.packet_len);
}
@pragma stage 5
@pragma hbm_table
table tx_rewrite0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table tx_rewrite1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table tx_rewrite2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        tx_l4_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

control tx_rewrites {
    apply(tx_rewrite0);
    apply(tx_rewrite1);
    apply(tx_rewrite2);
}
