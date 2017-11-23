/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


header_type parser_metadata_t {
    fields {
        parse_ipv4_counter              : 8;
        parse_inner_ipv4_counter        : 8;
        parse_tcp_counter               : 8;
        ipv6_nextHdr                    : 8;
        l4_trailer                      : 16;
        l4_len                          : 16;
        //Parser local field to specify destination field 
        //to store icrc value. No PHV allocated.
        ipv6_options_len                : 16;
        icrc                            : 32; 
    }
}
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

header_type parser_csum_t {
    fields {
        ipv4___hdr_len                     : 16;
        inner_ipv4___hdr_len               : 16;
        inner_ipv6_payloadLen              : 16;
    }
}

header_type parser_ohi_t {
    // These are write-only variables
    // Each header can have upto two variables - <hdrname>___start_off and <hdr>___hdr_len
    // In most cases start_offset and len is computed internally whenever header is extracted 
    // via OHI slots. User should define fields here only if they are written outside of header
    // extraction
    // In addition, some of the checksum  related variables can use this feature to store checksum
    // start/len information to write-only variables
    fields {
        ipv6_options_blob___hdr_len : 16;
    }
}
@pragma pa_parser_local
@pragma parser_write_only
metadata parser_ohi_t ohi;

@pragma pa_parser_local
@pragma parser_no_reg
metadata parser_csum_t parser_csum;

/* tag depths */
#define MPLS_DEPTH 3

#define PARSE_ETHERTYPE                                    \
        ETHERTYPE_VLAN : parse_vlan;                       \
        ETHERTYPE_MPLS : parse_mpls;                       \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        ETHERTYPE_ARP : parse_arp_rarp;                    \
        ETHERTYPE_RARP : parse_arp_rarp;                   \
        default: ingress

#define PARSE_ETHERTYPE_MINUS_VLAN                         \
        ETHERTYPE_MPLS : parse_mpls;                       \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        ETHERTYPE_ARP : parse_arp_rarp;                    \
        ETHERTYPE_RARP : parse_arp_rarp;                   \
        default: ingress

header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t     capri_p4_intrinsic;
header cap_phv_intr_rxdma_t  capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t  capri_txdma_intrinsic;

@pragma pa_header_union egress p4_to_p4plus_roce_eth
header ethernet_t ethernet;
header recirc_header_t recirc_header;
header llc_header_t llc_header;
header snap_header_t snap_header;
header vlan_tag_t vlan_tag;
header mpls_t mpls[MPLS_DEPTH];

@pragma pa_header_union egress ipv6 p4_to_p4plus_roce_ip
@pragma pa_field_union ingress ipv4.ttl ipv6.nextHdr        // keep ordering so parser can combine
@pragma pa_field_union ingress ipv4.protocol ipv6.hopLimit  // keep ordering so parser can combine
@pragma pa_field_union ingress ipv4.identification ipv6.payloadLen
@pragma pa_field_union ingress ipv4.dstAddr ipv6.dstAddr
@pragma pa_field_union ingress ipv4.srcAddr ipv6.srcAddr
header ipv4_t ipv4;
header ipv6_t ipv6;
header icmp_t icmp;
header icmp_t icmpv6;
@pragma pa_header_union ingress v6_ah_esp
header ah_t ah;
@pragma pa_header_union ingress v6_ah_esp
header esp_t esp;

header udp_t udp;
// udp payload is treated as header when parsing udp trailer options
@pragma hdr_len parser_metadata.l4_len
header udp_payload_t udp_payload;
@pragma pa_header_union xgress vxlan_gpe genv nvgre
header vxlan_t vxlan;
header vxlan_gpe_t vxlan_gpe;
header genv_t genv;

header roce_bth_t roce_bth;
header roce_deth_t roce_deth;
header roce_deth_immdt_t roce_deth_immdt;

header gre_t gre;
header nvgre_t nvgre;
header erspan_header_t3_t erspan_t3_header;

@pragma pa_header_union ingress inner_udp icmp icmpv6
header tcp_t tcp;
// TCP options
header tcp_option_unknown_t tcp_option_unknown;
@pragma no_ohi xgress
header tcp_option_eol_t tcp_option_eol;
@pragma no_ohi xgress
header tcp_option_nop_t tcp_option_nop;
@pragma no_ohi xgress
header tcp_option_nop_t tcp_option_nop_1;
@pragma no_ohi xgress
header tcp_option_mss_t tcp_option_mss;
@pragma no_ohi xgress
header tcp_option_ws_t tcp_option_ws;
@pragma no_ohi xgress
header tcp_option_sack_perm_t tcp_option_sack_perm;
@pragma pa_header_union xgress tcp_option_two_sack tcp_option_three_sack tcp_option_four_sack
@pragma no_ohi xgress
header tcp_option_one_sack_t tcp_option_one_sack;
@pragma no_ohi xgress
header tcp_option_two_sack_t tcp_option_two_sack;
@pragma no_ohi xgress
header tcp_option_three_sack_t tcp_option_three_sack;
@pragma no_ohi xgress
header tcp_option_four_sack_t tcp_option_four_sack;
@pragma no_ohi xgress
header tcp_option_timestamp_t tcp_option_timestamp;

// IPv4 Options
header ipv4_option_EOL_t ipv4_option_EOL;
header ipv4_option_NOP_t ipv4_option_NOP;
header ipv4_option_security_t ipv4_option_security;
header ipv4_option_timestamp_t ipv4_option_timestamp;
header ipv4_option_lsr_t ipv4_option_lsr;
header ipv4_option_ssr_t ipv4_option_ssr;
header ipv4_option_rr_t ipv4_option_rr;

// Inner IPv4 Options
header ipv4_option_EOL_t inner_ipv4_option_EOL;
header ipv4_option_NOP_t inner_ipv4_option_NOP;
header ipv4_option_security_t inner_ipv4_option_security;
header ipv4_option_timestamp_t inner_ipv4_option_timestamp;
header ipv4_option_lsr_t inner_ipv4_option_lsr;
header ipv4_option_ssr_t inner_ipv4_option_ssr;
header ipv4_option_rr_t inner_ipv4_option_rr;

header ethernet_t inner_ethernet;
@pragma pa_header_union xgress inner_ipv6
header ipv4_t inner_ipv4;
header ipv6_t inner_ipv6;
header udp_t inner_udp;

// name 'capri_i2e_metadata' has a special meaning
header capri_i2e_metadata_t capri_i2e_metadata;

header p4_to_p4plus_ipsec_header_t p4_to_p4plus_ipsec;
header p4_to_p4plus_roce_header_t p4_to_p4plus_roce;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.srcPort      tcp.srcPort
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.dstPort      tcp.dstPort
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.seqNo        tcp.seqNo
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.ackNo        tcp.ackNo
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.dataOffset   tcp.dataOffset
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.res          tcp.res
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.flags        tcp.flags
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.window       tcp.window
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.urgentPtr    tcp.urgentPtr
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.ts           tcp_option_timestamp.ts
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy.prev_echo_ts tcp_option_timestamp.prev_echo_ts
header p4_to_p4plus_tcp_proxy_base_header_t p4_to_p4plus_tcp_proxy;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.start_seq0 tcp_option_four_sack.first_le
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.end_seq0   tcp_option_four_sack.first_re
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.start_seq1 tcp_option_four_sack.second_le
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.end_seq1   tcp_option_four_sack.second_re
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.start_seq2 tcp_option_four_sack.third_le
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.end_seq2   tcp_option_four_sack.third_re
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.start_seq3 tcp_option_four_sack.fourth_le
@pragma pa_field_union egress p4_to_p4plus_tcp_proxy_sack.end_seq3   tcp_option_four_sack.fourth_re
header p4_to_p4plus_tcp_proxy_sack_header_t p4_to_p4plus_tcp_proxy_sack;

@pragma synthetic_header
header ethernet_t p4_to_p4plus_roce_eth;
@pragma synthetic_header
header ipv6_t p4_to_p4plus_roce_ip;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_classic_nic_ip.ip_sa        ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_classic_nic_ip.ip_da        ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_classic_nic_ip;
@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_classic_nic_inner_ip.ip_sa  inner_ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_classic_nic_inner_ip.ip_da  inner_ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_classic_nic_inner_ip;
header p4_to_p4plus_classic_nic_header_t p4_to_p4plus_classic_nic;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_cpu_ip.ip_sa             ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_cpu_ip.ip_da             ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_cpu_ip;
@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_flags        tcp.flags
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_seqNo        tcp.seqNo
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_ackNo        tcp.ackNo
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_window       tcp.window
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_mss          tcp_option_mss.value
@pragma pa_field_union egress p4_to_p4plus_cpu_pkt.tcp_ws           tcp_option_ws.value
header p4_to_p4plus_cpu_pkt_t p4_to_p4plus_cpu_pkt;
header p4_to_p4plus_cpu_header_t p4_to_p4plus_cpu;

header p4_to_p4plus_p4pt_header_t p4_to_p4plus_p4pt;
header p4plus_to_p4_header_t p4plus_to_p4;

parser start {
    extract(capri_intrinsic);
    set_metadata(control_metadata.tm_iport, capri_intrinsic.tm_iport + 0);
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_INGRESS : parse_recirc;
        TM_PORT_DMA : parse_txdma;
        default : parse_nic;
        0x1 mask 0 : egress_start;
    }
}

@pragma xgress egress
parser egress_start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.tm_iport) {
        TM_PORT_INGRESS : parse_ingress_to_egress;
        TM_PORT_EGRESS : parse_egress_to_egress;
        default : ingress;
        0x1 mask 0x0 : deparse_rxdma;
    }
}

@pragma xgress egress
parser parse_ingress_to_egress {
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_MULTICAST : parse_tm_replication_data1;
        default : parse_i2e_metadata1;
    }
}

@pragma xgress egress
parser parse_egress_to_egress {
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_span_copy;
        TM_INSTANCE_TYPE_SPAN_AND_DROP : parse_span_copy;
        default : parse_ethernet;
    }
}

@pragma xgress egress
// NCC work-around.. intrinsic meta and i2e meta are in different flits
// which is not allowed by hardware. Eventually NCC will implement logic
// to split these states internally, for now do it manually
parser parse_i2e_metadata1 {
    return parse_i2e_metadata;
}
@pragma xgress egress
parser parse_i2e_metadata {
    extract(capri_p4_intrinsic);
    extract(capri_i2e_metadata);
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_span_copy;
        TM_INSTANCE_TYPE_SPAN_AND_DROP : parse_span_copy;
        default : parse_ethernet;
    }
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    return parse_ethernet_span_copy;
}

@pragma xgress egress
parser parse_ethernet_span_copy {
    extract(ethernet);
    return ingress;
}

@pragma xgress egress
@pragma deparse_only
parser deparse_rxdma {
    extract(capri_p4_intrinsic);
    extract(capri_rxdma_intrinsic);
    extract(p4_to_p4plus_cpu);
    extract(p4_to_p4plus_cpu_ip);
    extract(p4_to_p4plus_classic_nic);
    extract(p4_to_p4plus_classic_nic_ip);
    extract(p4_to_p4plus_classic_nic_inner_ip);
    extract(p4_to_p4plus_roce);
    extract(p4_to_p4plus_ipsec);
    extract(p4_to_p4plus_tcp_proxy);
    extract(p4_to_p4plus_tcp_proxy_sack);
    extract(p4_to_p4plus_cpu_pkt);
    extract(p4_to_p4plus_p4pt);
    return parse_ethernet;
}

parser parse_recirc {
    extract(capri_p4_intrinsic);
    extract(recirc_header);
    set_metadata(control_metadata.tm_iport, recirc_header.src_tm_iport + 0);
    return select(recirc_header.src_tm_iport) {
        TM_PORT_DMA: parse_txdma;
        default: parse_nic;
    }
}

parser parse_vm_bounce {
    return parse_ethernet;
}

@pragma xgress ingress
parser parse_txdma {
    extract(capri_txdma_intrinsic);
    extract(p4plus_to_p4);
    return parse_ethernet;
}

parser parse_nic {
    set_metadata(control_metadata.uplink, 1);
    return parse_ethernet;
}

header tm_replication_data_t tm_replication_data;

// NCC work-around.. intrinsic meta and tm_replication_data are in different flits
// which is not allowed by hardware. Eventually NCC will implement logic
// to split these states internally, for now do it manually
@pragma xgress egress
parser parse_tm_replication_data1 {
    return parse_tm_replication_data;
}

@pragma xgress egress
parser parse_tm_replication_data {
    extract(tm_replication_data);
    return parse_i2e_metadata;
}

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        0 mask 0xfe00 : parse_llc_header;
        0 mask 0xfa00 : parse_llc_header;
        PARSE_ETHERTYPE;
    }
}

parser parse_llc_header {
    extract(llc_header);
    return select(llc_header.dsap, llc_header.ssap) {
        0xAAAA : parse_snap_header;
        default: ingress;
    }
}

parser parse_snap_header {
    extract(snap_header);
    return select(latest.type_) {
        PARSE_ETHERTYPE;
    }
}

parser parse_vlan {
    extract(vlan_tag);
    return select(latest.etherType) {
        PARSE_ETHERTYPE_MINUS_VLAN;
    }
}

parser parse_mpls {
    extract(mpls[next]);
    //set_metadata(tunnel_metadata.tunnel_vni, latest.label);
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
    return parse_inner_ethernet;
}

parser parse_mpls_inner_ipv4 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_MPLS_L3VPN);
    return parse_inner_ipv4;
}

parser parse_mpls_inner_ipv6 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_MPLS_L3VPN);
    return parse_inner_ipv6;
}

field_list ipv4_checksum_list {
    ipv4.version;
    ipv4.ihl;
    ipv4.diffserv;
    ipv4.totalLen;
    ipv4.identification;
    ipv4.flags;
    ipv4.fragOffset;
    ipv4.ttl;
    ipv4.protocol;
    ipv4.srcAddr;
    ipv4.dstAddr;
}

@pragma checksum hdr_len_expr parser_csum.ipv4___hdr_len + 20
field_list_calculation ipv4_checksum {
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.hdrChecksum  {
    verify ipv4_checksum;
    update ipv4_checksum;
}


//For icrc, specify invariant crc fields.
field_list ipv4_icrc_list {
    ipv4.ttl;
    ipv4.diffserv;
    ipv4.hdrChecksum;
    udp.checksum;
}
@pragma icrc update_len capri_deparser_len.icrc_payload_len
field_list_calculation ipv4_roce_icrc {
    input {
        ipv4_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

//For icrc, specify invariant crc fields.
field_list ipv6_icrc_list {
    ipv6.trafficClass;
    ipv6.flowLabel;
    ipv6.hopLimit;
    udp.checksum;
}

@pragma icrc update_len capri_deparser_len.icrc_payload_len
field_list_calculation ipv6_roce_icrc {
    input {
        ipv6_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

field_list inner_ipv4_icrc_list {
    inner_ipv4.ttl;
    inner_ipv4.diffserv;
    inner_ipv4.hdrChecksum;
    inner_udp.checksum;
}
@pragma icrc update_len capri_deparser_len.icrc_payload_len
field_list_calculation inner_ipv4_roce_icrc {
    input {
        inner_ipv4_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

//For icrc, specify invariant crc fields in the same order as they appear in packet
field_list inner_ipv6_icrc_list {
    inner_ipv6.trafficClass;
    inner_ipv6.flowLabel;
    inner_ipv6.hopLimit;
    inner_udp.checksum;
}

@pragma icrc update_len capri_deparser_len.icrc_payload_len
field_list_calculation inner_ipv6_roce_icrc {
    input {
        inner_ipv6_icrc_list;
    }
    algorithm : icrc;
    output_width : 32;
}

calculated_field parser_metadata.icrc {
    // Header instance specified within if condition that checks validity
    // of the header, will be used to trigger icrc calculation. In below 
    // case "roce_bth" header instance will be used to enable icrc.
    // In the parse state where roce_bth becomes valid, icrc computation
    // is enabled.
    verify ipv4_roce_icrc if (valid(roce_bth));
    update ipv4_roce_icrc if (valid(roce_bth));
    verify ipv6_roce_icrc if (valid(roce_bth));
    update ipv6_roce_icrc if (valid(roce_bth));

    // to n/w RDMA pkt can be encapped. Support icrc computation.
    update inner_ipv4_roce_icrc if (valid(roce_bth));
    update inner_ipv6_roce_icrc if (valid(roce_bth));

    // currently RDMA pkt from host is not encaped . Hence its L3 hdr is only v4/v6
    // not inner v4/v6. Adding code below in comment so that it can
    // be enabled once p4 support for RDMA encap is supported.
#if 0
    verify inner_ipv4_roce_icrc if (valid(roce_bth));
    verify inner_ipv6_roce_icrc if (valid(roce_bth));
#endif
}

parser parse_ipv4_frag {
    set_metadata(l3_metadata.ip_frag, 1);
    return ingress;
}
parser parse_inner_ipv4_frag {
    set_metadata(l3_metadata.inner_ip_frag, 1);
    return ingress;
}

parser parse_base_ipv4 {
    // option-blob parsing - parse_ipv4 does not extract ipv4 header
    extract(ipv4);
    set_metadata(parser_csum.ipv4___hdr_len, (ipv4.ihl << 2) - 20);
    return select(ipv4.flags, ipv4.fragOffset, ipv4.protocol) {
        IP_PROTO_ICMP mask 0x3fffff : parse_icmp;
        IP_PROTO_TCP mask 0x3fffff : parse_tcp;
        IP_PROTO_UDP mask 0x3fffff : parse_udp;
        IP_PROTO_GRE mask 0x3fffff : parse_gre;
        IP_PROTO_IPV4 mask 0x3fffff : parse_ipv4_in_ip;
        IP_PROTO_IPV6 mask 0x3fffff : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH mask 0x3fffff : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP mask 0x3fffff : parse_ipsec_esp;
        0x000000 mask 0x3fff00 : ingress;
        default: parse_ipv4_frag;
    }
}

#if 0
parser parse_ipv4_option_EOL {
     extract(ipv4_option_EOL);
     set_metadata(parser_metadata.parse_ipv4_counter, parser_metadata.parse_ipv4_counter -1);
     return parse_ipv4_options;
}

parser parse_ipv4_option_NOP {
     extract(ipv4_option_NOP);
     set_metadata(parser_metadata.parse_ipv4_counter, parser_metadata.parse_ipv4_counter -1);
     return parse_ipv4_options;
}

parser parse_ipv4_option_security {
    extract(ipv4_option_security);
    set_metadata(parser_metadata.parse_ipv4_counter, parser_metadata.parse_ipv4_counter - 11);
    return parse_ipv4_options;
}

parser parse_ipv4_option_timestamp {
    extract(ipv4_option_timestamp);
    set_metadata(parser_metadata.parse_ipv4_counter,
                 parser_metadata.parse_ipv4_counter - ipv4_option_timestamp.len);
    return parse_ipv4_options;
}

parser parse_ipv4_option_lsr{
    extract(ipv4_option_lsr);
    set_metadata(parser_metadata.parse_ipv4_counter,
                 parser_metadata.parse_ipv4_counter - ipv4_option_lsr.len);
    return parse_ipv4_options;
}

parser parse_ipv4_option_ssr{
    extract(ipv4_option_ssr);
    set_metadata(parser_metadata.parse_ipv4_counter,
                 parser_metadata.parse_ipv4_counter - ipv4_option_ssr.len);
    return parse_ipv4_options;
}

parser parse_ipv4_option_rr{
    extract(ipv4_option_rr);
    set_metadata(parser_metadata.parse_ipv4_counter,
                 parser_metadata.parse_ipv4_counter - ipv4_option_rr.len);
    return parse_ipv4_options;
}

@pragma header_ordering ipv4_option_security ipv4_option_timestamp ipv4_option_lsr ipv4_option_ssr ipv4_option_rr ipv4_option_NOP ipv4_option_EOL
parser parse_ipv4_options {
    set_metadata(l3_metadata.ip_option_seen, 1);
    return select(parser_metadata.parse_ipv4_counter, current(0, 8)) {
        0x0000 mask 0xff00 : parse_base_ipv4;
        0x0000 mask 0x00ff : parse_ipv4_option_EOL;
        0x0001 mask 0x00ff : parse_ipv4_option_NOP;
        0x0082 mask 0x00ff : parse_ipv4_option_security;
        0x0083 mask 0x00ff : parse_ipv4_option_lsr;
        0x0087 mask 0x00ff : parse_ipv4_option_rr;
        0x0089 mask 0x00ff : parse_ipv4_option_ssr;
        0x0044 mask 0x00ff : parse_ipv4_option_timestamp;
    }
}
#endif


@pragma hdr_len ipv4.ihl
header ipv4_options_blob_t ipv4_options_blob;
parser parse_ipv4_options_blob {
    // Separate state is created to set options_seen flag 
    // Otherwise options can be extracted blindly.. if they happen to be 0 len
    // hw (deparser) can handle it

    // Must extract ipv4 header and ipv4_options_blob in the same state
    extract(ipv4);

    // set hdr len same as option header len. In csum profile
    // standard IP hdr len of 20 bytes is adjusted.
    set_metadata(parser_csum.ipv4___hdr_len, (ipv4.ihl << 2) - 20);

    // All options are extracted as a single header
    extract(ipv4_options_blob);
    set_metadata(l3_metadata.ip_option_seen, 1);
    return select(ipv4.flags, ipv4.fragOffset, ipv4.protocol) {
        IP_PROTO_ICMP mask 0x3fffff : parse_icmp;
        IP_PROTO_TCP mask 0x3fffff : parse_tcp;
        IP_PROTO_UDP mask 0x3fffff : parse_udp;
        IP_PROTO_GRE mask 0x3fffff : parse_gre;
        IP_PROTO_IPV4 mask 0x3fffff : parse_ipv4_in_ip;
        IP_PROTO_IPV6 mask 0x3fffff : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH mask 0x3fffff : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP mask 0x3fffff : parse_ipsec_esp;
        0x000000 mask 0x3fff00 : ingress;
        default: parse_ipv4_frag;
    }

}

parser parse_ipv4 {
    // initialize l4_trailer to ip payload len. It is adjusted later in parse_udp state
    // since ipv4 header is not extracted here, use current()
    // set_metadata(parser_metadata.l4_trailer, ipv4.totalLen - (ipv4.ihl << 2));
    set_metadata(parser_metadata.l4_trailer, current(16,16) - (current(4,4) << 2));
    return select(current(0,8)) {
        0x45    : parse_base_ipv4;
        0x44    : ingress;
        0x40 mask 0x4C    : ingress;
        default : parse_ipv4_options_blob;
    }
}

parser parse_ipv4_in_ip {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_inner_ipv4;
}

parser parse_ipv6_in_ip {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_IP_IN_IP);
    return parse_inner_ipv6;
}
header ipv6_extn_generic_t v6_generic;
header ipv6_extn_frag_t v6_frag;
header ipv6_extn_ah_esp_t v6_ah_esp;

// use no_extract pragma to simply move the packet offset forward w/o extraction
// similar to advance() feature in P4-16
@pragma no_extract
parser parse_v6_generic_ext_hdr {
    extract(v6_generic);
    set_metadata(parser_metadata.ipv6_nextHdr, latest.nextHdr);
    set_metadata(parser_metadata.ipv6_options_len, \
            parser_metadata.ipv6_options_len + (v6_generic.len << 3) + 8);
    return parse_ipv6_extn_hdrs;
}


// if AH or ESP are hit - done - no more parsing as these are supposed to
// be last in v6 ext-header sequence.
@pragma allow_set_meta ipsec_metadata.seq_no
@pragma allow_set_meta ipsec_metadata.ipsec_type
parser parse_v6_ipsec_esp_hdr {
    extract(esp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 8);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.spi_hi);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.spi_lo);
    set_metadata(ipsec_metadata.seq_no, latest.seqNo);
    set_metadata(ipsec_metadata.ipsec_type, IPSEC_HEADER_ESP);
    return ingress;
}

@pragma allow_set_meta ipsec_metadata.seq_no
@pragma allow_set_meta ipsec_metadata.ipsec_type
parser parse_v6_ipsec_ah_hdr {
    extract(v6_ah_esp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 12);
    set_metadata(parser_metadata.ipv6_nextHdr, latest.nextHdr);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.spi_hi);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.spi_lo);
    set_metadata(ipsec_metadata.seq_no, latest.seqNo);
    set_metadata(ipsec_metadata.ipsec_type, IPSEC_HEADER_AH);
    return ingress;
}

parser parse_fragment_hdr {
    extract(v6_frag);
    set_metadata(parser_metadata.ipv6_nextHdr, latest.nextHdr);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 8);
    return parse_ipv6_extn_hdrs;
}

@pragma hdr_len parser_metadata.ipv6_options_len
header ipv6_options_blob_t ipv6_options_blob;

@pragma dont_advance_packet
parser parse_ipv6_option_blob {
    // ipv6_option_blob header uses ipv6_option_len as variable header len
    // setup the ipv6_option_len = 0 and keep updating the len as options are
    // parsed.
    // Note1: that options are parsed using 'no_extract' state i.e. they are not
    // individually extracted to phv/ohi (but hv bits will be set)
    // Note2: ipv6_options_len value used to setup ohi slot is previous value (before init)
    set_metadata(parser_metadata.ipv6_options_len, 0);
    extract(ipv6_options_blob);
    return parse_ipv6_extn_hdrs;
}

parser parse_ipv6_ulp {
    // update the header len of the ipv6_options_blob header
    // must use expression to update ohi variable.
    set_metadata(ohi.ipv6_options_blob___hdr_len, parser_metadata.ipv6_options_len+0);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - parser_metadata.ipv6_options_len);
    return select(parser_metadata.ipv6_nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp;
        IP_PROTO_GRE : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        0x33 : parse_v6_ipsec_ah_hdr;
        0x32 : parse_v6_ipsec_esp_hdr;
        default: ingress;
    }
}

@pragma header_ordering v6_generic 
parser parse_ipv6_extn_hdrs {
    set_metadata(l3_metadata.ip_option_seen, 1);
    // To store back into OHI payloadLen - Sum of option hdr len,
    // use set_metadata with zero len
    // set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer + 0);
    return select(parser_metadata.ipv6_nextHdr) {
        0x00: ingress; // hop-by-hop option, not supported
        0x2b: parse_v6_generic_ext_hdr;
        // Note2: ipv6_options_len value used to setup ohi slot is previous value (before init)
        0x2c: parse_v6_generic_ext_hdr;
        0x3c: parse_v6_generic_ext_hdr;
        0x87: parse_v6_generic_ext_hdr;
        default: parse_ipv6_ulp;
    }
}

parser parse_ipv6 {
    extract(ipv6);
    set_metadata(parser_metadata.ipv6_nextHdr, latest.nextHdr);
    set_metadata(parser_metadata.l4_trailer, ipv6.payloadLen);
    return select(parser_metadata.ipv6_nextHdr) {
        // go to ulp if no extention headers to avoid a state transition
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_udp;
        IP_PROTO_GRE : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        0x33 : parse_v6_ipsec_ah_hdr;
        0x32 : parse_v6_ipsec_esp_hdr;
        // add other known options ipsec etc here
        0x2b: parse_ipv6_option_blob;
        0x2c: parse_ipv6_option_blob;
        0x3c: parse_ipv6_option_blob;   // dest_option
        0x87: parse_ipv6_option_blob;
        default: ingress;
    }
}

@pragma allow_set_meta ipsec_metadata.seq_no
@pragma allow_set_meta ipsec_metadata.ipsec_type
parser parse_ipsec_ah {
    extract(ah);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.spi_hi);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.spi_lo);
    set_metadata(ipsec_metadata.seq_no, latest.seqNo);
    set_metadata(ipsec_metadata.ipsec_type, IPSEC_HEADER_AH);
    return ingress;
}

@pragma allow_set_meta ipsec_metadata.seq_no
@pragma allow_set_meta ipsec_metadata.ipsec_type
parser parse_ipsec_esp {
    extract(esp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.spi_hi);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.spi_lo);
    set_metadata(ipsec_metadata.seq_no, latest.seqNo);
    set_metadata(ipsec_metadata.ipsec_type, IPSEC_HEADER_ESP);
    return ingress;
}

parser parse_icmp {
    extract(icmp);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.typeCode);
    return select(latest.typeCode) {
        ICMP_ECHO_REQ_TYPE_CODE : parse_icmp_echo_req_reply;
        ICMP_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_req_reply;
        default: ingress;
    }
}

parser parse_icmpv6 {
    extract(icmpv6);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.typeCode);
    return select(latest.typeCode) {
        ICMPV6_ECHO_REQ_TYPE_CODE : parse_icmp_echo_req_reply;
        ICMPV6_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_req_reply;
        default: ingress;
    }
}

header icmp_echo_req_reply_t icmp_echo;

parser parse_icmp_echo_req_reply {
    extract(icmp_echo);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.identifier);
    return ingress;
}

field_list ipv4_tcp_checksum_list {
    ipv4.srcAddr;
    ipv4.dstAddr;
    8'0;
    ipv4.protocol;
    nat_metadata.tcp_checksum_len;
    tcp.srcPort;
    tcp.dstPort;
    tcp.seqNo;
    tcp.ackNo;
    tcp.dataOffset;
    tcp.res;
    tcp.flags;
    tcp.window;
    tcp.urgentPtr;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
field_list_calculation ipv4_tcp_checksum {
    input {
        ipv4_tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list ipv6_tcp_checksum_list {
    ipv6.srcAddr;
    ipv6.dstAddr;
    8'0;
    ipv6.nextHdr;
    nat_metadata.tcp_checksum_len;
    tcp.srcPort;
    tcp.dstPort;
    tcp.seqNo;
    tcp.ackNo;
    tcp.dataOffset;
    tcp.res;
    tcp.flags;
    tcp.window;
    tcp.urgentPtr;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum verify_len parser_metadata.l4_trailer
field_list_calculation ipv6_tcp_checksum {
    input {
        ipv6_tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field tcp.checksum {
    verify ipv4_tcp_checksum;
    verify ipv6_tcp_checksum;
    update ipv4_tcp_checksum if (nat_metadata.update_checksum == TRUE);
    update ipv6_tcp_checksum if (valid(ipv6));
}

@pragma hdr_len parser_metadata.parse_tcp_counter
header tcp_options_blob_t tcp_options_blob;
parser parse_tcp {
    extract(tcp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.srcPort);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.dstPort);
    set_metadata(parser_metadata.parse_tcp_counter, (tcp.dataOffset << 2) - 20);
    return select(parser_metadata.parse_tcp_counter) {
        0 : ingress;
        default : parse_tcp_options_blob;
    }
}

@pragma dont_advance_packet
@pragma capture_payload_offset 
parser parse_tcp_options_blob {
    set_metadata(parser_metadata.parse_tcp_counter, parser_metadata.parse_tcp_counter + 0);
    extract(tcp_options_blob);
    return parse_tcp_options;
}

parser parse_tcp_option_one_sack {
    extract(tcp_option_one_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 10);
    return parse_tcp_options;
}

parser parse_tcp_option_two_sack {
    extract(tcp_option_two_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 18);
    return parse_tcp_options;
}

parser parse_tcp_option_three_sack {
    extract(tcp_option_three_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 26);
    return parse_tcp_options;
}

parser parse_tcp_option_four_sack {
    extract(tcp_option_four_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 34);
    return parse_tcp_options;
}

parser parse_tcp_option_sack {
    return select(current(8,8)) {
        0xa : parse_tcp_option_one_sack;
        0x12 : parse_tcp_option_two_sack;
        0x1a : parse_tcp_option_three_sack;
        0x22 : parse_tcp_option_four_sack;
        default : parse_tcp_unknown_option;
    }
}

parser parse_tcp_option_EOL {
    extract(tcp_option_eol);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 1);
    return parse_tcp_options;
}

parser parse_tcp_option_NOP {
    extract(tcp_option_nop);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 1);
    return parse_tcp_options;
}

@pragma deparse_only
parser parse_tcp_deparse_options {
    extract(tcp_option_nop_1);
    return parse_tcp_options;
}

parser parse_tcp_option_mss {
    extract(tcp_option_mss);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 4);
    return parse_tcp_options;
}

parser parse_tcp_option_ws {
    extract(tcp_option_ws);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 3);
    return parse_tcp_options;
}

parser parse_tcp_option_sack_perm {
    extract(tcp_option_sack_perm);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 2);
    return parse_tcp_options;
}

parser parse_tcp_timestamp {
    extract(tcp_option_timestamp);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 10);
    return parse_tcp_options;
}

parser parse_tcp_unknown_option {
    extract(tcp_option_unknown);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - tcp_option_unknown.optLength);
    return parse_tcp_options;
}

@pragma dont_capture_payload_offset 
parser parse_tcp_option_error {
    set_metadata(control_metadata.parse_tcp_option_error, 1);
    return ingress;
}

@pragma header_ordering tcp_option_mss tcp_option_ws tcp_option_sack_perm tcp_option_one_sack tcp_option_two_sack tcp_option_three_sack tcp_option_four_sack tcp_option_timestamp tcp_option_unknown tcp_option_nop tcp_option_nop_1 tcp_option_eol
parser parse_tcp_options {
    return select(parser_metadata.parse_tcp_counter, current(0, 8)) {
        0x0000 mask 0xff00 : ingress;
        0x8000 mask 0x8000 : parse_tcp_option_error;
        0x0000 mask 0x00ff: parse_tcp_option_EOL;
        0x0001 mask 0x00ff: parse_tcp_option_NOP;
        0x0002 mask 0x00ff: parse_tcp_option_mss;
        0x0003 mask 0x00ff: parse_tcp_option_ws;
        0x0004 mask 0x00ff: parse_tcp_option_sack_perm;
        0x0005 mask 0x00ff: parse_tcp_option_sack;
        0x0008 mask 0x00ff: parse_tcp_timestamp;
        default: parse_tcp_unknown_option;
        0x0001 mask 0x0000: parse_tcp_deparse_options;
    }
}

parser parse_roce_v2 {
    extract(roce_bth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-12);
    return select(latest.opCode) {
        0x64 : parse_roce_deth;
        0x65 : parse_roce_deth_immdt;
        default : parse_udp_trailer;
        //default : ingress;
    }
}

parser parse_roce_deth {
    extract(roce_deth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-8);
    return select(latest.reserved) {
        //default : ingress;
        default : parse_udp_trailer;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

parser parse_roce_deth_immdt {
    extract(roce_deth_immdt);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-12);
    return select(latest.reserved) {
        default : parse_udp_trailer;
        //default : ingress;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

@pragma xgress egress
parser parse_udp_trailer {
    return select(parser_metadata.l4_trailer) {
        0x0000 mask 0xffff: ingress;
        //0x8000 mask 0x8000: ingress_error;
        default : parse_udp_payload;
    }
}

@pragma xgress egress
parser parse_udp_payload {
    // dummy set_metadata to make ncc compiler reserve a register (wka)
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len + 0);
    extract(udp_payload);
    return parse_udp_options;
}

@pragma xgress egress
parser parse_udp_options {
    return select (current(0,8)) {
       0x02 : parse_udp_option_ocs;
       // Add others here
       default : ingress;
    }
}

@pragma xgress egress
header udp_opt_ocs_t udp_opt_ocs;
parser parse_udp_option_ocs {
    extract(udp_opt_ocs);
    return ingress;
}

@pragma xgress egress
@pragma deparse_only
parser parse_roce_eth {
    extract(p4_to_p4plus_roce_eth);
    extract(p4_to_p4plus_roce_ip);
    return ingress;
}

field_list ipv4_udp_checksum_list {
    ipv4.srcAddr;
    ipv4.dstAddr;
    8'0;
    ipv4.protocol;
    udp.len;
    udp.srcPort;
    udp.dstPort;
    payload;
}

@pragma checksum update_len capri_deparser_len.l4_payload_len
field_list_calculation ipv4_udp_checksum {
    input {
        ipv4_udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list ipv6_udp_checksum_list {
    ipv6.srcAddr;
    ipv6.dstAddr;
    8'0;
    ipv6.nextHdr;
    udp.srcPort;
    udp.dstPort;
    payload;
}

@pragma checksum update_len capri_deparser_len.l4_payload_len
@pragma checksum verify_len parser_metadata.l4_trailer
field_list_calculation ipv6_udp_checksum {
    input {
        ipv6_udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp.checksum {
    verify ipv4_udp_checksum;
    verify ipv6_udp_checksum;
    update ipv4_udp_checksum if (nat_metadata.update_checksum == TRUE);
    update ipv6_udp_checksum if (valid(ipv6));
}

parser parse_udp {
    extract(udp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - udp.len);
    set_metadata(parser_metadata.l4_len, udp.len-8);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan;
        UDP_PORT_GENV: parse_geneve;
        UDP_PORT_VXLAN_GPE : parse_vxlan_gpe;
        UDP_PORT_ROCE_V2: parse_roce_v2;
        UDP_PORT_NATT : parse_ipsec_esp;
        default: ingress;
    }
}

parser parse_gre {
    extract(gre);
    return select(latest.C, latest.R, latest.K, latest.S, latest.s,
                  latest.recurse, latest.flags, latest.ver, latest.proto) {
        GRE_PROTO_NVGRE : parse_nvgre;
        ETHERTYPE_IPV4 : parse_gre_ipv4;
        ETHERTYPE_IPV6 : parse_gre_ipv6;
        GRE_PROTO_ERSPAN_T3 : parse_erspan_t3;
        default: ingress;
    }
}

parser parse_gre_ipv4 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_GRE);
    return parse_inner_ipv4;
}

parser parse_gre_ipv6 {
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_GRE);
    return parse_inner_ipv6;
}

parser parse_nvgre {
    extract(nvgre);
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_NVGRE);
    set_metadata(tunnel_metadata.tunnel_vni, latest.tni);
    return parse_inner_ethernet;
}

parser parse_erspan_t3 {
    extract(erspan_t3_header);
    return parse_inner_ethernet;
}

parser parse_arp_rarp {
    return ingress;
}

parser parse_vxlan {
    extract(vxlan);
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_VXLAN);
    set_metadata(tunnel_metadata.tunnel_vni, latest.vni);
    return parse_inner_ethernet;
}

parser parse_vxlan_gpe {
    extract(vxlan_gpe);
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_VXLAN_GPE);
    set_metadata(tunnel_metadata.tunnel_vni, latest.vni);
    return parse_inner_ethernet;
}

parser parse_geneve {
    extract(genv);
    set_metadata(tunnel_metadata.tunnel_vni, latest.vni);
    set_metadata(tunnel_metadata.tunnel_type, INGRESS_TUNNEL_TYPE_GENEVE);
    return select(genv.ver, genv.optLen, genv.protoType) {
        ETHERTYPE_ETHERNET : parse_inner_ethernet;
        ETHERTYPE_IPV4 : parse_inner_ipv4;
        ETHERTYPE_IPV6 : parse_inner_ipv6;
        default : ingress;
    }
}

field_list inner_ipv4_checksum_list {
    inner_ipv4.version;
    inner_ipv4.ihl;
    inner_ipv4.diffserv;
    inner_ipv4.totalLen;
    inner_ipv4.identification;
    inner_ipv4.flags;
    inner_ipv4.fragOffset;
    inner_ipv4.ttl;
    inner_ipv4.protocol;
    inner_ipv4.srcAddr;
    inner_ipv4.dstAddr;
}

@pragma checksum hdr_len_expr parser_csum.inner_ipv4___hdr_len + 20
field_list_calculation inner_ipv4_checksum {
    input {
        inner_ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field inner_ipv4.hdrChecksum {
    verify inner_ipv4_checksum if (inner_ipv4.ihl == 5);
    update inner_ipv4_checksum if (inner_ipv4.ihl == 5);
}

parser parse_base_inner_ipv4 {
    // option-blob parsing - extract inner_ipv4 here
    extract(inner_ipv4);
    set_metadata(parser_csum.inner_ipv4___hdr_len, (inner_ipv4.ihl << 2) - 20);
    return select(inner_ipv4.flags, inner_ipv4.fragOffset, inner_ipv4.protocol) {
        IP_PROTO_ICMP mask 0x3fffff : parse_icmp;
        IP_PROTO_TCP mask 0x3fffff : parse_tcp;
        IP_PROTO_UDP mask 0x3fffff : parse_inner_udp;
        0x000000 mask 0x3fff00 : ingress;
        default: parse_inner_ipv4_frag;
    }
}

#if 0
parser parse_inner_ipv4_option_EOL {
     extract(inner_ipv4_option_EOL);
     set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter -1);
     return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_NOP {
     extract(inner_ipv4_option_NOP);
     set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter -1);
     return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_security {
    extract(inner_ipv4_option_security);
    set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter - 11);
    return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_timestamp {
    extract(inner_ipv4_option_timestamp);
    set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter - inner_ipv4_option_timestamp.len);
    return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_lsr {
    extract(inner_ipv4_option_lsr);
    set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter - inner_ipv4_option_lsr.len);
    return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_ssr {
    extract(inner_ipv4_option_ssr);
    set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter - inner_ipv4_option_ssr.len);
    return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_option_rr {
    extract(inner_ipv4_option_rr);
    set_metadata(parser_metadata.parse_inner_ipv4_counter, parser_metadata.parse_inner_ipv4_counter - inner_ipv4_option_rr.len);
    return parse_inner_ipv4_options;
}

@pragma header_ordering inner_ipv4_option_security inner_ipv4_option_timestamp inner_ipv4_option_lsr inner_ipv4_option_ssr inner_ipv4_option_rr inner_ipv4_option_NOP inner_ipv4_option_EOL
parser parse_inner_ipv4_options {
    set_metadata(l3_metadata.inner_ip_option_seen, 1);
    return select(parser_metadata.parse_inner_ipv4_counter, current(0, 8)) {
        0x0000 mask 0xff00 : parse_base_inner_ipv4;
        0x0000 mask 0x00ff : parse_inner_ipv4_option_EOL;
        0x0001 mask 0x00ff : parse_inner_ipv4_option_NOP;
        0x0082 mask 0x00ff : parse_inner_ipv4_option_security;
        0x0083 mask 0x00ff : parse_inner_ipv4_option_lsr;
        0x0087 mask 0x00ff : parse_inner_ipv4_option_rr;
        0x0089 mask 0x00ff : parse_inner_ipv4_option_ssr;
        0x0044 mask 0x00ff : parse_inner_ipv4_option_timestamp;
    }
}
#endif


@pragma hdr_len inner_ipv4.ihl
header ipv4_options_blob_t inner_ipv4_options_blob;

parser parse_inner_ipv4_options_blob {
    // Separate state is created to set options_seen flag 
    // Otherwise options can be extracted blindly.. if they happen to be 0 len
    // hw (deparser) can handle it

    // Must extract inner_ipv4 header and inner_ipv4_options_blob in the same state
    extract(inner_ipv4);
    // set hdr len same as option header len. In csum profile
    // standard IP hdr len of 20 bytes is adjusted.
    set_metadata(parser_csum.inner_ipv4___hdr_len, (inner_ipv4.ihl << 2) - 20);
    // All options are extracted as a single header
    extract(inner_ipv4_options_blob);
    set_metadata(l3_metadata.inner_ip_option_seen, 1);

    return select(inner_ipv4.fragOffset, inner_ipv4.protocol) {
        IP_PROTO_ICMP mask 0x3fffff : parse_icmp;
        IP_PROTO_TCP mask 0x3fffff : parse_tcp;
        IP_PROTO_UDP mask 0x3fffff : parse_inner_udp;
        0x000000 mask 0x3fff00 : ingress;
        default: parse_inner_ipv4_frag;
    }
}

parser parse_inner_ipv4 {
    return select(current(0,8)) {
        0x45    : parse_base_inner_ipv4;
        0x44    : ingress;
        0x40 mask 0x4C    : ingress;
        default : parse_inner_ipv4_options_blob;
    }
}

field_list inner_ipv4_tcp_checksum_list {
    inner_ipv4.srcAddr;
    inner_ipv4.dstAddr;
    8'0;
    inner_ipv4.protocol;
    nat_metadata.tcp_checksum_len;
    tcp.srcPort;
    tcp.dstPort;
    tcp.seqNo;
    tcp.ackNo;
    tcp.dataOffset;
    tcp.res;
    tcp.flags;
    tcp.window;
    tcp.urgentPtr;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
field_list_calculation inner_ipv4_tcp_checksum {
    input {
        inner_ipv4_tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list inner_ipv6_tcp_checksum_list {
    inner_ipv6.srcAddr;
    inner_ipv6.dstAddr;
    8'0;
    inner_ipv6.nextHdr;
    nat_metadata.tcp_checksum_len;
    tcp.srcPort;
    tcp.dstPort;
    tcp.seqNo;
    tcp.ackNo;
    tcp.dataOffset;
    tcp.res;
    tcp.flags;
    tcp.window;
    tcp.urgentPtr;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum verify_len parser_csum.inner_ipv6_payloadLen
field_list_calculation inner_ipv6_tcp_checksum {
    input {
        inner_ipv6_tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field tcp.checksum {
    verify inner_ipv4_tcp_checksum;
    verify inner_ipv6_tcp_checksum;
    update inner_ipv4_tcp_checksum if (nat_metadata.update_inner_checksum == TRUE);
    update inner_ipv6_tcp_checksum if (valid(inner_ipv6));
}

field_list inner_ipv4_udp_checksum_list {
    inner_ipv4.srcAddr;
    inner_ipv4.dstAddr;
    8'0;
    inner_ipv4.protocol;
    inner_udp.len;
    inner_udp.srcPort;
    inner_udp.dstPort;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
field_list_calculation inner_ipv4_udp_checksum {
    input {
        inner_ipv4_udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list inner_ipv6_udp_checksum_list {
    inner_ipv6.srcAddr;
    inner_ipv6.dstAddr;
    8'0;
    inner_ipv6.nextHdr;
    inner_udp.srcPort;
    inner_udp.dstPort;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum verify_len parser_csum.inner_ipv6_payloadLen
field_list_calculation inner_ipv6_udp_checksum {
    input {
        inner_ipv6_udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field inner_udp.checksum {
    verify inner_ipv4_udp_checksum;
    verify inner_ipv6_udp_checksum;
    update inner_ipv4_udp_checksum if (nat_metadata.update_inner_checksum == TRUE);
    update inner_ipv6_udp_checksum if (valid(inner_ipv6));
}

parser parse_inner_udp {
    extract(inner_udp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.srcPort);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.dstPort);
    return parse_dummy;
    //return ingress;
}

@pragma deparse_only
parser parse_dummy {
    // This state is added as a work-around until NCC has a fix for handling 
    // hdr unions and same set_metadata from multiple states while computing topo-graph
    return select (inner_udp.srcPort) {
        default: ingress;
        0x1 mask 0x0000 : parse_v6_ipsec_esp_hdr;
        0x2 mask 0x0000 : parse_ipsec_ah;
        0x3 mask 0x0000 : parse_ipsec_esp;
        0x4 mask 0x0000 : parse_icmp;
        0x5 mask 0x0000 : parse_tcp;
        0x6 mask 0x0000 : parse_v6_ipsec_ah_hdr;
    }
}

/* This state extracts 72 bytes (40 bytes of header + 32 bytes to lkp)
 * Capri-Parser only allows for max of 64 bytes per state. Capri-ncc will
 * check this and internally split this state. Until then this code is
 * re-written by splitting the parse_inner_ipv6 state into two states
 */
parser parse_inner_ipv6 {
    extract(inner_ipv6);
    set_metadata(flow_lkp_metadata.lkp_src, latest.srcAddr);
    set_metadata(flow_lkp_metadata.lkp_dst, latest.dstAddr);
    set_metadata(parser_csum.inner_ipv6_payloadLen, inner_ipv6.payloadLen+0);
    return select(latest.nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_tcp;
        IP_PROTO_UDP : parse_inner_udp;
        default: ingress;
    }
}

parser parse_inner_ethernet {
    extract(inner_ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_inner_ipv4;
        ETHERTYPE_IPV6 : parse_inner_ipv6;
        default: ingress;
    }
}
