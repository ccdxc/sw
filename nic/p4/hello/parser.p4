/******************************************************************************
 * Capri Intrinsic header definitions
 *****************************************************************************/
header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;

// layer 0
header ethernet_t ethernet_0;
header vlan_tag_t ctag_0;
header ipv4_t ipv4_0;
header ipv6_t ipv6_0;
header udp_t udp_0;
header vxlan_t vxlan_0;
header gre_t gre_0;
header mpls_t mpls_src_0;
header mpls_t mpls_dst_0;

// layer 1
header ethernet_t ethernet_1;
header vlan_tag_t ctag_1;
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
header tcp_t tcp;

parser start {
    extract(capri_intrinsic);
    return select(latest.tm_iport) {
        default : parse_packet;
        0x1 mask 0 : deparse_ingress;
    }
}

@pragma deparse_only
@pragma xgress ingress
parser deparse_ingress {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);

    extract(ethernet_0);
    extract(ctag_0);
    extract(ipv4_0);
    extract(ipv6_0);
    extract(udp_0);
    extract(vxlan_0);
    extract(gre_0);
    extract(mpls_src_0);
    extract(mpls_dst_0);

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
        IP_PROTO_TCP : parse_tcp;
        default : ingress;
    }
}

parser parse_ipv6_1 {
    extract(ipv6_1);
    return ingress;
}

parser parse_tcp {
    extract(tcp);
    return ingress;
}
