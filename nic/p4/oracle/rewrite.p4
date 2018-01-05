/*****************************************************************************/
/* Common actions                                                            */
/*****************************************************************************/
action rewrite_layer_0(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_ETHERNET_0) != 0) {
        add_header(ethernet_0);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV4_0) != 0) {
        add_header(ipv4_0);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV6_0) != 0) {
        add_header(ipv6_0);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_ETHERNET_0) != 0) {
        remove_header(ethernet_0);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV4_0) != 0) {
        remove_header(ipv4_0);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV6_0) != 0) {
        remove_header(ipv6_0);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_SRC_0) != 0) {
        modify_field(ethernet_0.srcAddr, ethernet_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_DST_0) != 0) {
        modify_field(ethernet_0.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_TYPE_0) != 0) {
        modify_field(ethernet_0.etherType, ethernet_type);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_CTAG_0) != 0) {
        modify_field(ctag_0.vid, ctag);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_STAG_0) != 0) {
        modify_field(stag_0.vid, stag);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_SRC_0) != 0) {
        modify_field(ipv4_0.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DST_0) != 0) {
        modify_field(ipv4_0.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DSCP_0) != 0) {
        modify_field(ipv4_0.diffserv, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_PROTO_0) != 0) {
        modify_field(ipv4_0.protocol, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_TTL_0) != 0) {
        modify_field(ipv4_0.ttl, ip_ttl);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_SRC_0) != 0) {
        modify_field(ipv6_0.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DST_0) != 0) {
        modify_field(ipv6_0.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DSCP_0) != 0) {
        modify_field(ipv6_0.trafficClass, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_PROTO_0) != 0) {
        modify_field(ipv6_0.nextHdr, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_TTL_0) != 0) {
        modify_field(ipv6_0.hopLimit, ip_ttl);
    }
}

action rewrite_layer_1(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_ETHERNET_1) != 0) {
        add_header(ethernet_1);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV4_1) != 0) {
        add_header(ipv4_1);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV6_1) != 0) {
        add_header(ipv6_1);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_ETHERNET_1) != 0) {
        remove_header(ethernet_1);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV4_1) != 0) {
        remove_header(ipv4_1);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV6_1) != 0) {
        remove_header(ipv6_1);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_SRC_1) != 0) {
        modify_field(ethernet_1.srcAddr, ethernet_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_DST_1) != 0) {
        modify_field(ethernet_1.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_TYPE_1) != 0) {
        modify_field(ethernet_1.etherType, ethernet_type);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_CTAG_1) != 0) {
        modify_field(ctag_1.vid, ctag);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_STAG_1) != 0) {
        modify_field(stag_1.vid, stag);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_SRC_1) != 0) {
        modify_field(ipv4_1.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DST_1) != 0) {
        modify_field(ipv4_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DSCP_1) != 0) {
        modify_field(ipv4_1.diffserv, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_PROTO_1) != 0) {
        modify_field(ipv4_1.protocol, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_TTL_1) != 0) {
        modify_field(ipv4_1.ttl, ip_ttl);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_SRC_1) != 0) {
        modify_field(ipv6_1.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DST_1) != 0) {
        modify_field(ipv6_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DSCP_1) != 0) {
        modify_field(ipv6_1.trafficClass, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_PROTO_1) != 0) {
        modify_field(ipv6_1.nextHdr, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_TTL_1) != 0) {
        modify_field(ipv6_1.hopLimit, ip_ttl);
    }
}

action rewrite_layer_2(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                       ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_ETHERNET_2) != 0) {
        add_header(ethernet_2);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV4_2) != 0) {
        add_header(ipv4_2);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_IPV6_2) != 0) {
        add_header(ipv6_2);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_ETHERNET_2) != 0) {
        remove_header(ethernet_2);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV4_2) != 0) {
        remove_header(ipv4_2);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_IPV6_2) != 0) {
        remove_header(ipv6_2);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_SRC_2) != 0) {
        modify_field(ethernet_2.srcAddr, ethernet_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_DST_2) != 0) {
        modify_field(ethernet_2.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ETHERNET_TYPE_2) != 0) {
        modify_field(ethernet_2.etherType, ethernet_type);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_CTAG_2) != 0) {
        modify_field(ctag_2.vid, ctag);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_STAG_2) != 0) {
        modify_field(stag_2.vid, stag);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_SRC_2) != 0) {
        modify_field(ipv4_2.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DST_2) != 0) {
        modify_field(ipv4_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_DSCP_2) != 0) {
        modify_field(ipv4_2.diffserv, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_PROTO_2) != 0) {
        modify_field(ipv4_2.protocol, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV4_TTL_2) != 0) {
        modify_field(ipv4_2.ttl, ip_ttl);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_SRC_2) != 0) {
        modify_field(ipv6_2.srcAddr, ip_src);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DST_2) != 0) {
        modify_field(ipv6_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_DSCP_2) != 0) {
        modify_field(ipv6_2.trafficClass, ip_dscp);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_PROTO_2) != 0) {
        modify_field(ipv6_2.nextHdr, ip_proto);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_IPV6_TTL_2) != 0) {
        modify_field(ipv6_2.hopLimit, ip_ttl);
    }
}

action rewrite(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
               ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl) {
    rewrite_layer_0(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                     ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl);
    rewrite_layer_1(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                    ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl);
    rewrite_layer_2(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                    ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto, ip_ttl);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
}

action l4_rewrite_layer_0(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_VXLAN_0) != 0) {
        add_header(vxlan_0);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_UDP_0) != 0) {
        add_header(udp_0);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_VXLAN_0) != 0) {
        remove_header(vxlan_0);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_UDP_0) != 0) {
        remove_header(udp_0);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_SPORT_0) != 0) {
        modify_field(udp_0.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_DPORT_0) != 0) {
        modify_field(udp_0.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_SPORT_0) != 0) {
        modify_field(tcp_0.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_DPORT_0) != 0) {
        modify_field(tcp_0.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_TYPE_0) != 0) {
        modify_field(icmp_0.icmp_type, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_0) != 0) {
        modify_field(icmp_0.icmp_code, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_0) != 0) {
        modify_field(icmp_0.icmp_code, l4_dport);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_TENANT_ID_0) != 0) {
        modify_field(vxlan_0.vni, tenant_id);
    }
}

action l4_rewrite_layer_1(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_VXLAN_1) != 0) {
        add_header(vxlan_1);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_UDP_1) != 0) {
        add_header(udp_1);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_VXLAN_1) != 0) {
        remove_header(vxlan_1);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_UDP_1) != 0) {
        remove_header(udp_1);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_SPORT_1) != 0) {
        modify_field(udp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_DPORT_1) != 0) {
        modify_field(udp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_SPORT_1) != 0) {
        modify_field(tcp_1.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_DPORT_1) != 0) {
        modify_field(tcp_1.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_TYPE_1) != 0) {
        modify_field(icmp_1.icmp_type, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_1) != 0) {
        modify_field(icmp_1.icmp_code, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_1) != 0) {
        modify_field(icmp_1.icmp_code, l4_dport);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_TENANT_ID_1) != 0) {
        modify_field(vxlan_1.vni, tenant_id);
    }
}

action l4_rewrite_layer_2(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & REWRITE_TYPE_PUSH_VXLAN_2) != 0) {
        add_header(vxlan_2);
    }
    if ((hdr_bits & REWRITE_TYPE_PUSH_UDP_2) != 0) {
        add_header(udp_2);
    }

    if ((hdr_bits & REWRITE_TYPE_POP_VXLAN_2) != 0) {
        remove_header(vxlan_2);
    }
    if ((hdr_bits & REWRITE_TYPE_POP_UDP_2) != 0) {
        remove_header(udp_2);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_SPORT_2) != 0) {
        modify_field(udp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_UDP_DPORT_2) != 0) {
        modify_field(udp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_SPORT_2) != 0) {
        modify_field(tcp_2.srcPort, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_TCP_DPORT_2) != 0) {
        modify_field(tcp_2.dstPort, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_TYPE_2) != 0) {
        modify_field(icmp_2.icmp_type, l4_sport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_2) != 0) {
        modify_field(icmp_2.icmp_code, l4_dport);
    }
    if ((hdr_bits & REWRITE_TYPE_MODIFY_ICMP_CODE_2) != 0) {
        modify_field(icmp_2.icmp_code, l4_dport);
    }

    if ((hdr_bits & REWRITE_TYPE_MODIFY_TENANT_ID_2) != 0) {
        modify_field(vxlan_2.vni, tenant_id);
    }
}

action l4_rewrite(hdr_bits, tenant_id, l4_sport, l4_dport,
                  in_packets, in_bytes) {
    l4_rewrite_layer_0(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_rewrite_layer_1(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_rewrite_layer_2(hdr_bits, tenant_id, l4_sport, l4_dport);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
    modify_field(scratch_metadata.in_packets, in_packets);
    modify_field(scratch_metadata.in_bytes, in_bytes);
}

/*****************************************************************************/
/* Ingress pipeline                                                          */
/*****************************************************************************/
@pragma stage 5
@pragma hbm_table
table ingress_rewrite0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table ingress_rewrite1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table ingress_rewrite2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        l4_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

control ingress_rewrites {
    apply(ingress_rewrite0);
    apply(ingress_rewrite1);
    apply(ingress_rewrite2);
}

/*****************************************************************************/
/* Egress pipeline                                                           */
/*****************************************************************************/
@pragma stage 5
@pragma hbm_table
table egress_rewrite0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table egress_rewrite1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table egress_rewrite2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        l4_rewrite;
    }
    size : REWRITE_TABLE_SIZE;
}

control egress_rewrites {
    apply(egress_rewrite0);
    apply(egress_rewrite1);
    apply(egress_rewrite2);
}
