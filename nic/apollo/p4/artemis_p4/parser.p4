/******************************************************************************
 * Capri Intrinsic header definitions                                         *
 *****************************************************************************/
header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;

/******************************************************************************
 * Headers                                                                    *
 *****************************************************************************/
@pragma synthetic_header
@pragma pa_field_union ingress p4i_i2e.vnic_id                      vnic_metadata.vnic_id
header artemis_i2e_metadata_t p4i_i2e;
header artemis_i2e_metadata_t p4e_i2e;

header artemis_ingress_recirc_header_t ingress_recirc;
@pragma hdr_len parser_metadata.mirror_blob_len
header mirror_blob_t mirror_blob;

// Inter-pipeline headers
header artemis_predicate_header_t predicate_header;

@pragma hdr_len parser_metadata.cps_blob_len
header cps_blob_t cps_blob;

@pragma synthetic_header
@pragma pa_field_union ingress p4_to_rxdma.flow_src                 key_metadata.src
@pragma pa_field_union ingress p4_to_rxdma.flow_dst                 key_metadata.dst
@pragma pa_field_union ingress p4_to_rxdma.flow_proto               key_metadata.proto
@pragma pa_field_union ingress p4_to_rxdma.flow_sport               key_metadata.sport
@pragma pa_field_union ingress p4_to_rxdma.flow_dport               key_metadata.dport
@pragma pa_field_union ingress p4_to_rxdma.vnic_id                  vnic_metadata.vnic_id
@pragma pa_field_union ingress p4_to_rxdma.vpc_id                   vnic_metadata.vpc_id
header artemis_p4_to_rxdma_header_t p4_to_rxdma;
@pragma synthetic_header
@pragma pa_field_union ingress p4_to_rxdma2.service_xlate_idx       p4i_i2e.service_xlate_idx
@pragma pa_field_union ingress p4_to_rxdma2.pa_or_ca_xlate_idx      p4i_i2e.pa_or_ca_xlate_idx
@pragma pa_field_union ingress p4_to_rxdma2.public_xlate_idx        p4i_i2e.public_xlate_idx
header artemis_p4_to_rxdma_header2_t p4_to_rxdma2;
@pragma synthetic_header
@pragma pa_field_union ingress p4_to_rxdma3.flow_hash               p4i_i2e.entropy_hash
header artemis_p4_to_rxdma_header3_t p4_to_rxdma3;

@pragma synthetic_header
header p4_to_p4plus_classic_nic_header_t p4_to_p4plus_classic_nic;
@pragma pa_field_union ingress p4_to_p4plus_classic_nic.l4_sport    key_metadata.sport
@pragma pa_field_union ingress p4_to_p4plus_classic_nic.l4_dport    key_metadata.dport
@pragma synthetic_header
@pragma pa_field_union ingress p4_to_p4plus_classic_nic_ip.ip_sa    key_metadata.src
@pragma pa_field_union ingress p4_to_p4plus_classic_nic_ip.ip_da    key_metadata.dst
header p4_to_p4plus_ip_addr_t p4_to_p4plus_classic_nic_ip;
header p4plus_to_p4_s1_t p4plus_to_p4;
@pragma pa_header_union ingress ctag_1
header p4plus_to_p4_s2_t p4plus_to_p4_vlan;

@pragma synthetic_header
@pragma pa_field_union ingress p4_to_arm.l2_1_offset                offset_metadata.l2_1
@pragma pa_field_union ingress p4_to_arm.l2_2_offset                offset_metadata.l2_2
@pragma pa_field_union ingress p4_to_arm.l3_1_offset                offset_metadata.l3_1
@pragma pa_field_union ingress p4_to_arm.l3_2_offset                offset_metadata.l3_2
@pragma pa_field_union ingress p4_to_arm.l4_1_offset                offset_metadata.l4_1
@pragma pa_field_union ingress p4_to_arm.l4_2_offset                offset_metadata.l4_2
header p4_to_arm_header_t p4_to_arm;

header txdma_to_p4e_header_t txdma_to_p4e;

// layer 0
header ethernet_t ethernet_0;
header vlan_tag_t ctag_0;
@pragma pa_header_union egress ipv4_0 ipv6_0
header ipv4_t ipv4_0;
header ipv6_t ipv6_0;
header udp_t udp_0;
header vxlan_t vxlan_0;
header gre_t gre_0;
header erspan_header_t3_t erspan;

// layer 1
header ethernet_t ethernet_1;
header vlan_tag_t ctag_1;
@pragma pa_header_union xgress ipv4_1 ipv6_1
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
header udp_t udp_1;
header vxlan_t vxlan_1;

// layer 2
header ethernet_t ethernet_2;
header vlan_tag_t ctag_2;
@pragma pa_header_union ingress ipv4_2 ipv6_2
header ipv4_t ipv4_2;
header ipv6_t ipv6_2;
header udp_t udp_2;
header vxlan_t vxlan_2;

// layer 3
header ethernet_t ethernet_3;
header vlan_tag_t ctag_3;
@pragma pa_header_union ingress ipv4_3 ipv6_3
header ipv4_t ipv4_3;
header ipv6_t ipv6_3;
@pragma pa_header_union ingress udp_3 tcp icmp
header udp_t udp_3;
header tcp_t tcp;
header icmp_t icmp;

/******************************************************************************
 * Parser OHI                                                                 *
 *****************************************************************************/
header_type parser_ohi_t {
    fields {
        ipv4_1_len  : 16;
        ipv4_2_len  : 16;
        ipv4_3_len  : 16;
    }
}
@pragma parser_write_only
metadata parser_ohi_t ohi;

/******************************************************************************
 * Parser metadata                                                            *
 *****************************************************************************/
header_type parser_metadata_t {
    fields {
        mirror_blob_len : 8;
        cps_blob_len    : 16;
    }
}
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

/******************************************************************************
 * Parser start                                                               *
 *****************************************************************************/
parser start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_INGRESS : parse_ingress_recirc_header;
        TM_PORT_DMA : parse_txdma_to_ingress;
        TM_PORT_EGRESS : parse_egress_to_ingress;
        default : parse_uplink;
        0x1 mask 0 : deparse_ingress;
        0x2 mask 0 : egress_start;
    }
}

@pragma xgress ingress
parser parse_uplink {
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_UPLINK_0 : parse_packet_from_host;
        TM_PORT_UPLINK_1 : parse_packet_from_switch;
        default : parse_ingress_packet;
    }
}

@pragma xgress ingress
parser parse_ingress_recirc_header {
    extract(capri_p4_intrinsic);
    extract(ingress_recirc);
    return parse_ingress_pass2;
}

@pragma xgress ingress
parser parse_egress_to_ingress {
    extract(capri_p4_intrinsic);
    return parse_ingress_pass2;
}

@pragma xgress ingress
parser parse_txdma_to_ingress {
    set_metadata(parser_metadata.cps_blob_len, 0xFF);
    extract(capri_txdma_intrinsic);
    return select(current(72, 4)) {
        0 : parse_txdma_cps;
        default : parse_txdma_app;
    }
}

@pragma xgress ingress
parser parse_txdma_cps {
    set_metadata(parser_metadata.cps_blob_len,
                 (parser_metadata.cps_blob_len +
                  (ARTEMIS_TXDMA_TO_P4I_HDR_SZ - 0xFF)));
    return parse_txdma_cps_blob;
}

@pragma xgress ingress
parser parse_txdma_cps_blob {
    set_metadata(parser_metadata.cps_blob_len,
                 parser_metadata.cps_blob_len + 0);
    extract(cps_blob);
    return parse_ingress_pass2;
}

@pragma xgress ingress
parser parse_txdma_app {
    extract(p4plus_to_p4);
    extract(p4plus_to_p4_vlan);
    return select(p4plus_to_p4.p4plus_app_id) {
        P4PLUS_APPTYPE_CPU : parse_cpu_packet;
        default : parse_packet_from_host;
    }
}

@pragma xgress ingress
parser parse_cpu_packet {
    return parse_ingress_pass2;
}

@pragma xgress ingress
parser parse_ingress_pass2 {
    extract(predicate_header);
    return select(predicate_header.direction) {
        RX_FROM_SWITCH : parse_packet_from_switch;
        TX_FROM_HOST : parse_packet_from_host;
        default : ingress;
    }
}

@pragma xgress ingress
parser parse_packet_from_host {
    set_metadata(control_metadata.direction, TX_FROM_HOST);
    return parse_ingress_packet;
}

@pragma xgress ingress
parser parse_packet_from_switch {
    set_metadata(control_metadata.direction, RX_FROM_SWITCH);
    return parse_ingress_packet;
}

/******************************************************************************
 * Layer 1                                                                    *
 *****************************************************************************/
parser parse_ingress_packet {
    extract(ethernet_1);
    set_metadata(offset_metadata.l2_1, current + 0);
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
    set_metadata(offset_metadata.l3_1, current + 0);
    set_metadata(ohi.ipv4_1_len, ipv4_1.ihl << 2);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_1;
        default: ingress;
    }
}

parser parse_ipv6_1 {
    extract(ipv6_1);
    set_metadata(offset_metadata.l3_1, current + 0);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_1;
        default : ingress;
    }
}

parser parse_icmp {
    extract(icmp);
    set_metadata(offset_metadata.l4_3, current + 0);
    return ingress;
}

parser parse_tcp {
    extract(tcp);
    set_metadata(offset_metadata.l4_3, current + 0);
    set_metadata(key_metadata.sport, latest.srcPort);
    set_metadata(key_metadata.dport, latest.dstPort);
    return ingress;
}

parser parse_udp_1 {
    extract(udp_1);
    set_metadata(offset_metadata.l4_1, current + 0);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_1;
        default: ingress;
    }
}

parser parse_vxlan_1 {
    extract(vxlan_1);
    return parse_ethernet_2;
}

/******************************************************************************
 * Layer 2                                                                    *
 *****************************************************************************/
parser parse_ethernet_2 {
    extract(ethernet_2);
    set_metadata(offset_metadata.l2_2, current + 0);
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
    set_metadata(offset_metadata.l3_2, current + 0);
    set_metadata(ohi.ipv4_2_len, ipv4_2.ihl << 2);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_2;
        default: ingress;
    }
}

parser parse_ipv6_2 {
    extract(ipv6_2);
    set_metadata(offset_metadata.l3_2, current + 0);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_2;
        default : ingress;
    }
}

parser parse_udp_2 {
    extract(udp_2);
    set_metadata(offset_metadata.l4_2, current + 0);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_2;
        default: ingress;
    }
}


parser parse_vxlan_2 {
    extract(vxlan_2);
    return parse_ethernet_3;
}

/******************************************************************************
 * Layer 3                                                                    *
 *****************************************************************************/
parser parse_ethernet_3 {
    set_metadata(offset_metadata.l2_3, current + 0);
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
    set_metadata(offset_metadata.l3_3, current + 0);
    set_metadata(ohi.ipv4_3_len, ipv4_3.ihl << 2);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_3;
        default: ingress;
    }
}

parser parse_ipv6_3 {
    extract(ipv6_3);
    set_metadata(offset_metadata.l3_3, current + 0);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmp;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp_3;
        default : ingress;
    }
}

parser parse_udp_3 {
    extract(udp_3);
    set_metadata(offset_metadata.l4_3, current + 0);
    set_metadata(key_metadata.sport, latest.srcPort);
    set_metadata(key_metadata.dport, latest.dstPort);
    return ingress;
}

/******************************************************************************/
/* Ingress deparser                                                           */
/******************************************************************************/
@pragma deparse_only
@pragma xgress ingress
parser deparse_ingress {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    extract(ingress_recirc);
    extract(capri_rxdma_intrinsic);

    extract(p4_to_p4plus_classic_nic);
    extract(p4_to_p4plus_classic_nic_ip);
    // splitter offset here for arm path
    extract(p4_to_arm);
    extract(cps_blob);

    extract(p4_to_rxdma);
    extract(p4_to_rxdma2);
    extract(p4_to_rxdma3);
    extract(predicate_header);
    // splitter offset here for cps path

    extract(txdma_to_p4e);
    extract(p4i_i2e);

    return parse_ingress_packet;
}

/******************************************************************************/
/* Egress parser                                                              */
/******************************************************************************/
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
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_egress_span_copy;
        default : parse_egress;
    }
}

@pragma xgress egress
parser parse_egress_common {
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_ingress_span_copy;
        default : parse_egress;
    }
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.direction
parser parse_egress {
    extract(predicate_header);
    return select(predicate_header.direction) {
        RX_FROM_SWITCH : parse_egress_predicate_header_rx;
        TX_FROM_HOST : parse_egress_predicate_header_tx;
        default : ingress;
    }
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.direction
parser parse_egress_predicate_header_rx {
    set_metadata(control_metadata.direction, RX_FROM_SWITCH);
    extract(txdma_to_p4e);
    return parse_i2e_metadata;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.direction
parser parse_egress_predicate_header_tx {
    set_metadata(control_metadata.direction, TX_FROM_HOST);
    extract(txdma_to_p4e);
    return parse_i2e_metadata;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_egress_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    return parse_ethernet_span_copy;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_ingress_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    set_metadata(parser_metadata.mirror_blob_len, ARTEMIS_INGRESS_MIRROR_BLOB_SZ);
    return parse_span_copy_blob;
}

@pragma xgress egress
parser parse_span_copy_blob {
    set_metadata(parser_metadata.mirror_blob_len,
                 parser_metadata.mirror_blob_len + 0);
    extract(mirror_blob);
    return parse_ethernet_span_copy;
}

@pragma xgress egress
@pragma capture_payload_offset
@pragma allow_set_meta offset_metadata.l2_1
parser parse_ethernet_span_copy {
    extract(ethernet_1);
    set_metadata(offset_metadata.l2_1, current + 0);
    return select(latest.etherType) {
        ETHERTYPE_VLAN : parse_ctag_span_copy;
        default : ingress;
    }
}

@pragma xgress egress
@pragma dont_capture_payload_offset
parser parse_ctag_span_copy {
    extract(ctag_1);
    return ingress;
}

@pragma xgress egress
parser parse_i2e_metadata {
    extract(p4e_i2e);
    return parse_egress_packet;
}

@pragma xgress egress
@pragma allow_set_meta offset_metadata.l2_1
parser parse_egress_packet {
    extract(ethernet_1);
    set_metadata(offset_metadata.l2_1, current + 0);
    return select(latest.etherType) {
        ETHERTYPE_CTAG : parse_egress_ctag_1;
        ETHERTYPE_IPV4 : parse_egress_ipv4_1;
        ETHERTYPE_IPV6 : parse_egress_ipv6_1;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_egress_ctag_1 {
    extract(ctag_1);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_egress_ipv4_1;
        ETHERTYPE_IPV6 : parse_egress_ipv6_1;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_egress_ipv4_1 {
    extract(ipv4_1);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTO_ICMP : parse_egress_icmp;
        IP_PROTO_TCP : parse_egress_tcp;
        IP_PROTO_UDP : parse_egress_udp;
        default: ingress;
    }
}

@pragma xgress egress
parser parse_egress_ipv6_1 {
    extract(ipv6_1);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_egress_icmp;
        IP_PROTO_TCP : parse_egress_tcp;
        IP_PROTO_UDP : parse_egress_udp;
        default : ingress;
    }
}

@pragma xgress egress
parser parse_egress_icmp {
    extract(icmp);
    return ingress;
}

@pragma xgress egress
parser parse_egress_tcp {
    extract(tcp);
    return ingress;
}

@pragma xgress egress
parser parse_egress_udp {
    extract(udp_1);
    return ingress;
}

/******************************************************************************
 * Egress deparser                                                            *
 *****************************************************************************/
@pragma deparse_only
@pragma xgress egress
parser deparse_egress {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    extract(capri_rxdma_intrinsic);

    // Below are headers used in case of egress-to-egress recirc
    extract(predicate_header);
    extract(txdma_to_p4e);
    extract(p4e_i2e);

    // layer 0
    extract(ethernet_0);
    extract(ctag_0);
    extract(ipv4_0);
    extract(ipv6_0);
    extract(udp_0);
    extract(vxlan_0);
    extract(gre_0);
    extract(erspan);

    return parse_egress_packet;
}

/******************************************************************************
 * Checksums : Layer 0 (compute only, no verification)                        *
 *****************************************************************************/
field_list ipv4_0_checksum_list {
    ipv4_0.version;
    ipv4_0.ihl;
    ipv4_0.diffserv;
    ipv4_0.totalLen;
    ipv4_0.identification;
    ipv4_0.flags;
    ipv4_0.fragOffset;
    ipv4_0.ttl;
    ipv4_0.protocol;
    ipv4_0.srcAddr;
    ipv4_0.dstAddr;
}

@pragma checksum update_len capri_deparser_len.ipv4_0_hdr_len
@pragma checksum gress egress
field_list_calculation ipv4_0_checksum {
    input {
        ipv4_0_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_0.hdrChecksum  {
    update ipv4_0_checksum;
}

/******************************************************************************
 * Checksums : Layer 1                                                        *
 *****************************************************************************/
field_list ipv4_1_checksum_list {
    ipv4_1.version;
    ipv4_1.ihl;
    ipv4_1.diffserv;
    ipv4_1.totalLen;
    ipv4_1.identification;
    ipv4_1.flags;
    ipv4_1.fragOffset;
    ipv4_1.ttl;
    ipv4_1.protocol;
    ipv4_1.srcAddr;
    ipv4_1.dstAddr;
}

@pragma checksum hdr_len_expr ohi.ipv4_1_len + 0
@pragma checksum verify_len ohi.ipv4_1_len
@pragma checksum update_len capri_deparser_len.ipv4_1_hdr_len
field_list_calculation ipv4_1_checksum {
    input {
        ipv4_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_1.hdrChecksum  {
    verify ipv4_1_checksum;
    update ipv4_1_checksum;
}

/******************************************************************************
 * Checksums : Layer 2 (verify only)                                          *
 *****************************************************************************/
field_list ipv4_2_checksum_list {
    ipv4_2.version;
    ipv4_2.ihl;
    ipv4_2.diffserv;
    ipv4_2.totalLen;
    ipv4_2.identification;
    ipv4_2.flags;
    ipv4_2.fragOffset;
    ipv4_2.ttl;
    ipv4_2.protocol;
    ipv4_2.srcAddr;
    ipv4_2.dstAddr;
}

@pragma checksum hdr_len_expr ohi.ipv4_2_len + 0
@pragma checksum verify_len ohi.ipv4_2_len
@pragma checksum gress ingress
field_list_calculation ipv4_2_checksum {
    input {
        ipv4_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_2.hdrChecksum  {
    verify ipv4_2_checksum;
}

/******************************************************************************
 * Checksums : Layer 3 (verify only)                                          *
 *****************************************************************************/
field_list ipv4_3_checksum_list {
    ipv4_3.version;
    ipv4_3.ihl;
    ipv4_3.diffserv;
    ipv4_3.totalLen;
    ipv4_3.identification;
    ipv4_3.flags;
    ipv4_3.fragOffset;
    ipv4_3.ttl;
    ipv4_3.protocol;
    ipv4_3.srcAddr;
    ipv4_3.dstAddr;
}

@pragma checksum hdr_len_expr ohi.ipv4_3_len + 0
@pragma checksum verify_len ohi.ipv4_3_len
@pragma checksum gress ingress
field_list_calculation ipv4_3_checksum {
    input {
        ipv4_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_3.hdrChecksum  {
    verify ipv4_3_checksum;
}
