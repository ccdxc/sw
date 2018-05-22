/*****************************************************************************/
/* Rx pipeline                                                               */
/*****************************************************************************/
action rx_key1(match_fields) {
    if ((match_fields & MATCH_ETHERNET_DST) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_1, ethernet_1.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_1, ethernet_1.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_1, ethernet_1.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID) != 0) {
        modify_field(flow_lkp_metadata.ctag_1, ctag_1.vid);
    }

    if (ipv4_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv4_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv4_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv4_1.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv4_1.protocol);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv4_1.ttl);
        }
    }
    if (ipv6_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv6_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv6_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv6_1.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv6_1.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv6_1.hopLimit);
        }
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

action rx_key2(match_fields) {
    if ((match_fields & MATCH_ETHERNET_DST) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_2, ethernet_2.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_2, ethernet_2.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_2, ethernet_2.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID) != 0) {
        modify_field(flow_lkp_metadata.ctag_2, ctag_2.vid);
    }

    if (ipv4_2.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_2, ipv4_2.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_2, ipv4_2.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_2, ipv4_2.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_2, ipv4_2.protocol);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_2, ipv4_2.ttl);
        }
    }
    if (ipv6_2.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_2, ipv6_2.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_2, ipv6_2.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_2, ipv6_2.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_2, ipv6_2.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_2, ipv6_2.hopLimit);
        }
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

action rx_key3(match_fields) {
    if ((match_fields & MATCH_TRANSPORT_SRC_PORT_1) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_1, l4_metadata.l4_sport_1);
    }
    if ((match_fields & MATCH_TRANSPORT_DST_PORT_1) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_1, l4_metadata.l4_dport_1);
    }
    if ((match_fields & MATCH_TCP_FLAGS_1) != 0) {
        modify_field(flow_lkp_metadata.tcp_flags_1, tcp_1.flags);
    }
    if ((match_fields & MATCH_ICMP_TYPE_1) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_1, icmp_1.icmp_type);
    }
    if ((match_fields & MATCH_ICMP_CODE_1) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_1, icmp_1.icmp_code);
    }
    if ((match_fields & MATCH_TENANT_ID_1) != 0) {
        modify_field(flow_lkp_metadata.tunnel_type_1, tunnel_metadata.tunnel_type_1);
        modify_field(flow_lkp_metadata.tenant_id_1, tunnel_metadata.tunnel_vni_1);
    }
    if ((match_fields & MATCH_MPLS_1) != 0) {
        if (mpls[0].valid == TRUE) {
            modify_field(flow_lkp_metadata.mpls0_label, mpls[0].label);
            modify_field(flow_lkp_metadata.mpls0_exp, mpls[0].exp);
            modify_field(flow_lkp_metadata.mpls0_bos, mpls[0].bos);
            modify_field(flow_lkp_metadata.mpls0_ttl, mpls[0].ttl);
        }
        if (mpls[1].valid == TRUE) {
            modify_field(flow_lkp_metadata.mpls1_label, mpls[1].label);
            modify_field(flow_lkp_metadata.mpls1_exp, mpls[1].exp);
            modify_field(flow_lkp_metadata.mpls1_bos, mpls[1].bos);
            modify_field(flow_lkp_metadata.mpls1_ttl, mpls[1].ttl);
        }
        if (mpls[2].valid == TRUE) {
            modify_field(flow_lkp_metadata.mpls2_label, mpls[2].label);
            modify_field(flow_lkp_metadata.mpls2_exp, mpls[2].exp);
            modify_field(flow_lkp_metadata.mpls2_bos, mpls[2].bos);
            modify_field(flow_lkp_metadata.mpls2_ttl, mpls[2].ttl);
        }
    }

    if ((match_fields & MATCH_TRANSPORT_SRC_PORT_2) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_2, l4_metadata.l4_sport_2);
    }
    if ((match_fields & MATCH_TRANSPORT_DST_PORT_2) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_2, l4_metadata.l4_dport_2);
    }
    if ((match_fields & MATCH_TCP_FLAGS_2) != 0) {
        modify_field(flow_lkp_metadata.tcp_flags_2, tcp_2.flags);
    }
    if ((match_fields & MATCH_ICMP_TYPE_2) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_2, icmp_2.icmp_type);
    }
    if ((match_fields & MATCH_ICMP_CODE_2) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_2, icmp_2.icmp_code);
    }

    if ((match_fields & MATCH_CUSTOM_FIELD_1) != 0) {
        modify_field(flow_lkp_metadata.custom_field_1, 0);
    }
    if ((match_fields & MATCH_CUSTOM_FIELD_2) != 0) {
        modify_field(flow_lkp_metadata.custom_field_1, 0);
    }
    if ((match_fields & MATCH_CUSTOM_FIELD_3) != 0) {
        modify_field(flow_lkp_metadata.custom_field_1, 0);
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

@pragma stage 0
table rx_key1 {
    reads {
        ethernet_1.valid               : ternary;
        ipv4_1.valid                   : ternary;
        ipv6_1.valid                   : ternary;
        tcp_1.valid                    : ternary;
        udp_1.valid                    : ternary;
        icmp_1.valid                   : ternary;
        tunnel_metadata.tunnel_type_1  : ternary;

        ethernet_2.valid               : ternary;
        ipv4_2.valid                   : ternary;
        ipv6_2.valid                   : ternary;
        tcp_2.valid                    : ternary;
        udp_2.valid                    : ternary;
        icmp_2.valid                   : ternary;
    }
    actions {
        rx_key1;
    }
    size : KEY_TABLE_SIZE;
}

@pragma stage 1
table rx_key2 {
    reads {
        ethernet_1.valid               : ternary;
        ipv4_1.valid                   : ternary;
        ipv6_1.valid                   : ternary;
        tcp_1.valid                    : ternary;
        udp_1.valid                    : ternary;
        icmp_1.valid                   : ternary;
        tunnel_metadata.tunnel_type_1  : ternary;

        ethernet_2.valid               : ternary;
        ipv4_2.valid                   : ternary;
        ipv6_2.valid                   : ternary;
        tcp_2.valid                    : ternary;
        udp_2.valid                    : ternary;
        icmp_2.valid                   : ternary;
    }
    actions {
        rx_key2;
    }
    size : KEY_TABLE_SIZE;
}

@pragma stage 1
table rx_key3 {
    reads {
        ethernet_1.valid               : ternary;
        ipv4_1.valid                   : ternary;
        ipv6_1.valid                   : ternary;
        tcp_1.valid                    : ternary;
        udp_1.valid                    : ternary;
        icmp_1.valid                   : ternary;
        tunnel_metadata.tunnel_type_1  : ternary;

        ethernet_2.valid               : ternary;
        ipv4_2.valid                   : ternary;
        ipv6_2.valid                   : ternary;
        tcp_2.valid                    : ternary;
        udp_2.valid                    : ternary;
        icmp_2.valid                   : ternary;
    }
    actions {
        rx_key3;
    }
    size : KEY_TABLE_SIZE;
}

control rx_key {
    apply(rx_key1);
    apply(rx_key2);
    apply(rx_key3);
}

/*****************************************************************************/
/* Tx pipeline                                                               */
/*****************************************************************************/
action tx_key(match_fields) {
    // match_fields : lower 16 bits => L2/L3, upper 16 bits => L4
    if ((match_fields & MATCH_ETHERNET_DST) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_1, ethernet_1.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_1, ethernet_1.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_1, ethernet_1.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID) != 0) {
        modify_field(flow_lkp_metadata.ctag_1, ctag_1.vid);
    }

    if (ipv4_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv4_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv4_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv4_1.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv4_1.protocol);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv4_1.ttl);
        }
    }
    if (ipv6_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv6_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv6_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv6_1.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv6_1.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv6_1.hopLimit);
        }
    }

    if ((match_fields & MATCH_TRANSPORT_SRC_PORT_1) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_1, l4_metadata.l4_sport_1);
    }
    if ((match_fields & MATCH_TRANSPORT_DST_PORT_1) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_1, l4_metadata.l4_dport_1);
    }
    if ((match_fields & MATCH_TCP_FLAGS_1) != 0) {
        modify_field(flow_lkp_metadata.tcp_flags_1, tcp_1.flags);
    }
    if ((match_fields & MATCH_ICMP_TYPE_1) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_1, icmp_1.icmp_type);
    }
    if ((match_fields & MATCH_ICMP_CODE_1) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_1, icmp_1.icmp_code);
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

@pragma stage 0
table tx_key {
    reads {
        ethernet_1.valid               : ternary;
        ipv4_1.valid                   : ternary;
        ipv6_1.valid                   : ternary;
        tcp_1.valid                    : ternary;
        udp_1.valid                    : ternary;
        icmp_1.valid                   : ternary;
    }
    actions {
        tx_key;
    }
    size : KEY_TABLE_SIZE;
}

control tx_key {
    apply(tx_key);
}
