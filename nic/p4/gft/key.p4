action ingress_key1(match_fields) {
    if ((match_fields & MATCH_ETHERNET_DST_1) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_1, ethernet_1.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC_1) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_1, ethernet_1.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE_1) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_1, ethernet_1.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID_1) != 0) {
        modify_field(flow_lkp_metadata.ctag_1, ctag_1.vid);
    }
    if ((match_fields & MATCH_SERVICE_VLAN_ID_1) != 0) {
        modify_field(flow_lkp_metadata.stag_1, stag_1.vid);
    }

    if (ipv4_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_1) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv4_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_1) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv4_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_1) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv4_1.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO_1) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv4_1.protocol);
        }
        if ((match_fields & MATCH_IP_TTL_1) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv4_1.ttl);
        }
    }
    if (ipv6_1.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_1) != 0) {
            modify_field(flow_lkp_metadata.ip_src_1, ipv6_1.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_1) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_1, ipv6_1.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_1) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_1, ipv6_1.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO_1) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_1, ipv6_1.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL_1) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_1, ipv6_1.hopLimit);
        }
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

action ingress_key2(match_fields) {
    if ((match_fields & MATCH_ETHERNET_DST_2) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_2, ethernet_2.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC_2) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_2, ethernet_2.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE_2) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_2, ethernet_2.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID_2) != 0) {
        modify_field(flow_lkp_metadata.ctag_2, ctag_2.vid);
    }
    if ((match_fields & MATCH_SERVICE_VLAN_ID_2) != 0) {
        modify_field(flow_lkp_metadata.stag_2, stag_2.vid);
    }

    if (ipv4_2.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_2) != 0) {
            modify_field(flow_lkp_metadata.ip_src_2, ipv4_2.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_2) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_2, ipv4_2.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_2) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_2, ipv4_2.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO_2) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_2, ipv4_2.protocol);
        }
        if ((match_fields & MATCH_IP_TTL_2) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_2, ipv4_2.ttl);
        }
    }
    if (ipv6_2.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_2) != 0) {
            modify_field(flow_lkp_metadata.ip_src_2, ipv6_2.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_2) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_2, ipv6_2.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_2) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_2, ipv6_2.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO_2) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_2, ipv6_2.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL_2) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_2, ipv6_2.hopLimit);
        }
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

action ingress_key3(match_fields) {
    if ((match_fields & MATCH_ETHERNET_DST_3) != 0) {
        modify_field(flow_lkp_metadata.ethernet_dst_3, ethernet_3.dstAddr);
    }
    if ((match_fields & MATCH_ETHERNET_SRC_3) != 0) {
        modify_field(flow_lkp_metadata.ethernet_src_3, ethernet_3.srcAddr);
    }
    if ((match_fields & MATCH_ETHERNET_TYPE_3) != 0) {
        modify_field(flow_lkp_metadata.ethernet_type_3, ethernet_3.etherType);
    }

    if ((match_fields & MATCH_CUSTOMER_VLAN_ID_3) != 0) {
        modify_field(flow_lkp_metadata.ctag_3, ctag_3.vid);
    }
    if ((match_fields & MATCH_SERVICE_VLAN_ID_3) != 0) {
        modify_field(flow_lkp_metadata.stag_3, stag_3.vid);
    }

    if (ipv4_3.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_3) != 0) {
            modify_field(flow_lkp_metadata.ip_src_3, ipv4_3.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_3) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_3, ipv4_3.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_3) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_3, ipv4_3.diffserv);
        }
        if ((match_fields & MATCH_IP_PROTO_3) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_3, ipv4_3.protocol);
        }
        if ((match_fields & MATCH_IP_TTL_3) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_3, ipv4_3.ttl);
        }
    }
    if (ipv6_3.valid == TRUE) {
        if ((match_fields & MATCH_IP_SRC_3) != 0) {
            modify_field(flow_lkp_metadata.ip_src_3, ipv6_3.srcAddr);
        }
        if ((match_fields & MATCH_IP_DST_3) != 0) {
            modify_field(flow_lkp_metadata.ip_dst_3, ipv6_3.dstAddr);
        }
        if ((match_fields & MATCH_IP_DSCP_3) != 0) {
            modify_field(flow_lkp_metadata.ip_dscp_3, ipv6_3.trafficClass);
        }
        if ((match_fields & MATCH_IP_PROTO_3) != 0) {
            modify_field(flow_lkp_metadata.ip_proto_3, ipv6_3.nextHdr);
        }
        if ((match_fields & MATCH_IP_TTL_3) != 0) {
            modify_field(flow_lkp_metadata.ip_ttl_3, ipv6_3.hopLimit);
        }
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

action ingress_key4(match_fields) {
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
        modify_field(flow_lkp_metadata.tenant_id_1, tunnel_metadata.tunnel_vni_1);
    }
    if ((match_fields & MATCH_GRE_PROTO_1) != 0) {
        modify_field(flow_lkp_metadata.gre_proto_1, gre_1.proto);
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
    if ((match_fields & MATCH_TENANT_ID_2) != 0) {
        modify_field(flow_lkp_metadata.tenant_id_2, tunnel_metadata.tunnel_vni_2);
    }
    if ((match_fields & MATCH_GRE_PROTO_2) != 0) {
        modify_field(flow_lkp_metadata.gre_proto_2, gre_2.proto);
    }

    if ((match_fields & MATCH_TRANSPORT_SRC_PORT_3) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_3, l4_metadata.l4_sport_3);
    }
    if ((match_fields & MATCH_TRANSPORT_DST_PORT_3) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_3, l4_metadata.l4_dport_3);
    }
    if ((match_fields & MATCH_TCP_FLAGS_3) != 0) {
        modify_field(flow_lkp_metadata.tcp_flags_3, tcp_3.flags);
    }
    if ((match_fields & MATCH_ICMP_TYPE_3) != 0) {
        modify_field(flow_lkp_metadata.l4_sport_3, icmp_3.icmp_type);
    }
    if ((match_fields & MATCH_ICMP_CODE_3) != 0) {
        modify_field(flow_lkp_metadata.l4_dport_3, icmp_3.icmp_code);
    }
    if ((match_fields & MATCH_TENANT_ID_3) != 0) {
        modify_field(flow_lkp_metadata.tenant_id_3, tunnel_metadata.tunnel_vni_3);
    }
    if ((match_fields & MATCH_GRE_PROTO_3) != 0) {
        modify_field(flow_lkp_metadata.gre_proto_3, gre_3.proto);
    }

    modify_field(scratch_metadata.match_fields, match_fields);
}

@pragma stage 1
table ingress_key1 {
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
        tunnel_metadata.tunnel_type_2  : ternary;

        ethernet_3.valid               : ternary;
        ipv4_3.valid                   : ternary;
        ipv6_3.valid                   : ternary;
        tcp_3.valid                    : ternary;
        udp_3.valid                    : ternary;
        icmp_3.valid                   : ternary;
        tunnel_metadata.tunnel_type_3  : ternary;
    }
    actions {
        ingress_key1;
    }
    size : 512;
}

@pragma stage 1
table ingress_key2 {
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
        tunnel_metadata.tunnel_type_2  : ternary;

        ethernet_3.valid               : ternary;
        ipv4_3.valid                   : ternary;
        ipv6_3.valid                   : ternary;
        tcp_3.valid                    : ternary;
        udp_3.valid                    : ternary;
        icmp_3.valid                   : ternary;
        tunnel_metadata.tunnel_type_3  : ternary;
    }
    actions {
        ingress_key2;
    }
    size : 512;
}

@pragma stage 2
table ingress_key3 {
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
        tunnel_metadata.tunnel_type_2  : ternary;

        ethernet_3.valid               : ternary;
        ipv4_3.valid                   : ternary;
        ipv6_3.valid                   : ternary;
        tcp_3.valid                    : ternary;
        udp_3.valid                    : ternary;
        icmp_3.valid                   : ternary;
        tunnel_metadata.tunnel_type_3  : ternary;
    }
    actions {
        ingress_key3;
    }
    size : 512;
}

@pragma stage 0
table ingress_key4 {
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
        tunnel_metadata.tunnel_type_2  : ternary;

        ethernet_3.valid               : ternary;
        ipv4_3.valid                   : ternary;
        ipv6_3.valid                   : ternary;
        tcp_3.valid                    : ternary;
        udp_3.valid                    : ternary;
        icmp_3.valid                   : ternary;
        tunnel_metadata.tunnel_type_3  : ternary;
    }
    actions {
        ingress_key4;
    }
    size : 512;
}

action ingress_vport_miss() {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, EXCEPTION_VPORT);
}

action ingress_vport_hit(vport) {
    modify_field(capri_intrinsic.tm_oport, TM_PORT_DMA);
    modify_field(capri_intrinsic.lif, vport);
}

@pragma stage 0
table ingress_vport {
    reads {
        ethernet_1.valid               : ternary;
        ethernet_2.valid               : ternary;
        ethernet_1.dstAddr             : ternary;
        ethernet_2.dstAddr             : ternary;
    }
    actions {
        ingress_vport_miss;
        ingress_vport_hit;
    }
    size : 1024;
}

control ingress_key {
    apply(ingress_vport);
    apply(ingress_key1);
    apply(ingress_key2);
    apply(ingress_key3);
    apply(ingress_key4);
}
