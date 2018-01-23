header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header capri_i2e_metadata_t capri_i2e_metadata;

// layer 00
header ethernet_t ethernet_00;
header vlan_tag_t ctag_00;
@pragma pa_header_union egress ipv4_00 ipv6_00
header ipv4_t ipv4_00;
header ipv6_t ipv6_00;
@pragma pa_header_union egress udp_00 tcp_00 icmp_00
header udp_t udp_00;
header tcp_t tcp_00;
header icmp_t icmp_00;
header vxlan_t vxlan_00;
header gre_t gre_00;
header erspan_header_t3_t erspan_00;

// layer 01
header ethernet_t ethernet_01;
header vlan_tag_t ctag_01;
@pragma pa_header_union egress ipv4_01 ipv6_01
header ipv4_t ipv4_01;
header ipv6_t ipv6_01;
@pragma pa_header_union egress udp_01 tcp_01 icmp_01
header udp_t udp_01;
header tcp_t tcp_01;
header icmp_t icmp_01;
header vxlan_t vxlan_01;
header gre_t gre_01;
header erspan_header_t3_t erspan_01;

// layer 1
header ethernet_t ethernet_1;
header vlan_tag_t ctag_1;
@pragma pa_header_union xgress ipv4_1 ipv6_1
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
@pragma pa_header_union xgress udp_1 tcp_1 icmp_1
header udp_t udp_1;
header tcp_t tcp_1;
header icmp_t icmp_1;
header vxlan_t vxlan_1;
header gre_t gre_1;
header erspan_header_t3_t erspan_1;

// layer 2
header ethernet_t ethernet_2;
header vlan_tag_t ctag_2;
@pragma pa_header_union ingress ipv4_2 ipv6_2
header ipv4_t ipv4_2;
header ipv6_t ipv6_2;
@pragma pa_header_union ingress udp_2 tcp_2 icmp_2
header udp_t udp_2;
header tcp_t tcp_2;
header icmp_t icmp_2;
header vxlan_t vxlan_2;
header gre_t gre_2;
header erspan_header_t3_t erspan_2;

// layer 3
header ethernet_t ethernet_3;
header vlan_tag_t ctag_3;
//@pragma pa_header_union xgress ipv4_3 ipv6_3
@pragma pa_field_union ingress ipv4_3.ttl ipv6_3.nextHdr        // keep ordering so parser can combine
@pragma pa_field_union ingress ipv4_3.protocol ipv6_3.hopLimit  // keep ordering so parser can combine
@pragma pa_field_union ingress ipv4_3.identification ipv6_3.payloadLen
@pragma pa_field_union ingress ipv4_3.dstAddr ipv6_3.dstAddr
@pragma pa_field_union ingress ipv4_3.srcAddr ipv6_3.srcAddr
header ipv4_t ipv4_3;
header ipv6_t ipv6_3;
@pragma pa_header_union ingress udp_3 tcp_3 icmp_3
header udp_t udp_3;
header tcp_t tcp_3;
header icmp_t icmp_3;
header vxlan_t vxlan_3;
header gre_t gre_3;
header erspan_header_t3_t erspan_3;

header roce_bth_t roce_bth_1;
header roce_deth_t roce_deth_1;
header roce_deth_immdt_t roce_deth_immdt_1;

header roce_bth_t roce_bth_2;
header roce_deth_t roce_deth_2;
header roce_deth_immdt_t roce_deth_immdt_2;

parser start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.csum_err) {
        0 : parse_ethernet_1;
        1 : start_ipv4_bth_deth;
        3 : start_ipv6_bth_deth;
        4 : start_vlan_ipv6_bth_deth;
        5 : start_ipv4_bth_deth_immdt;
        6 : start_vlan_ipv4_bth_deth_immdt;
        7 : start_ipv6_bth_deth_immdt;
        8 : start_vlan_ipv6_bth_deth_immdt;
        9 : start_ipv4_bth;
        10 : start_vlan_ipv4_bth;
        11 : start_ipv6_bth;
        12 : start_vlan_ipv6_bth;
        // tunneled roce - only one level of vxlan tunnel is accelerated in h/w
        // running into header valid bit problems per state, Enable this when parser
        // split ingress/egress
        13 : start_outer_vxlan_ipv4_bth_deth;
        14 : start_outer_vlan_vxlan_ipv4_bth_deth;
        15 : start_outer_vxlan_ipv6_bth_deth;
        16 : start_outer_vlan_vxlan_ipv6_bth_deth;
        17 : start_outer_vxlan_ipv4_bth_deth_immdt;
        18 : start_outer_vlan_vxlan_ipv4_bth_deth_immdt;
        19 : start_outer_vxlan_ipv6_bth_deth_immdt;
        20 : start_outer_vlan_vxlan_ipv6_bth_deth_immdt;
        21 : start_outer_vxlan_ipv4_bth;
        22 : start_outer_vlan_vxlan_ipv4_bth;
        23 : start_outer_vxlan_ipv6_bth;
        24 : start_outer_vlan_vxlan_ipv6_bth;
        default: parse_nic;
        0x1 mask 0 : egress_start;
        0x2 mask 0 : dummy;
    }
}

@pragma deparse_only
parser dummy {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    return ingress;
}

parser parse_nic {
    return parse_ethernet_1;
}


/******************************************************************************
 * RoCE optimization
 *****************************************************************************/
parser parse_bth_deth {
    extract(udp_1);
    extract(roce_bth_1);
    extract(roce_deth_1);
    return ingress;
}
parser parse_bth_deth_immdt {
    extract(udp_1);
    extract(roce_bth_1);
    extract(roce_deth_immdt_1);
    return ingress;
}
parser parse_bth {
    extract(udp_1);
    extract(roce_bth_1);
    return ingress;
}
parser start_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_bth_deth;
}
parser start_vlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_bth_deth;
}
parser start_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ipv6_1);
    return parse_bth_deth;
}
parser start_vlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv6_1);
    return parse_bth_deth;
}
parser start_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_bth_deth_immdt;
}
parser start_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv6_1);
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv6_1);
    return parse_bth_deth_immdt;
}
parser start_ipv4_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_bth;
}
parser start_vlan_ipv4_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_bth;
}
parser start_ipv6_bth {
    extract(ethernet_1);
    extract(ipv6_1);
    return parse_bth;
}
parser start_vlan_ipv6_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv6_1);
    return parse_bth;
}

// tunneled roce
parser parse_bth_deth_2 {
    extract(udp_2);
    extract(roce_bth_2);
    extract(roce_deth_2);
    return ingress;
}
parser parse_bth_deth_immdt_2 {
    extract(udp_2);
    extract(roce_bth_2);
    extract(roce_deth_immdt_2);
    return ingress;
}
parser parse_bth_2 {
    extract(udp_2);
    extract(roce_bth_2);
    return ingress;
}

parser start_outer_vxlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vxlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vxlan_ipv4_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vlan_vxlan_ipv4_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vxlan_ipv6_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth;
}
parser start_outer_vlan_vxlan_ipv6_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    return parse_vxlan_ipv6_bth;
}

parser parse_vxlan_ipv4_bth_deth {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_deth_split;
}
parser parse_vxlan_ipv4_bth_deth_split {
    extract(ipv4_2);
    return parse_bth_deth_2;
}
parser parse_vxlan_ipv6_bth_deth {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_deth_split;
}
parser parse_vxlan_ipv6_bth_deth_split {
    extract(ipv6_2);
    return parse_bth_deth_2;
}
parser parse_vxlan_ipv4_bth_deth_immdt {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_deth_immdt_split;
}

parser parse_vxlan_ipv4_bth_deth_immdt_split {
    extract(ipv4_2);
    return parse_bth_deth_immdt_2;
}
parser parse_vxlan_ipv6_bth_deth_immdt {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_deth_immdt_split;
}
parser parse_vxlan_ipv6_bth_deth_immdt_split {
    extract(ipv6_2);
    return parse_bth_deth_immdt_2;
}
parser parse_vxlan_ipv4_bth {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_split;
}
parser parse_vxlan_ipv4_bth_split {
    extract(ipv4_2);
    return parse_bth_2;
}
parser parse_vxlan_ipv6_bth {
    extract(udp_1);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_split;
}
parser parse_vxlan_ipv6_bth_split {
    extract(ipv6_2);
    return parse_bth_2;
}


/******************************************************************************
 * Layer 1
 *****************************************************************************/
parser parse_ethernet_1 {
    extract(ethernet_1);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_ctag_1;
        ETHERTYPE_IPV4 : parse_ipv4_1;
        ETHERTYPE_IPV6 : parse_ipv6_1;
        default: ingress;
    }
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
    return parse_ipv4_3;
}

parser parse_ipv6_in_ip_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_ipv6_3;
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
        ETHERTYPE_IPV4 : parse_gre_ipv4_2;
        ETHERTYPE_IPV6 : parse_gre_ipv6_2;
        GRE_PROTO_ERSPAN_T3 : parse_erspan_2;
        default: ingress;
    }
}

parser parse_gre_ipv4_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv4_3;
}

parser parse_gre_ipv6_2 {
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_GRE);
    return parse_ipv6_3;
}

parser parse_erspan_2 {
    extract(erspan_2);
    return parse_ethernet_3;
}

parser parse_vxlan_2 {
    extract(vxlan_2);
    set_metadata(tunnel_metadata.tunnel_type_2, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni_2, latest.vni);
    return parse_ethernet_3;
}

/******************************************************************************
 * Layer 3
 *****************************************************************************/
parser parse_ethernet_3 {
    extract(ethernet_3);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_ctag_3;
        ETHERTYPE_IPV4 : parse_ipv4_3;
        ETHERTYPE_IPV6 : parse_ipv6_3;
        default: ingress;
    }
}

parser parse_ctag_3 {
    extract(ctag_3);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4_3;
        ETHERTYPE_IPV6 : parse_ipv6_3;
        default: ingress;
    }
}

parser parse_ipv4_3 {
    extract(ipv4_3);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp_3;
        IP_PROTO_TCP : parse_tcp_3;
        IP_PROTO_UDP : parse_udp_3;
        IP_PROTO_GRE : parse_gre_3;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_3;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_3;
        default: ingress;
    }
}

parser parse_ipv4_in_ip_3 {
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return ingress;
}

parser parse_ipv6_in_ip_3 {
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return ingress;
}

parser parse_ipv6_3 {
    extract(ipv6_3);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp_3;
        IP_PROTO_TCP : parse_tcp_3;
        IP_PROTO_UDP : parse_udp_3;
        IP_PROTO_GRE : parse_gre_3;
        IP_PROTO_IPV4 : parse_ipv4_in_ip_3;
        IP_PROTO_IPV6 : parse_ipv6_in_ip_3;
        default : ingress;
    }
}

parser parse_icmp_3 {
    extract(icmp_3);
    return ingress;
}

parser parse_tcp_3 {
    extract(tcp_3);
    set_metadata(l4_metadata.l4_sport_3, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_3, latest.dstPort);
    return ingress;
}

parser parse_udp_3 {
    extract(udp_3);
    set_metadata(l4_metadata.l4_sport_3, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_3, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_3;
        default: ingress;
        0x1 mask 0x0: parse_gre_3; // for avoiding parser flit violation
    }
}

parser parse_gre_3 {
    extract(gre_3);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        ETHERTYPE_IPV4 : parse_gre_ipv4_3;
        ETHERTYPE_IPV6 : parse_gre_ipv6_3;
        GRE_PROTO_ERSPAN_T3 : parse_erspan_3;
        default: ingress;
    }
}

parser parse_gre_ipv4_3 {
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_GRE);
    return ingress;
}

parser parse_gre_ipv6_3 {
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_GRE);
    return ingress;
}

parser parse_erspan_3 {
    extract(erspan_3);
    return ingress;
}

parser parse_vxlan_3 {
    extract(vxlan_3);
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni_3, latest.vni);
    return parse_end;
}

parser parse_end {
    return ingress;
}


/******************************************************************************/
/* Egress parser                                                              */
/******************************************************************************/
@pragma xgress egress
parser egress_start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.csum_err) {
        default: parse_txdma;
        0x1 mask 0 : egress_deparse_start;
    }
}

@pragma deparse_only
@pragma xgress egress
parser egress_deparse_start {
    // layer 00
    extract(ethernet_00);
    extract(ctag_00);
    extract(ipv4_00);
    extract(ipv6_00);
    extract(udp_00);
    extract(tcp_00);
    extract(icmp_00);
    extract(vxlan_00);
    extract(gre_00);
    extract(erspan_00);

    // layer 01
    extract(ethernet_01);
    extract(ctag_01);
    extract(ipv4_01);
    extract(ipv6_01);
    extract(udp_01);
    extract(tcp_01);
    extract(icmp_01);
    extract(vxlan_01);
    extract(gre_01);
    extract(erspan_01);

    return parse_txdma;
}

@pragma xgress egress
parser parse_txdma {
    extract(ethernet_1);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_tx_ctag_1;
        ETHERTYPE_IPV4 : parse_tx_ipv4_1;
        ETHERTYPE_IPV6 : parse_tx_ipv6_1;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_tx_ctag_1 {
    extract(ctag_1);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_tx_ipv4_1;
        ETHERTYPE_IPV6 : parse_tx_ipv6_1;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_tx_ipv4_1 {
    extract(ipv4_1);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_tx_icmp_1;
        IP_PROTO_TCP : parse_tx_tcp_1;
        IP_PROTO_UDP : parse_tx_udp_1;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_tx_ipv6_1 {
    extract(ipv6_1);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_tx_icmp_1;
        IP_PROTO_TCP : parse_tx_tcp_1;
        IP_PROTO_UDP : parse_tx_udp_1;
        default : ingress;
    }
}

@pragma xgress egress
parser parse_tx_icmp_1 {
    extract(icmp_1);
    return ingress;
}

@pragma xgress egress
parser parse_tx_tcp_1 {
    extract(tcp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return ingress;
}

@pragma xgress egress
parser parse_tx_udp_1 {
    extract(udp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return ingress;
}
