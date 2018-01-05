header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header capri_i2e_metadata_t capri_i2e_metadata;

// layer 0
header ethernet_t ethernet_0;
header vlan_tag_t stag_0;
header vlan_tag_t ctag_0;
@pragma pa_header_union xgress ipv4_0 ipv6_0
header ipv4_t ipv4_0;
header ipv6_t ipv6_0;
@pragma pa_header_union xgress udp_0 tcp_0 icmp_0
header udp_t udp_0;
header tcp_t tcp_0;
header icmp_t icmp_0;
@pragma pa_header_union xgress vxlan_0 nvgre_0
header vxlan_t vxlan_0;
header nvgre_t nvgre_0;
header gre_t gre_0;
header erspan_header_t3_t erspan_0;

// layer 1
header ethernet_t ethernet_1;
header vlan_tag_t stag_1;
header vlan_tag_t ctag_1;
@pragma pa_header_union xgress ipv4_1 ipv6_1
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
@pragma pa_header_union xgress udp_1 tcp_1 icmp_1
header udp_t udp_1;
header tcp_t tcp_1;
header icmp_t icmp_1;
@pragma pa_header_union xgress vxlan_1 nvgre_1
header vxlan_t vxlan_1;
header nvgre_t nvgre_1;
header gre_t gre_1;
header erspan_header_t3_t erspan_1;

// layer 2
header ethernet_t ethernet_2;
header vlan_tag_t stag_2;
header vlan_tag_t ctag_2;
@pragma pa_header_union xgress ipv4_2 ipv6_2
header ipv4_t ipv4_2;
header ipv6_t ipv6_2;
@pragma pa_header_union xgress udp_2 tcp_2 icmp_2
header udp_t udp_2;
header tcp_t tcp_2;
header icmp_t icmp_2;
@pragma pa_header_union xgress vxlan_2 nvgre_2
header vxlan_t vxlan_2;
header nvgre_t nvgre_2;
header gre_t gre_2;
header erspan_header_t3_t erspan_2;

parser start {
    return select(current(0, 4)) {
        default : parse_nic;
        0x1 mask 0 : deparse_start;
        0x2 mask 0 : dummy;
    }
}

parser dummy {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    return ingress;
}

@pragma deparse_only
parser deparse_start {
    // layer 0
    extract(ethernet_0);
    extract(stag_0);
    extract(ctag_0);
    extract(ipv4_0);
    extract(ipv6_0);
    extract(udp_0);
    extract(tcp_0);
    extract(icmp_0);
    extract(vxlan_0);
    extract(nvgre_0);
    extract(gre_0);
    extract(erspan_0);

    return parse_nic;
}


parser parse_nic {
    extract(capri_intrinsic);
    return parse_ethernet_1;
}

/******************************************************************************
 * Layer 1
 *****************************************************************************/
parser parse_ethernet_1 {
    extract(ethernet_1);
    return select(latest.etherType) {
        ETHERTYPE_STAG : parse_stag_1;
        ETHERTYPE_CTAG : parse_ctag_1;
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: ingress;
    }
}

parser parse_stag_1 {
    extract(stag_1);
    return parse_ctag_1;
}

parser parse_ctag_1 {
    extract(ctag_1);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: ingress;
    }
}

parser parse_ipv4_1 {
    extract(ipv4_1);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp_1;
        IP_PROTO_TCP : parse_tcp_1;
        IP_PROTO_UDP : parse_udp_1;
        IP_PROTO_GRE : parse_gre_1;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_1;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_1;
        default: ingress;
    }
}

parser parse_ipv4_in_ip_1 {
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_ipv4_2;
}

parser parse_ipv6_in_ip_1 {
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_ipv6_2;
}

parser parse_ipv6_1 {
    extract(ipv6_1);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp_1;
        IP_PROTO_TCP : parse_tcp_1;
        IP_PROTO_UDP : parse_udp_1;
        IP_PROTO_GRE : parse_gre_1;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_1;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_1;
        default : ingress;
    }
}

parser parse_icmp_1 {
    extract(icmp_1);
    return ingress;
}

parser parse_tcp_1 {
    extract(tcp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return ingress;
}

parser parse_udp_1 {
    extract(udp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_1;
        default: ingress;
    }
}

parser parse_gre_1 {
    extract(gre_1);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        GRE_PROTO_NVGRE : parse_nvgre_1;
        ETHERTYPE_IPV4 : parse_gre_ipv4_1;
        ETHERTYPE_IPV6 : parse_gre_ipv6_1;
        GRE_PROTO_ERSPAN_T3 : parse_erspan_1;
        default: ingress;
    }
}

parser parse_gre_ipv4_1 {
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv4_2;
}

parser parse_gre_ipv6_1 {
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv6_2;
}

parser parse_nvgre_1 {
    extract(nvgre_1);
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_NVGRE);
    set_metadata(tunnel_metadata.tunnel_vni_1, latest.tni);
    return parse_ethernet_2;
}

parser parse_erspan_1 {
    extract(erspan_1);
    return parse_ethernet_2;
}

parser parse_vxlan_1 {
    extract(vxlan_1);
    set_metadata(tunnel_metadata.tunnel_type_1, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni_1, latest.vni);
    return parse_ethernet_2;
}

/******************************************************************************
 * Layer 2
 *****************************************************************************/
parser parse_ethernet_2 {
    extract(ethernet_2);
    return select(latest.etherType) {
        ETHERTYPE_STAG : parse_stag_2;
        ETHERTYPE_CTAG : parse_ctag_2;
        ETHERTYPE_IPV4 : parse_ipv4_2;
        ETHERTYPE_IPV6 : parse_ipv6_2;
        default: ingress;
    }
}

parser parse_stag_2 {
    extract(stag_2);
    return parse_ctag_2;
}

parser parse_ctag_2 {
    extract(ctag_2);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_2;
        ETHERTYPE_IPV6 : parse_ipv6_2;
        default: ingress;
    }
}

parser parse_ipv4_2 {
    extract(ipv4_2);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp_2;
        IP_PROTO_TCP : parse_tcp_2;
        IP_PROTO_UDP : parse_udp_2;
        IP_PROTO_GRE : parse_gre_2;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_2;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_2;
        default: ingress;
    }
}

parser parse_ipv4_in_ip_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return ingress;
}

parser parse_ipv6_in_ip_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return ingress;
}

parser parse_ipv6_2 {
    extract(ipv6_2);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp_2;
        IP_PROTO_TCP : parse_tcp_2;
        IP_PROTO_UDP : parse_udp_2;
        IP_PROTO_GRE : parse_gre_2;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_2;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_2;
        default : ingress;
    }
}

parser parse_icmp_2 {
    extract(icmp_2);
    return ingress;
}

parser parse_tcp_2 {
    extract(tcp_2);
    set_metadata(l4_metadata.l4_sport_2, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_2, latest.dstPort);
    return ingress;
}

parser parse_udp_2 {
    extract(udp_2);
    set_metadata(l4_metadata.l4_sport_2, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_2, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_2;
        default: ingress;
    }
}

parser parse_gre_2 {
    extract(gre_2);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        GRE_PROTO_NVGRE : parse_nvgre_2;
        ETHERTYPE_IPV4 : parse_gre_ipv4_2;
        ETHERTYPE_IPV6 : parse_gre_ipv6_2;
        GRE_PROTO_ERSPAN_T3 : parse_erspan_2;
        default: ingress;
    }
}

parser parse_gre_ipv4_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_GRE);
    return ingress;
}

parser parse_gre_ipv6_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_GRE);
    return ingress;
}

parser parse_nvgre_2 {
    extract(nvgre_2);
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_NVGRE);
    set_metadata(tunnel_metadata.tunnel_vni_2, latest.tni);
    return ingress;
}

parser parse_erspan_2 {
    extract(erspan_2);
    return ingress;
}

parser parse_vxlan_2 {
    extract(vxlan_2);
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni_2, latest.vni);
    return ingress;
}

