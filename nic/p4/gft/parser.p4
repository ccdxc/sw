/******************************************************************************
 * Parser local definitions
 *****************************************************************************/
header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len           : 16;
        icrc_payload_len        : 16;
        rx_l4_payload_len       : 16;
        tx_l4_payload_len       : 16;
        udp_opt_l2_checksum_len : 16;
    }
}

header_type parser_metadata_t {
    fields {
        icrc_len                        : 16;
        icrc_len_1                      : 16;
        icrc_len_2                      : 16;
        icrc                            : 32;
        l4_trailer                      : 16;
        l4_len                          : 16;
        opCode                          : 8;
    }
}

@pragma dont_trim
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

header_type parser_ohi_t {
    fields {
        ipv4_1___hdr_len              : 16;
        ipv4_1___start_off            : 16;
        ipv6_1___start_off            : 16;
        udp_1___start_off             : 16;
        tcp_1___start_off             : 16;
        ipv4_2___hdr_len              : 16;
        ipv4_2___start_off            : 16;
        ipv6_2___start_off            : 16;
        udp_2___start_off             : 16;
        tcp_2___start_off             : 16;
        ipv4_3___hdr_len              : 16;
        ipv4_3___start_off            : 16;
        ipv6_3___start_off            : 16;
        udp_3___start_off             : 16;
        tcp_3___start_off             : 16;
        l4_1_len                      : 16;
        l4_2_len                      : 16;
        l4_3_len                      : 16;
        icrc_len                      : 16;
        kind                          : 16;
        chksum                        : 16;
        ipv4_00___hdr_len             : 16;
        ipv4_00___start_off           : 16;
        ipv6_00___start_off           : 16;
        udp_00___start_off            : 16;
        ipv4_01___hdr_len             : 16;
        ipv4_01___start_off           : 16;
        ipv6_01___start_off           : 16;
        udp_01___start_off            : 16;
    }
}

@pragma dont_trim
@pragma parser_write_only
@pragma parser_share_ohi icrc_1 ipv4_1___start_off ipv6_1___start_off
@pragma parser_share_ohi icrc_2 ipv4_2___start_off ipv6_2___start_off
@pragma parser_share_ohi icrc_3 ipv4_3___start_off ipv6_3___start_off
metadata parser_ohi_t ohi;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t capri_deparser_len;

/******************************************************************************
 * Capri Intrinsic header definitions
 *****************************************************************************/
header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;
header capri_i2e_metadata_t capri_i2e_metadata;

/******************************************************************************
 * Header definitions
 *****************************************************************************/
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
@pragma pa_header_union xgress p4_to_p4plus_roce_eth_1
header ethernet_t ethernet_1;
header vlan_tag_t ctag_1;
@pragma pa_header_union xgress ipv4_1 ipv6_1 p4_to_p4plus_roce_ip_1
header ipv4_t ipv4_1;
header ipv6_t ipv6_1;
@pragma pa_header_union xgress udp_1 tcp_1 icmp_1
header udp_t udp_1;
header tcp_t tcp_1;
header icmp_t icmp_1;
header vxlan_t vxlan_1;
header gre_t gre_1;

// layer 2
@pragma pa_header_union ingress p4_to_p4plus_roce_eth_2
header ethernet_t ethernet_2;
header vlan_tag_t ctag_2;
@pragma pa_header_union ingress ipv4_2 ipv6_2 p4_to_p4plus_roce_ip_2
header ipv4_t ipv4_2;
header ipv6_t ipv6_2;
@pragma pa_header_union ingress udp_2 tcp_2 icmp_2
header udp_t udp_2;
header tcp_t tcp_2;
header icmp_t icmp_2;
header vxlan_t vxlan_2;
header gre_t gre_2;

// layer 3
@pragma pa_header_union ingress p4_to_p4plus_roce_eth_3
header ethernet_t ethernet_3;
header vlan_tag_t ctag_3;
@pragma pa_header_union ingress ipv4_3 ipv6_3 p4_to_p4plus_roce_ip_3
header ipv4_t ipv4_3;
header ipv6_t ipv6_3;
@pragma pa_header_union ingress udp_3 tcp_3 icmp_3
header udp_t udp_3;
header tcp_t tcp_3;
header icmp_t icmp_3;
header vxlan_t vxlan_3;
header gre_t gre_3;

// UDP payload and options
@pragma hdr_len parser_metadata.l4_len
header udp_payload_t udp_payload;
@pragma no_ohi ingress
header udp_opt_eol_t udp_opt_eol;
@pragma no_ohi ingress
header udp_opt_nop_t udp_opt_nop;
@pragma no_ohi ingress
header udp_opt_ocs_t udp_opt_ocs;
@pragma no_ohi ingress
header udp_opt_mss_t udp_opt_mss;
@pragma no_ohi ingress
header udp_opt_timestamp_t udp_opt_timestamp;
header udp_opt_unknown_t udp_opt_unknown;

// roce headers
header roce_bth_t roce_bth;
header roce_deth_t roce_deth;
header roce_deth_immdt_t roce_deth_immdt;
header icrc_t icrc;

@pragma no_ohi xgress
header roce_bth_t roce_bth_1;
@pragma no_ohi xgress
header roce_deth_t roce_deth_1;
//TODO: roce_deth_immdt_1 cannot go into PHV due to parser flit violation.
//      When this hdr goes into OHI, it takes away ohi instruction
//      leading to instruction exhaustion in parse state sparse_bth_deth_immdt.
//      For now udp.len is not loaded into ohi ; instead l4 len be set to zero.
//      This implies incoming packet from uplink shoud have udp.csum = 0 for
//      roce pkts hitting optimized path. When udp.checksum is non zero,
//      checksum error will be set. It upto MPU pipeline to ignore udp.csum error.
header roce_deth_immdt_t roce_deth_immdt_1;

@pragma no_ohi xgress
header roce_bth_t roce_bth_2;
@pragma no_ohi xgress
header roce_deth_t roce_deth_2;
@pragma no_ohi xgress
header roce_deth_immdt_t roce_deth_immdt_2;

// p4 to p4plus headers
@pragma synthetic_header
@pragma pa_field_union ingress p4_to_p4plus_roce.roce_opt_ts_value udp_opt_timestamp.ts_value
@pragma pa_field_union ingress p4_to_p4plus_roce.roce_opt_ts_echo  udp_opt_timestamp.ts_echo
@pragma pa_field_union ingress p4_to_p4plus_roce.roce_opt_mss      udp_opt_mss.mss
header p4_to_p4plus_roce_header_t p4_to_p4plus_roce;

@pragma synthetic_header
header ethernet_t p4_to_p4plus_roce_eth_1;
@pragma synthetic_header
header ipv6_t p4_to_p4plus_roce_ip_1;
@pragma synthetic_header
header ethernet_t p4_to_p4plus_roce_eth_2;
@pragma synthetic_header
header ipv6_t p4_to_p4plus_roce_ip_2;
@pragma synthetic_header
header ethernet_t p4_to_p4plus_roce_eth_3;
@pragma synthetic_header
header ipv6_t p4_to_p4plus_roce_ip_3;

// p4plus to p4 headers
header p4plus_to_p4_header_t p4plus_to_p4;

parser start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.csum_err) {
        0 : parse_ethernet_1;
        1 : start_ipv4_bth_deth;
        2 : start_vlan_ipv4_bth_deth;
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
        0x1 mask 0 : rx_deparse_start;
    }
}

@pragma deparse_only
parser rx_deparse_start {
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);
    extract(capri_rxdma_intrinsic);
    extract(p4_to_p4plus_roce);
#ifdef TXHACK
    extract(capri_txdma_intrinsic);
    extract(p4plus_to_p4);
#endif
    return parse_nic;
}

parser parse_nic {
    return parse_ethernet_1;
}

/******************************************************************************
 * RoCE optimization
 *****************************************************************************/
parser parse_bth_deth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(roce_bth_1);
    extract(roce_deth_1);
    return ingress;
}
parser parse_bth_deth_immdt {
    extract(udp_1);
    // This state needs 5 ohi instructions. Due to lack of instructions
    // and to keep roce pkt path optimized, udp.checksum of incoming
    // packet should be zero because udp.len is not used to verify checksum
    // instead zero is used.
    //set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(roce_bth_1);
    extract(roce_deth_immdt_1);
    return ingress;
}
parser parse_bth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(roce_bth_1);
    return ingress;
}
parser start_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth;
}
parser start_vlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth;
}
parser start_ipv6_bth_deth {
    extract(ethernet_1);
    return start_ipv6_bth_deth_split;
}
parser start_ipv6_bth_deth_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth;
}
parser start_vlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    return start_vlan_ipv6_bth_deth_split;
}
parser start_vlan_ipv6_bth_deth_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth;
}
parser start_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt;
}
parser start_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    return start_ipv6_bth_deth_immdt_split;
}
parser start_ipv6_bth_deth_immdt_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    return start_vlan_ipv6_bth_deth_immdt_split;
}
parser start_vlan_ipv6_bth_deth_immdt_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt;
}
parser start_ipv4_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth;
}
parser start_vlan_ipv4_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth;
}
parser start_ipv6_bth {
    extract(ethernet_1);
    return start_ipv6_bth_split;
}
parser start_ipv6_bth_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth;
}
parser start_vlan_ipv6_bth {
    extract(ethernet_1);
    extract(ctag_1);
    return start_vlan_ipv6_bth_split;
}
parser start_vlan_ipv6_bth_split {
    extract(ipv6_1);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth;
}

// tunneled roce
parser parse_bth_deth_2 {
    extract(udp_2);
    set_metadata(ohi.l4_2_len, udp_2.len + 0);
    set_metadata(ohi.icrc_len, udp_2.len + parser_metadata.icrc_len);
    extract(roce_bth_2);
    extract(roce_deth_2);
    return ingress;
}
parser parse_bth_deth_immdt_2 {
    extract(udp_2);
    set_metadata(ohi.l4_2_len, udp_2.len + 0);
    set_metadata(ohi.icrc_len, udp_2.len + parser_metadata.icrc_len);
    extract(roce_bth_2);
    extract(roce_deth_immdt_2);
    return ingress;
}
parser parse_bth_2 {
    extract(udp_2);
    set_metadata(ohi.l4_2_len, udp_2.len + 0);
    set_metadata(ohi.icrc_len, udp_2.len + parser_metadata.icrc_len);
    extract(roce_bth_2);
    return ingress;
}

parser start_outer_vxlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vxlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vxlan_ipv4_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vlan_vxlan_ipv4_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vxlan_ipv6_bth {
    extract(ethernet_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth;
}
parser start_outer_vlan_vxlan_ipv6_bth {
    extract(ethernet_1);
    extract(ctag_1);
    extract(ipv4_1);
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_vxlan_ipv6_bth;
}

parser parse_vxlan_ipv4_bth_deth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_deth_split;
}
parser parse_vxlan_ipv4_bth_deth_split {
    extract(ipv4_2);
    set_metadata(ohi.ipv4_2___hdr_len, (ipv4_2.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_2;
}
parser parse_vxlan_ipv6_bth_deth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_deth_split;
}
parser parse_vxlan_ipv6_bth_deth_split {
    extract(ipv6_2);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_2;
}
parser parse_vxlan_ipv4_bth_deth_immdt {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_deth_immdt_split;
}

parser parse_vxlan_ipv4_bth_deth_immdt_split {
    extract(ipv4_2);
    set_metadata(ohi.ipv4_2___hdr_len, (ipv4_2.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt_2;
}
parser parse_vxlan_ipv6_bth_deth_immdt {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_deth_immdt_split;
}
parser parse_vxlan_ipv6_bth_deth_immdt_split {
    extract(ipv6_2);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_deth_immdt_2;
}
parser parse_vxlan_ipv4_bth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv4_bth_split;
}
parser parse_vxlan_ipv4_bth_split {
    extract(ipv4_2);
    set_metadata(ohi.ipv4_2___hdr_len, (ipv4_2.ihl << 2));
    set_metadata(parser_metadata.icrc_len, 28); //std ipv4 hdr size + 8 bytes of icrc invariant
    return parse_bth_2;
}
parser parse_vxlan_ipv6_bth {
    extract(udp_1);
    set_metadata(ohi.l4_1_len, udp_1.len + 0);
    set_metadata(ohi.icrc_len, udp_1.len + parser_metadata.icrc_len);
    extract(vxlan_1);
    extract(ethernet_2);
    return parse_vxlan_ipv6_bth_split;
}
parser parse_vxlan_ipv6_bth_split {
    extract(ipv6_2);
    set_metadata(parser_metadata.icrc_len, 48); //std ipv4 hdr size + 8 bytes of icrc invariant
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
    set_metadata(ohi.ipv4_1___hdr_len, (ipv4_1.ihl << 2));
    set_metadata(ohi.l4_1_len, ipv4_1.totalLen + 0);
    set_metadata(parser_metadata.l4_trailer, ipv4_1.totalLen - (ipv4_1.ihl << 2));
    set_metadata(parser_metadata.icrc_len_1, (ipv4_1.ihl << 2)); //ipv4 hdr size; 8 bytes of icrc invariant to be added
    return select(latest.protocol) {
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
    set_metadata(parser_metadata.l4_trailer, latest.payloadLen);
    set_metadata(ohi.l4_1_len, ipv6_1.payloadLen + 0);
    set_metadata(parser_metadata.icrc_len_1, 40); //std ipv6 hdr size; 8 bytes of icrc invariant to be added.
                                                //v6-option size will be added later in the parse graph
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
    set_metadata(ohi.l4_1_len, parser_metadata.l4_trailer + 0);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return ingress;
}

parser parse_udp_1 {
    set_metadata(parser_metadata.l4_len, current(32, 16) + 0);
    set_metadata(ohi.l4_1_len, current(32, 16) + 0);
    set_metadata(ohi.icrc_len, current(32, 16) + parser_metadata.icrc_len_1 + 8); // 8 bytes of invariant
    return parse_udp_1_split;
}
parser parse_udp_1_split {
    extract(udp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_1;
        UDP_PORT_ROCE_V2: parse_roce_v2;
        default: ingress;
    }
}

parser parse_gre_1 {
    extract(gre_1);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        ETHERTYPE_IPV4 : parse_gre_ipv4_1;
        ETHERTYPE_IPV6 : parse_gre_ipv6_1;
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
    set_metadata(ohi.ipv4_2___hdr_len, (ipv4_2.ihl << 2));
    set_metadata(ohi.l4_2_len, ipv4_2.totalLen + 0);
    set_metadata(parser_metadata.l4_trailer, ipv4_2.totalLen - (ipv4_2.ihl << 2));
    set_metadata(parser_metadata.icrc_len_2, (ipv4_2.ihl << 2)); //ipv4 hdr size; 8 bytes of icrc invariant to be added
    return select(latest.protocol) {
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
    set_metadata(ohi.l4_2_len, ipv6_2.payloadLen + 0);
    set_metadata(parser_metadata.l4_trailer, latest.payloadLen);
    set_metadata(parser_metadata.icrc_len_2, 40); //std ipv6 hdr size; 8 bytes of icrc invariant to be added
                                                //v6-option size will be added later in the parse graph
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
    set_metadata(ohi.l4_2_len, parser_metadata.l4_trailer + 0);
    set_metadata(l4_metadata.l4_sport_2, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_2, latest.dstPort);
    return ingress;
}

parser parse_udp_2 {
    set_metadata(parser_metadata.l4_len, current(32, 16) + 0);
    set_metadata(ohi.l4_2_len, current(32, 16) + 0);
    set_metadata(ohi.icrc_len, current(32, 16) + parser_metadata.icrc_len_2 + 8); // 8 bytes of invariant
    return parse_udp_2_split;
}

parser parse_udp_2_split {
    extract(udp_2);
    set_metadata(l4_metadata.l4_sport_2, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_2, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_2;
        UDP_PORT_ROCE_V2: parse_roce_v2;
        default: ingress;
    }
}

parser parse_gre_2 {
    extract(gre_2);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        ETHERTYPE_IPV4 : parse_gre_ipv4_2;
        ETHERTYPE_IPV6 : parse_gre_ipv6_2;
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
    set_metadata(parser_metadata.l4_trailer, ipv4_3.totalLen - (ipv4_3.ihl << 2));
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
    set_metadata(parser_metadata.l4_trailer, latest.payloadLen);
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
    set_metadata(ohi.l4_3_len, parser_metadata.l4_trailer + 0);
    set_metadata(l4_metadata.l4_sport_3, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_3, latest.dstPort);
    return ingress;
}

parser parse_udp_3 {
    extract(udp_3);
    set_metadata(parser_metadata.l4_len, udp_3.len);
    set_metadata(l4_metadata.l4_sport_3, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_3, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan_3;
        UDP_PORT_ROCE_V2: parse_roce_v2;
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

parser parse_vxlan_3 {
    extract(vxlan_3);
    set_metadata(tunnel_metadata.tunnel_type_3, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni_3, latest.vni);
    return parse_end;
}

parser parse_roce_v2 {
    extract(roce_bth);
    set_metadata(parser_metadata.l4_trailer,
                 parser_metadata.l4_trailer - parser_metadata.l4_len);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len - 24);
    return select(latest.opCode) {
        0x64 : parse_roce_deth;
        0x65 : parse_roce_deth_immdt;
        default : parse_udp_payload;
    }
}

parser parse_roce_deth {
    extract(roce_deth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len - 8);
    return select(latest.reserved) {
        default : parse_udp_payload;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

parser parse_roce_deth_immdt {
    extract(roce_deth_immdt);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len - 12);
    return select(latest.reserved) {
        default : parse_udp_payload;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

parser parse_udp_payload {
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len + 0);
    extract(udp_payload);
    return select(parser_metadata.l4_trailer) {
        0x0000 mask 0xffff: ingress;
        default : parse_udp_trailer;
    }
}

parser parse_udp_trailer {
    extract(icrc);
    return parse_udp_options;
}

@pragma header_ordering udp_opt_ocs udp_opt_mss udp_opt_timestamp udp_opt_nop udp_opt_unknown
parser parse_udp_options {
    return select (current(0,8)) {
       UDP_KIND_EOL : parse_udp_option_eol;
       UDP_KIND_NOP : parse_udp_option_nop;
       UDP_KIND_OCS : parse_udp_option_ocs;
       UDP_KIND_MSS : parse_udp_option_mss;
       UDP_KIND_TIMESTAMP : parse_udp_option_timestamp;
       default : parse_udp_option_unknown;
    }
}

parser parse_udp_option_eol {
    extract(udp_opt_eol);
    return ingress;
}

parser parse_udp_option_nop {
    extract(udp_opt_nop);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 1);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

parser parse_udp_option_ocs {
    extract(udp_opt_ocs);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 2);
    set_metadata(ohi.kind, udp_opt_ocs.kind + 0);
    set_metadata(ohi.chksum, udp_opt_ocs.chksum + 0);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

parser parse_udp_option_mss {
    extract(udp_opt_mss);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 4);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

parser parse_udp_option_timestamp {
    extract(udp_opt_timestamp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 10);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

@pragma no_extract
parser parse_udp_option_unknown {
    extract(udp_opt_unknown);
    set_metadata(parser_metadata.l4_trailer,
                 parser_metadata.l4_trailer - udp_opt_unknown.len);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

@pragma deparse_only
parser parse_roce_eth {
    extract(p4_to_p4plus_roce_eth_1);
    extract(p4_to_p4plus_roce_ip_1);
    extract(p4_to_p4plus_roce_eth_2);
    extract(p4_to_p4plus_roce_ip_2);
    extract(p4_to_p4plus_roce_eth_3);
    extract(p4_to_p4plus_roce_ip_3);
    return ingress;
}

parser parse_end {
    return ingress;
}


/******************************************************************************
 * Checksum  in Rx Path
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

field_list ipv4_1_udp_1_checksum_list {
    ipv4_1.srcAddr;
    ipv4_1.dstAddr;
    8'0;
    ipv4_1.protocol;
    udp_1.len;
    udp_1.srcPort;
    udp_1.dstPort;
    payload;
}

field_list ipv4_2_udp_2_checksum_list {
    ipv4_2.srcAddr;
    ipv4_2.dstAddr;
    8'0;
    ipv4_2.protocol;
    udp_2.len;
    udp_2.srcPort;
    udp_2.dstPort;
    payload;
}

field_list ipv4_3_udp_3_checksum_list {
    ipv4_3.srcAddr;
    ipv4_3.dstAddr;
    8'0;
    ipv4_3.protocol;
    udp_3.len;
    udp_3.srcPort;
    udp_3.dstPort;
    payload;
}

field_list ipv4_1_tcp_1_checksum_list {
    ipv4_1.srcAddr;
    ipv4_1.dstAddr;
    8'0;
    ipv4_1.protocol;
    capri_deparser_len.tx_l4_payload_len;
    tcp_1.srcPort;
    tcp_1.dstPort;
    tcp_1.seqNo;
    tcp_1.ackNo;
    tcp_1.dataOffset;
    tcp_1.res;
    tcp_1.flags;
    tcp_1.window;
    tcp_1.urgentPtr;
    payload;
}

field_list ipv4_2_tcp_2_checksum_list {
    ipv4_2.srcAddr;
    ipv4_2.dstAddr;
    8'0;
    ipv4_2.protocol;
    capri_deparser_len.tx_l4_payload_len;
    tcp_2.srcPort;
    tcp_2.dstPort;
    tcp_2.seqNo;
    tcp_2.ackNo;
    tcp_2.dataOffset;
    tcp_2.res;
    tcp_2.flags;
    tcp_2.window;
    tcp_2.urgentPtr;
    payload;
}

field_list ipv4_3_tcp_3_checksum_list {
    ipv4_3.srcAddr;
    ipv4_3.dstAddr;
    8'0;
    ipv4_3.protocol;
    capri_deparser_len.tx_l4_payload_len;
    tcp_3.srcPort;
    tcp_3.dstPort;
    tcp_3.seqNo;
    tcp_3.ackNo;
    tcp_3.dataOffset;
    tcp_3.res;
    tcp_3.flags;
    tcp_3.window;
    tcp_3.urgentPtr;
    payload;
}

field_list ipv6_1_udp_1_checksum_list {
    ipv6_1.srcAddr;
    ipv6_1.dstAddr;
    8'0;
    ipv6_1.nextHdr;
    udp_1.srcPort;
    udp_1.dstPort;
    payload;
}

field_list ipv6_2_udp_2_checksum_list {
    ipv6_2.srcAddr;
    ipv6_2.dstAddr;
    8'0;
    ipv6_2.nextHdr;
    udp_2.srcPort;
    udp_2.dstPort;
    payload;
}

field_list ipv6_3_udp_3_checksum_list {
    ipv6_3.srcAddr;
    ipv6_3.dstAddr;
    8'0;
    ipv6_3.nextHdr;
    udp_3.srcPort;
    udp_3.dstPort;
    payload;
}

field_list ipv6_1_tcp_1_checksum_list {
    ipv6_1.srcAddr;
    ipv6_1.dstAddr;
    8'0;
    ipv6_1.nextHdr;
    capri_deparser_len.tx_l4_payload_len;
    tcp_1.srcPort;
    tcp_1.dstPort;
    tcp_1.seqNo;
    tcp_1.ackNo;
    tcp_1.dataOffset;
    tcp_1.res;
    tcp_1.flags;
    tcp_1.window;
    tcp_1.urgentPtr;
    payload;
}

field_list ipv6_2_tcp_2_checksum_list {
    ipv6_2.srcAddr;
    ipv6_2.dstAddr;
    8'0;
    ipv6_2.nextHdr;
    capri_deparser_len.tx_l4_payload_len;
    tcp_2.srcPort;
    tcp_2.dstPort;
    tcp_2.seqNo;
    tcp_2.ackNo;
    tcp_2.dataOffset;
    tcp_2.res;
    tcp_2.flags;
    tcp_2.window;
    tcp_2.urgentPtr;
    payload;
}

field_list ipv6_3_tcp_3_checksum_list {
    ipv6_3.srcAddr;
    ipv6_3.dstAddr;
    8'0;
    ipv6_3.nextHdr;
    capri_deparser_len.tx_l4_payload_len;
    tcp_3.srcPort;
    tcp_3.dstPort;
    tcp_3.seqNo;
    tcp_3.ackNo;
    tcp_3.dataOffset;
    tcp_3.res;
    tcp_3.flags;
    tcp_3.window;
    tcp_3.urgentPtr;
    payload;
}

@pragma checksum hdr_len_expr ohi.ipv4_1___hdr_len + 0
@pragma checksum verify_len ohi.l4_1_len
@pragma checksum gress ingress
field_list_calculation rx_ipv4_1_checksum {
    input {
        ipv4_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_1.hdrChecksum  {
    verify rx_ipv4_1_checksum;
    update rx_ipv4_1_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len  ohi.l4_1_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_1_udp_1_checksum {
    input {
        ipv4_1_udp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_1_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_1_udp_1_checksum {
    input {
        ipv6_1_udp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_1.checksum {
    verify rx_ipv4_1_udp_1_checksum;
    verify rx_ipv6_1_udp_1_checksum;
    update rx_ipv4_1_udp_1_checksum;
    update rx_ipv6_1_udp_1_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_1_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_1_tcp_1_checksum {
    input {
        ipv4_1_tcp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_1_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_1_tcp_1_checksum {
    input {
        ipv6_1_tcp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}
calculated_field tcp_1.checksum {
    verify rx_ipv4_1_tcp_1_checksum;
    verify rx_ipv6_1_tcp_1_checksum;
    update rx_ipv4_1_tcp_1_checksum;
    update rx_ipv6_1_tcp_1_checksum;
}

@pragma checksum hdr_len_expr ohi.ipv4_2___hdr_len + 0
@pragma checksum verify_len ohi.l4_2_len
@pragma checksum gress ingress
field_list_calculation rx_ipv4_2_checksum {
    input {
        ipv4_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_2.hdrChecksum  {
    verify rx_ipv4_2_checksum;
    update rx_ipv4_2_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len  ohi.l4_2_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_2_udp_2_checksum {
    input {
        ipv4_2_udp_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_2_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_2_udp_2_checksum {
    input {
        ipv6_2_udp_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_2.checksum {
    verify rx_ipv4_2_udp_2_checksum;
    verify rx_ipv6_2_udp_2_checksum;
    update rx_ipv4_2_udp_2_checksum;
    update rx_ipv6_2_udp_2_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_2_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_2_tcp_2_checksum {
    input {
        ipv4_2_tcp_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_2_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_2_tcp_2_checksum {
    input {
        ipv6_2_tcp_2_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}
calculated_field tcp_2.checksum {
    verify rx_ipv4_2_tcp_2_checksum;
    verify rx_ipv6_2_tcp_2_checksum;
    //update rx_ipv4_2_tcp_2_checksum;
    //update rx_ipv6_2_tcp_2_checksum;
}

@pragma checksum hdr_len_expr ohi.ipv4_3___hdr_len + 0
@pragma checksum verify_len ohi.l4_3_len
@pragma checksum gress ingress
field_list_calculation rx_ipv4_3_checksum {
    input {
        ipv4_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_3.hdrChecksum  {
    update rx_ipv4_3_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len  ohi.l4_3_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_3_udp_3_checksum {
    input {
        ipv4_3_udp_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_3_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_3_udp_3_checksum {
    input {
        ipv6_3_udp_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_3.checksum {
    update rx_ipv4_3_udp_3_checksum;
    update rx_ipv6_3_udp_3_checksum;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_3_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv4_3_tcp_3_checksum {
    input {
        ipv4_3_tcp_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.rx_l4_payload_len
@pragma checksum verify_len ohi.l4_3_len
@pragma checksum gress ingress
@pragma checksum update_share udp_1.checksum, udp_2.checksum, udp_3.checksum, tcp_1.checksum, tcp_2.checksum, tcp_3.checksum
field_list_calculation rx_ipv6_3_tcp_3_checksum {
    input {
        ipv6_3_tcp_3_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}
calculated_field tcp_3.checksum {
    update rx_ipv4_3_tcp_3_checksum;
    update rx_ipv6_3_tcp_3_checksum;
}


field_list udp_opt_checksum_list {
    udp_opt_ocs.kind; // First field in UDP options related to csum
    payload; // specify payload keyword as list of options
}

@pragma checksum verify_len ohi.udp_opt_len
@pragma checksum gress ingress
@pragma checksum udp_option
field_list_calculation rx_udp_opt_checksum {
    input {
        udp_opt_checksum_list;
    }
    algorithm : csum8;
    output_width : 8;
}

calculated_field udp_opt_ocs.chksum {
    verify rx_udp_opt_checksum;
}


#if 0
field_list l2_complete_checksum_list {
   ethernet_3.dstAddr;
   payload;
}

@pragma checksum update_len capri_deparser_len.udp_opt_l2_checksum_len
@pragma checksum gress ingress
field_list_calculation l2_complete_checksum {
   input {
       l2_complete_checksum_list;
   }
   algorithm : l2_complete_csum; // Used to indicate L2 Complete Csum
   output_width : 16;
}

calculated_field p4_to_p4plus_classic_nic.csum {
   update l2_complete_checksum if (valid(ethernet_3));
}
#endif


/******************************************************************************
 * icrc in Rx Path
 *****************************************************************************/

field_list rx_ipv4_1_icrc_list {
    ipv4_1.ttl;
    ipv4_1.diffserv;
    ipv4_1.hdrChecksum;
    udp_1.checksum;
    roce_bth_1.reserved1;
}
@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv4_1_roce_1_icrc {
    input {
        rx_ipv4_1_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv6_1_icrc_list {
    ipv6_1.trafficClass;
    ipv6_1.flowLabel;
    ipv6_1.hopLimit;
    udp_1.checksum;
    roce_bth_1.reserved1;
}

@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv6_1_roce_1_icrc {
    input {
        rx_ipv6_1_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv4_2_icrc_list {
    ipv4_2.ttl;
    ipv4_2.diffserv;
    ipv4_2.hdrChecksum;
    udp_2.checksum;
    roce_bth_2.reserved1;
}
@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv4_2_roce_2_icrc {
    input {
        rx_ipv4_2_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv6_2_icrc_list {
    ipv6_2.trafficClass;
    ipv6_2.flowLabel;
    ipv6_2.hopLimit;
    udp_2.checksum;
    roce_bth_2.reserved1;
}

@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv6_2_roce_2_icrc {
    input {
        rx_ipv6_2_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv4_1_roce_icrc_list {
    ipv4_1.ttl;
    ipv4_1.diffserv;
    ipv4_1.hdrChecksum;
    udp_1.checksum;
    roce_bth.reserved1;
}
@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv4_1_roce_icrc {
    input {
        rx_ipv4_1_roce_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv6_1_roce_icrc_list {
    ipv6_1.trafficClass;
    ipv6_1.flowLabel;
    ipv6_1.hopLimit;
    udp_1.checksum;
    roce_bth.reserved1;
}

@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv6_1_roce_icrc {
    input {
        rx_ipv6_1_roce_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv4_2_roce_icrc_list {
    ipv4_2.ttl;
    ipv4_2.diffserv;
    ipv4_2.hdrChecksum;
    udp_2.checksum;
    roce_bth.reserved1;
}
@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv4_2_roce_icrc {
    input {
        rx_ipv4_2_roce_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list rx_ipv6_2_roce_icrc_list {
    ipv6_2.trafficClass;
    ipv6_2.flowLabel;
    ipv6_2.hopLimit;
    udp_2.checksum;
    roce_bth.reserved1;
}

@pragma icrc verify_len ohi.icrc_len
@pragma icrc gress ingress
field_list_calculation rx_ipv6_2_roce_icrc {
    input {
        rx_ipv6_2_roce_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

calculated_field parser_metadata.icrc {

    // icrc for optimized parse path case.
    verify rx_ipv4_1_roce_1_icrc if (valid(roce_bth_1));
    verify rx_ipv6_1_roce_1_icrc if (valid(roce_bth_1));
    verify rx_ipv4_2_roce_2_icrc if (valid(roce_bth_2));
    verify rx_ipv6_2_roce_2_icrc if (valid(roce_bth_2));

    // icrc for un-optimized parse path case.
    verify rx_ipv4_1_roce_icrc if (valid(roce_bth));
    verify rx_ipv6_1_roce_icrc if (valid(roce_bth));
    verify rx_ipv4_2_roce_icrc if (valid(roce_bth));
    verify rx_ipv6_2_roce_icrc if (valid(roce_bth));
}



/******************************************************************************/
/* Egress parser                                                              */
/******************************************************************************/
@pragma xgress egress
parser egress_start {
    extract(capri_intrinsic);
#ifdef TXHACK
    extract(capri_p4_intrinsic);
#endif
    extract(capri_txdma_intrinsic);
    extract(p4plus_to_p4);
    return select(capri_intrinsic.csum_err) {
        default: parse_txdma;
        0x1 mask 0 : egress_deparse_start;
    }
}

@pragma deparse_only
@pragma xgress egress
parser egress_deparse_start {
    // intrinsic headers
    extract(capri_intrinsic);
    extract(capri_p4_intrinsic);

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
@pragma allow_set_meta l4_metadata.l4_sport_1
@pragma allow_set_meta l4_metadata.l4_dport_1
parser parse_tx_tcp_1 {
    extract(tcp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return ingress;
}

@pragma xgress egress
@pragma allow_set_meta l4_metadata.l4_sport_1
@pragma allow_set_meta l4_metadata.l4_dport_1
parser parse_tx_udp_1 {
    extract(udp_1);
    set_metadata(l4_metadata.l4_sport_1, latest.srcPort);
    set_metadata(l4_metadata.l4_dport_1, latest.dstPort);
    return select(latest.dstPort) {
        UDP_PORT_ROCE_V2 : parse_tx_roce_v2;
        default : ingress;
    }
}

@pragma xgress egress
parser parse_tx_roce_v2 {
    extract(roce_bth_1);
    return ingress;
}

/******************************************************************************
 * Checksum  in Tx Path
 *****************************************************************************/

field_list ipv4_00_checksum_list {
    ipv4_00.version;
    ipv4_00.ihl;
    ipv4_00.diffserv;
    ipv4_00.totalLen;
    ipv4_00.identification;
    ipv4_00.flags;
    ipv4_00.fragOffset;
    ipv4_00.ttl;
    ipv4_00.protocol;
    ipv4_00.srcAddr;
    ipv4_00.dstAddr;
}

field_list ipv4_00_udp_00_checksum_list {
    ipv4_00.srcAddr;
    ipv4_00.dstAddr;
    8'0;
    ipv4_00.protocol;
    udp_00.len;
    udp_00.srcPort;
    udp_00.dstPort;
    payload;
}

field_list ipv6_00_udp_00_checksum_list {
    ipv6_00.srcAddr;
    ipv6_00.dstAddr;
    8'0;
    ipv6_00.nextHdr;
    udp_00.srcPort;
    udp_00.dstPort;
    payload;
}

field_list ipv4_01_checksum_list {
    ipv4_01.version;
    ipv4_01.ihl;
    ipv4_01.diffserv;
    ipv4_01.totalLen;
    ipv4_01.identification;
    ipv4_01.flags;
    ipv4_01.fragOffset;
    ipv4_01.ttl;
    ipv4_01.protocol;
    ipv4_01.srcAddr;
    ipv4_01.dstAddr;
}

field_list ipv4_01_udp_01_checksum_list {
    ipv4_01.srcAddr;
    ipv4_01.dstAddr;
    8'0;
    ipv4_01.protocol;
    udp_01.len;
    udp_01.srcPort;
    udp_01.dstPort;
    payload;
}

field_list ipv6_01_udp_01_checksum_list {
    ipv6_01.srcAddr;
    ipv6_01.dstAddr;
    8'0;
    ipv6_01.nextHdr;
    udp_01.srcPort;
    udp_01.dstPort;
    payload;
}

@pragma checksum hdr_len_expr ohi.ipv4_00___hdr_len + 0
@pragma checksum gress egress
field_list_calculation tx_ipv4_00_checksum {
    input {
        ipv4_00_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_00.hdrChecksum  {
    update tx_ipv4_00_checksum;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv4_00_udp_00_checksum {
    input {
        ipv4_00_udp_00_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv6_00_udp_00_checksum {
    input {
        ipv6_00_udp_00_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_00.checksum {
    update tx_ipv4_00_udp_00_checksum;
    update tx_ipv6_00_udp_00_checksum;
}

@pragma checksum hdr_len_expr ohi.ipv4_01___hdr_len + 0
@pragma checksum gress egress
field_list_calculation tx_ipv4_01_checksum {
    input {
        ipv4_01_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_01.hdrChecksum  {
    update tx_ipv4_01_checksum;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv4_01_udp_01_checksum {
    input {
        ipv4_01_udp_01_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv6_01_udp_01_checksum {
    input {
        ipv6_01_udp_01_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_01.checksum {
    update tx_ipv4_01_udp_01_checksum;
    update tx_ipv6_01_udp_01_checksum;
}

@pragma checksum hdr_len_expr ohi.ipv4_1___hdr_len + 0
@pragma checksum gress egress
field_list_calculation tx_ipv4_1_checksum {
    input {
        ipv4_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4_1.hdrChecksum  {
    update tx_ipv4_1_checksum;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv4_1_udp_1_checksum {
    input {
        ipv4_1_udp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv6_1_udp_1_checksum {
    input {
        ipv6_1_udp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp_1.checksum {
    update tx_ipv4_1_udp_1_checksum;
    update tx_ipv6_1_udp_1_checksum;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv4_1_tcp_1_checksum {
    input {
        ipv4_1_tcp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.tx_l4_payload_len
@pragma checksum gress egress
@pragma checksum update_share udp_00.checksum, udp_01.checksum, udp_1.checksum, tcp_1.checksum
field_list_calculation tx_ipv6_1_tcp_1_checksum {
    input {
        ipv6_1_tcp_1_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}
calculated_field tcp_1.checksum {
    update tx_ipv4_1_tcp_1_checksum;
    update tx_ipv6_1_tcp_1_checksum;
}

/******************************************************************************
 * icrc in Tx Path
 *****************************************************************************/

field_list tx_ipv4_1_icrc_list {
    ipv4_1.ttl;
    ipv4_1.diffserv;
    ipv4_1.hdrChecksum;
    udp_1.checksum;
    roce_bth_1.reserved1;
}
@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc gress egress
field_list_calculation tx_ipv4_1_roce_icrc {
    input {
        tx_ipv4_1_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list tx_ipv6_1_icrc_list {
    ipv6_1.trafficClass;
    ipv6_1.flowLabel;
    ipv6_1.hopLimit;
    udp_1.checksum;
    roce_bth_1.reserved1;
}

@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc gress egress
field_list_calculation tx_ipv6_1_roce_icrc {
    input {
        tx_ipv6_1_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

calculated_field parser_metadata.icrc {
    update tx_ipv4_1_roce_icrc if (valid(roce_bth_1));
    update tx_ipv6_1_roce_icrc if (valid(roce_bth_1));
}
