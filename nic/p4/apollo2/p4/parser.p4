/******************************************************************************
 * Capri Intrinsic header definitions
 *****************************************************************************/
header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;
@pragma synthetic_header
@pragma pa_field_union ingress apollo_i2e_metadata.local_vnic_tag   vnic_metadata.local_vnic_tag
header apollo_i2e_metadata_t apollo_i2e_metadata;

header service_header_t service_header;
header egress_service_header_t egress_service_header;

// Inter-pipeline headers
@pragma synthetic_header
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_bypass      slacl_metadata.bypass
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_base_addr   slacl_metadata.base_addr
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_addr1       slacl_metadata.addr1
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_addr2       slacl_metadata.addr2
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_ip_15_00    slacl_metadata.ip_15_00
@pragma pa_field_union ingress p4_to_rxdma_header.slacl_ip_31_16    slacl_metadata.ip_31_16
@pragma pa_field_union ingress p4_to_rxdma_header.ip_proto          key_metadata.proto
@pragma pa_field_union ingress p4_to_rxdma_header.l4_sport          key_metadata.sport
@pragma pa_field_union ingress p4_to_rxdma_header.l4_dport          key_metadata.dport
@pragma pa_field_union ingress p4_to_rxdma_header.direction         control_metadata.direction
@pragma pa_field_union ingress p4_to_rxdma_header.ingress_vnic      control_metadata.ingress_vnic
@pragma pa_field_union ingress p4_to_rxdma_header.egress_vnic       control_metadata.egress_vnic
header p4_to_rxdma_header_t p4_to_rxdma_header;

header p4_to_arm_header_t p4_to_arm_header;

@pragma synthetic_header
@pragma pa_field_union ingress p4_to_txdma_header.lpm_bypass        lpm_metadata.bypass
@pragma pa_field_union ingress p4_to_txdma_header.lpm_base_addr     lpm_metadata.base_addr
@pragma pa_field_union ingress p4_to_txdma_header.lpm_addr          lpm_metadata.addr
@pragma pa_field_union ingress p4_to_txdma_header.lpm_dst           key_metadata.dst
@pragma pa_field_union ingress p4_to_txdma_header.vcn_id            vnic_metadata.vcn_id
header p4_to_txdma_header_t p4_to_txdma_header;

header txdma_to_p4i_header_t txdma_to_p4i_header;
header txdma_to_p4e_header_t txdma_to_p4e_header;

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
header mpls_t mpls_0[MPLS_DEPTH];
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
        TM_PORT_DMA : parse_txdma;
        default : parse_uplink;
        0x1 mask 0 : deparse_ingress;
        0x1 mask 0 : egress_start;
    }
}

parser parse_uplink {
    return select(capri_intrinsic.lif) {
        HOSTPORT_LIF : parse_packet_from_host;
        SWITCHPORT_LIF : parse_packet_from_switch;
        default : parse_packet;
    }
}

parser parse_packet_from_host {
    set_metadata(control_metadata.direction, TX_FROM_HOST);
    return parse_packet;
}

parser parse_packet_from_switch {
    set_metadata(control_metadata.direction, RX_FROM_SWITCH);
    return parse_packet;
}

parser parse_service_header {
    extract(capri_p4_intrinsic);
    extract(service_header);
    return parse_packet;
}

@pragma xgress ingress
parser parse_txdma {
    extract(capri_txdma_intrinsic);
    extract(txdma_to_p4e_header);
    extract(txdma_to_p4i_header);
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
/*
 * Any data pkts entering p4e either come from txdma or from p4i after looping
 * around the rxdma and txdma. So pkts entering p4e will always have txdma headers
 * p4i should not invalidate them
 */
@pragma xgress egress
parser egress_start {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_EGRESS  : parse_egress_to_egress;
        default         : parse_egress_common;
        0x1 mask 0      : deparse_egress;
    }
}

@pragma xgress egress
parser parse_egress_to_egress {
    extract(egress_service_header);
    return parse_egress_common;
}

@pragma xgress egress
parser parse_egress_common {
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_span_copy;
        default : parse_egress;
    }
}

@pragma xgress egress
parser parse_egress {
    extract(capri_txdma_intrinsic);
    extract(txdma_to_p4e_header);
    return parse_i2e_metadata;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    // TODO: Should this go into parse_egress ?
    return parse_i2e_metadata;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.direction
parser parse_i2e_metadata {
    extract(apollo_i2e_metadata);
    // TODO: NCC error here. This is needed for predicating in p4eg
    //set_metadata(control_metadata.direction, apollo_i2e_metadata.direction + 0);
    return parse_packet;
}

@pragma deparse_only
@pragma xgress ingress
parser deparse_ingress {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    extract(capri_rxdma_intrinsic);
    extract(p4_to_rxdma_header);
    // p4_to_arm_header will be present only if to_arm is set in p4_to_rxdma_header
    extract(p4_to_arm_header);
    // set the splitter offset to here
    extract(p4_to_txdma_header);
    // Below 2 headers are carried over in p4i-p4e path
    extract(capri_txdma_intrinsic);
    extract(txdma_to_p4e_header);
    // i2e should be extracted below
    extract(apollo_i2e_metadata);

    return parse_packet;
}

@pragma deparse_only
@pragma xgress egress
parser deparse_egress {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    // Below are headers used in case of egress-to-egress recirc
    extract(egress_service_header);
    extract(capri_txdma_intrinsic);
    extract(txdma_to_p4e_header);
    extract(apollo_i2e_metadata);

    // layer 0
    /*
     * Control the egress deparsing according to the below order
     * Tx Traffic:
     *     The pkts will be extracted to eth_1, ctag_1, ip_1, tcp
     *     Remove the eth_1 and ctag_1 header
     *     Add eth_0, ip_0, gre_0 and mpls_0 header.
     *     So outgoing traffic will have
     *       eth_0, ip_0, gre_0, mpls_0, ip_1, tcp
     *
     * Rx Traffic:
     *     The pkts will be extracted to eth_1, ip_1, gre_1, mpls, ip_2, tcp
     *     Remove the eth_1, ip_1, gre_1, mpls headers
     *     Add eth_0, ctag_0 header.
     *     So outgoing traffic will have
     *       eth_0, ctag_0, ip_2, tcp
     *
     */
    extract(ethernet_0);
    extract(ctag_0);
    extract(ipv4_0);
    extract(ipv6_0);
    extract(udp_0);
    extract(vxlan_0);
    extract(gre_0);
    extract(erspan_0);
    extract(mpls_0[0]);
    extract(mpls_0[1]);

    return parse_packet;
}
