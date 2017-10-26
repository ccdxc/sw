action hdr_transpositions_layer_00(hdr_bits, ethernet_src, ethernet_dst,
                                   ethernet_type, ctag, stag, ip_src, ip_dst,
                                   ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_00) != 0) {
        add_header(ethernet_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_00) != 0) {
        add_header(ipv4_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_00) != 0) {
        add_header(ipv6_00);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET_00) != 0) {
        remove_header(ethernet_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4_00) != 0) {
        remove_header(ipv4_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6_00) != 0) {
        remove_header(ipv6_00);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC_00) != 0) {
        modify_field(ethernet_00.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST_00) != 0) {
        modify_field(ethernet_00.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE_00) != 0) {
        modify_field(ethernet_00.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG_00) != 0) {
        modify_field(ctag_00.vid, ctag);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_STAG_00) != 0) {
        modify_field(stag_00.vid, stag);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC_00) != 0) {
        modify_field(ipv4_00.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST_00) != 0) {
        modify_field(ipv4_00.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP_00) != 0) {
        modify_field(ipv4_00.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO_00) != 0) {
        modify_field(ipv4_00.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL_00) != 0) {
        modify_field(ipv4_00.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC_00) != 0) {
        modify_field(ipv6_00.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST_00) != 0) {
        modify_field(ipv6_00.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP_00) != 0) {
        modify_field(ipv6_00.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO_00) != 0) {
        modify_field(ipv6_00.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL_00) != 0) {
        modify_field(ipv6_00.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_01(hdr_bits, ethernet_src, ethernet_dst,
                                   ethernet_type, ctag, stag, ip_src, ip_dst,
                                   ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_01) != 0) {
        add_header(ethernet_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_01) != 0) {
        add_header(ipv4_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_01) != 0) {
        add_header(ipv6_01);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET_01) != 0) {
        remove_header(ethernet_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4_01) != 0) {
        remove_header(ipv4_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6_01) != 0) {
        remove_header(ipv6_01);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC_01) != 0) {
        modify_field(ethernet_01.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST_01) != 0) {
        modify_field(ethernet_01.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE_01) != 0) {
        modify_field(ethernet_01.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG_01) != 0) {
        modify_field(ctag_01.vid, ctag);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_STAG_01) != 0) {
        modify_field(stag_01.vid, stag);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC_01) != 0) {
        modify_field(ipv4_01.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST_01) != 0) {
        modify_field(ipv4_01.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP_01) != 0) {
        modify_field(ipv4_01.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO_01) != 0) {
        modify_field(ipv4_01.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL_01) != 0) {
        modify_field(ipv4_01.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC_01) != 0) {
        modify_field(ipv6_01.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST_01) != 0) {
        modify_field(ipv6_01.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP_01) != 0) {
        modify_field(ipv6_01.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO_01) != 0) {
        modify_field(ipv6_01.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL_01) != 0) {
        modify_field(ipv6_01.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_1(hdr_bits, ethernet_src, ethernet_dst,
                                  ethernet_type, ctag, stag, ip_src, ip_dst,
                                  ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_1) != 0) {
        add_header(ethernet_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_1) != 0) {
        add_header(ipv4_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_1) != 0) {
        add_header(ipv6_1);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET_1) != 0) {
        remove_header(ethernet_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4_1) != 0) {
        remove_header(ipv4_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6_1) != 0) {
        remove_header(ipv6_1);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC_1) != 0) {
        modify_field(ethernet_1.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST_1) != 0) {
        modify_field(ethernet_1.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE_1) != 0) {
        modify_field(ethernet_1.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG_1) != 0) {
        modify_field(ctag_1.vid, ctag);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_STAG_1) != 0) {
        modify_field(stag_1.vid, stag);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC_1) != 0) {
        modify_field(ipv4_1.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST_1) != 0) {
        modify_field(ipv4_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP_1) != 0) {
        modify_field(ipv4_1.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO_1) != 0) {
        modify_field(ipv4_1.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL_1) != 0) {
        modify_field(ipv4_1.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC_1) != 0) {
        modify_field(ipv6_1.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST_1) != 0) {
        modify_field(ipv6_1.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP_1) != 0) {
        modify_field(ipv6_1.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO_1) != 0) {
        modify_field(ipv6_1.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL_1) != 0) {
        modify_field(ipv6_1.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_2(hdr_bits, ethernet_src, ethernet_dst,
                                  ethernet_type, ctag, stag, ip_src, ip_dst,
                                  ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_2) != 0) {
        add_header(ethernet_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_2) != 0) {
        add_header(ipv4_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_2) != 0) {
        add_header(ipv6_2);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET_2) != 0) {
        remove_header(ethernet_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4_2) != 0) {
        remove_header(ipv4_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6_2) != 0) {
        remove_header(ipv6_2);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC_2) != 0) {
        modify_field(ethernet_2.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST_2) != 0) {
        modify_field(ethernet_2.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE_2) != 0) {
        modify_field(ethernet_2.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG_2) != 0) {
        modify_field(ctag_2.vid, ctag);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_STAG_2) != 0) {
        modify_field(stag_2.vid, stag);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC_2) != 0) {
        modify_field(ipv4_2.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST_2) != 0) {
        modify_field(ipv4_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP_2) != 0) {
        modify_field(ipv4_2.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO_2) != 0) {
        modify_field(ipv4_2.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL_2) != 0) {
        modify_field(ipv4_2.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC_2) != 0) {
        modify_field(ipv6_2.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST_2) != 0) {
        modify_field(ipv6_2.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP_2) != 0) {
        modify_field(ipv6_2.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO_2) != 0) {
        modify_field(ipv6_2.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL_2) != 0) {
        modify_field(ipv6_2.hopLimit, ip_ttl);
    }
}

action hdr_transpositions_layer_3(hdr_bits, ethernet_src, ethernet_dst,
                                  ethernet_type, ctag, stag, ip_src, ip_dst,
                                  ip_dscp, ip_proto, ip_ttl) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_ETHERNET_3) != 0) {
        add_header(ethernet_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV4_3) != 0) {
        add_header(ipv4_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_IPV6_3) != 0) {
        add_header(ipv6_3);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_ETHERNET_3) != 0) {
        remove_header(ethernet_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV4_3) != 0) {
        remove_header(ipv4_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_IPV6_3) != 0) {
        remove_header(ipv6_3);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_SRC_3) != 0) {
        modify_field(ethernet_3.srcAddr, ethernet_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_DST_3) != 0) {
        modify_field(ethernet_3.dstAddr, ethernet_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ETHERNET_TYPE_3) != 0) {
        modify_field(ethernet_3.etherType, ethernet_type);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_CTAG_3) != 0) {
        modify_field(ctag_3.vid, ctag);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_STAG_3) != 0) {
        modify_field(stag_3.vid, stag);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_SRC_3) != 0) {
        modify_field(ipv4_3.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DST_3) != 0) {
        modify_field(ipv4_3.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_DSCP_3) != 0) {
        modify_field(ipv4_3.diffserv, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_PROTO_3) != 0) {
        modify_field(ipv4_3.protocol, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV4_TTL_3) != 0) {
        modify_field(ipv4_3.ttl, ip_ttl);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_SRC_3) != 0) {
        modify_field(ipv6_3.srcAddr, ip_src);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DST_3) != 0) {
        modify_field(ipv6_3.dstAddr, ip_dst);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_DSCP_3) != 0) {
        modify_field(ipv6_3.trafficClass, ip_dscp);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_PROTO_3) != 0) {
        modify_field(ipv6_3.nextHdr, ip_proto);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_IPV6_TTL_3) != 0) {
        modify_field(ipv6_3.hopLimit, ip_ttl);
    }
}

action hdr_transpositions(hdr_bits, ethernet_src, ethernet_dst, ethernet_type,
                          ctag, stag, ip_src, ip_dst, ip_dscp, ip_proto,
                          ip_ttl) {
    hdr_transpositions_layer_00(hdr_bits, ethernet_src, ethernet_dst,
                                ethernet_type, ctag, stag, ip_src, ip_dst,
                                ip_dscp, ip_proto, ip_ttl);
    hdr_transpositions_layer_01(hdr_bits, ethernet_src, ethernet_dst,
                                ethernet_type, ctag, stag, ip_src, ip_dst,
                                ip_dscp, ip_proto, ip_ttl);
    hdr_transpositions_layer_1(hdr_bits, ethernet_src, ethernet_dst,
                               ethernet_type, ctag, stag, ip_src, ip_dst,
                               ip_dscp, ip_proto, ip_ttl);
    hdr_transpositions_layer_2(hdr_bits, ethernet_src, ethernet_dst,
                               ethernet_type, ctag, stag, ip_src, ip_dst,
                               ip_dscp, ip_proto, ip_ttl);
    hdr_transpositions_layer_3(hdr_bits, ethernet_src, ethernet_dst,
                               ethernet_type, ctag, stag, ip_src, ip_dst,
                               ip_dscp, ip_proto, ip_ttl);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
}

action l4_hdr_transpositions_layer_00(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_00) != 0) {
        add_header(vxlan_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_00) != 0) {
        add_header(udp_00);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_VXLAN_00) != 0) {
        remove_header(vxlan_00);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_UDP_00) != 0) {
        remove_header(udp_00);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_SPORT_00) != 0) {
        modify_field(udp_00.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_DPORT_00) != 0) {
        modify_field(udp_00.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_SPORT_00) != 0) {
        modify_field(tcp_00.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_DPORT_00) != 0) {
        modify_field(tcp_00.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_TYPE_00) != 0) {
        modify_field(icmp_00.icmp_type, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_00) != 0) {
        modify_field(icmp_00.icmp_code, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_00) != 0) {
        modify_field(icmp_00.icmp_code, l4_dport);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TENANT_ID_00) != 0) {
        modify_field(vxlan_00.vni, tenant_id);
    }
}

action l4_hdr_transpositions_layer_01(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_01) != 0) {
        add_header(vxlan_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_01) != 0) {
        add_header(udp_01);
    }

    if ((hdr_bits & TRANSPOSITIONS_POP_VXLAN_01) != 0) {
        remove_header(vxlan_01);
    }
    if ((hdr_bits & TRANSPOSITIONS_POP_UDP_01) != 0) {
        remove_header(udp_01);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_SPORT_01) != 0) {
        modify_field(udp_01.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_UDP_DPORT_01) != 0) {
        modify_field(udp_01.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_SPORT_01) != 0) {
        modify_field(tcp_01.srcPort, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TCP_DPORT_01) != 0) {
        modify_field(tcp_01.dstPort, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_TYPE_01) != 0) {
        modify_field(icmp_01.icmp_type, l4_sport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_01) != 0) {
        modify_field(icmp_01.icmp_code, l4_dport);
    }
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_01) != 0) {
        modify_field(icmp_01.icmp_code, l4_dport);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TENANT_ID_01) != 0) {
        modify_field(vxlan_01.vni, tenant_id);
    }
}

action l4_hdr_transpositions_layer_1(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_1) != 0) {
        add_header(vxlan_1);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_1) != 0) {
        add_header(udp_1);
    }

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
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_1) != 0) {
        modify_field(icmp_1.icmp_code, l4_dport);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TENANT_ID_1) != 0) {
        modify_field(vxlan_1.vni, tenant_id);
    }
}

action l4_hdr_transpositions_layer_2(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_2) != 0) {
        add_header(vxlan_2);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_2) != 0) {
        add_header(udp_2);
    }

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
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_2) != 0) {
        modify_field(icmp_2.icmp_code, l4_dport);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TENANT_ID_2) != 0) {
        modify_field(vxlan_2.vni, tenant_id);
    }
}

action l4_hdr_transpositions_layer_3(hdr_bits, tenant_id, l4_sport, l4_dport) {
    if ((hdr_bits & TRANSPOSITIONS_PUSH_VXLAN_3) != 0) {
        add_header(vxlan_3);
    }
    if ((hdr_bits & TRANSPOSITIONS_PUSH_UDP_3) != 0) {
        add_header(udp_3);
    }

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
    if ((hdr_bits & TRANSPOSITIONS_MODIFY_ICMP_CODE_3) != 0) {
        modify_field(icmp_3.icmp_code, l4_dport);
    }

    if ((hdr_bits & TRANSPOSITIONS_MODIFY_TENANT_ID_3) != 0) {
        modify_field(vxlan_3.vni, tenant_id);
    }
}

action l4_hdr_transpositions(hdr_bits, tenant_id, l4_sport, l4_dport) {
    l4_hdr_transpositions_layer_00(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_hdr_transpositions_layer_01(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_hdr_transpositions_layer_1(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_hdr_transpositions_layer_2(hdr_bits, tenant_id, l4_sport, l4_dport);
    l4_hdr_transpositions_layer_3(hdr_bits, tenant_id, l4_sport, l4_dport);

    modify_field(scratch_metadata.hdr_bits, hdr_bits);
}

@pragma stage 5
@pragma hbm_table
table hdr_transpositions0 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table hdr_transpositions1 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table hdr_transpositions2 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

@pragma stage 5
@pragma hbm_table
table hdr_transpositions3 {
    reads {
        flow_action_metadata.flow_index : exact;
    }
    actions {
        l4_hdr_transpositions;
    }
    size : HDR_TRANSPOSITIONS_TABLE_SIZE;
}

control ingress_transpositions {
    apply(hdr_transpositions0);
    apply(hdr_transpositions1);
    apply(hdr_transpositions2);
    apply(hdr_transpositions3);
}
