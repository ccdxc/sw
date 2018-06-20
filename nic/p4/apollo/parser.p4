/******************************************************************************
 * Capri Intrinsic header definitions
 *****************************************************************************/
header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;
header capri_i2e_metadata_t capri_i2e_metadata;

header service_header_t service_header;

// layer 0
header ethernet_t ethernet_0;
header vlan_tag_t ctag_0;
@pragma pa_header_union egress ipv4_0 ipv6_0
header ipv4_t ipv4_0;
header ipv6_t ipv6_0;
header udp_t udp_0;
@pragma pa_header_union egress vxlan_0 gre_0
header vxlan_t vxlan_0;
header gre_t gre_0;
header erspan_header_t3_t erspan_0;

// layer 1
header ethernet_t ethernet_1;
header vlan_tag_t ctag_1;
@pragma pa_header_union xgress ipv4_1 ipv6_1
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
header udp_t udp_1;
@pragma pa_header_union ingress vxlan_1
header vxlan_t vxlan_1;
header gre_t gre_1;
header mpls_t mpls[MPLS_DEPTH];

// layer 2
header ethernet_t ethernet_2;
header vlan_tag_t ctag_2;
@pragma pa_header_union xgress ipv4_2 ipv6_2
header ipv4_t ipv4_2;
header ipv6_t ipv6_2;
@pragma pa_header_union xgress udp_2 tcp icmp
header udp_t udp_2;
header tcp_t tcp;
header icmp_t icmp;

parser start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_INGRESS : parse_service_header;
        default : parse_packet;
        0x1 mask 0 : egress_start;
    }
}

parser parse_service_header {
    extract(service_header);
    return parse_packet;
}

/******************************************************************************
 * Layer 1
 *****************************************************************************/
parser parse_packet {
    extract(ethernet_1);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_ctag_1;
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        ETHERTYPE_MPLS_UNICAST : parse_mpls;
        ETHERTYPE_MPLS_MULTICAST : parse_mpls;
        default: ingress;
    }
}

parser parse_ctag_1 {
    extract(ctag_1);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        ETHERTYPE_MPLS_UNICAST : parse_mpls;
        ETHERTYPE_MPLS_MULTICAST : parse_mpls;
        default: ingress;
    }
}

parser parse_ipv4_1 {
    extract(ipv4_1);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_1;
        IP_PROTO_GRE : parse_gre_1;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_1;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_1;
        default: ingress;
    }
}

parser parse_ipv4_in_ip_1 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_ipv4_2;
}

parser parse_ipv6_in_ip_1 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_ipv6_2;
}

parser parse_ipv6_1 {
    extract(ipv6_1);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_1;
        IP_PROTO_GRE : parse_gre_1;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_1;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_1;
        default : ingress;
    }
}

parser parse_icmp {
    extract(icmp);
    return ingress;
}

parser parse_tcp {
    extract(tcp);
    set_metadata(key_metadata.sport, latest.srcPort);
    set_metadata(key_metadata.dport, latest.dstPort);
    return ingress;
}

parser parse_udp_1 {
    extract(udp_1);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_1;
        UDP_PORT_MPLS : parse_mpls;
        default: ingress;
    }
}

parser parse_gre_1 {
    extract(gre_1);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        ETHERTYPE_IPV4 : parse_gre_ipv4_1;
        ETHERTYPE_IPV6 : parse_gre_ipv6_1;
        ETHERTYPE_MPLS_UNICAST : parse_mpls;
        ETHERTYPE_MPLS_MULTICAST : parse_mpls;
        default: ingress;
    }
}

parser parse_gre_ipv4_1 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv4_2;
}

parser parse_gre_ipv6_1 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv6_2;
}

parser parse_vxlan_1 {
    extract(vxlan_1);
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni, latest.vni);
    return parse_ethernet_2;
}

parser parse_mpls {
    extract(mpls[next]);
    return select(latest.bos) {
        0 : parse_mpls;
        1 : parse_mpls_bos;
        default: ingress;
    }
}

parser parse_mpls_bos {
    return select(current(0, 4)) {
        0x4 : parse_mpls_inner_ipv4;
        0x6 : parse_mpls_inner_ipv6;
        default: parse_eompls;
    }
}

parser parse_eompls {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_MPLS_L2VPN);
    return parse_ethernet_2;
}

parser parse_mpls_inner_ipv4 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_MPLS_L3VPN);
    return parse_ipv4_2;
}

parser parse_mpls_inner_ipv6 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_MPLS_L3VPN);
    return parse_ipv6_2;
}

/******************************************************************************
 * Layer 2
 *****************************************************************************/
parser parse_ethernet_2 {
    extract(ethernet_2);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_ctag_2;
        ETHERTYPE_IPV4 : parse_ipv4_2;
        ETHERTYPE_IPV6 : parse_ipv6_2;
        default: ingress;
    }
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
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_2;
        default: ingress;
    }
}

parser parse_ipv6_2 {
    extract(ipv6_2);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_2;
        default : ingress;
    }
}

parser parse_udp_2 {
    extract(udp_2);
    set_metadata(key_metadata.sport, latest.srcPort);
    set_metadata(key_metadata.dport, latest.dstPort);
    return ingress;
}

/******************************************************************************/
/* Egress parser                                                              */
/******************************************************************************/
@pragma xgress egress
parser egress_start {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_span_copy;
        default : parse_i2e_metadata;
        0x1 mask 0 : deparse_egress;
    }
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    return parse_i2e_metadata;
}

@pragma xgress egress
parser parse_i2e_metadata {
    extract(capri_i2e_metadata);
    return parse_packet;
}

@pragma deparse_only
@pragma xgress egress
parser deparse_egress {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);

    // layer 0
    extract(ethernet_0);
    extract(ctag_0);
    extract(ipv4_0);
    extract(ipv6_0);
    extract(udp_0);
    extract(vxlan_0);
    extract(gre_0);
    extract(erspan_0);
    extract(mpls[0]);
    extract(mpls[1]);
    extract(mpls[2]);

    return parse_packet;
}
