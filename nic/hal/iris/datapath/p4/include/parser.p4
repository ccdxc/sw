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
        parse_tcp_counter       : 8;
        ipv6_nextHdr            : 8;
        inner_ipv6_nextHdr      : 8;
        l4_trailer              : 16;
        l4_len                  : 16;
        ip_options_len          : 16;
        inner_ip_options_len    : 16;
        icrc                    : 32;
        icrc_len                : 16;
        udp_len                 : 16;
    }
}
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

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
        inner_ipv6_options_blob___hdr_len : 16;
        ipv4___start_off            : 16;
        ipv6___start_off            : 16;
        inner_ipv4___start_off      : 16;
        inner_ipv6___start_off      : 16;
        udp___start_off             : 16;
        inner_udp___start_off       : 16;
        tcp___start_off             : 16;
        udp_opt_ocs___start_off     : 16;
        roce_bth___start_off        : 16; // ohi allocated to store hdr offset
        icmp___start_off            : 16;
        // Write only variable to capture length value from pkt for csum puporses into OHI.
        ipv4_options_blob___hdr_len : 16;
        inner_ipv4_options_blob___hdr_len : 16;
        l3_len                      : 16;   // l3 len - used for pkt_len check
        inner_l3_len                : 16;   // l3 len - used for pkt_len check
        l4_len                      : 16;
        inner_l4_len                : 16;
        gso_start                   : 16;
        gso_offset                  : 16;
        kind                        : 16; //UDP option chksum needs start in OHI (GSO engine is used)
        chksum                      : 16; //UDP option chksum needs location in OHI (GSO engine is used)
        udp_opt_len                 : 16; //ohi variable that captures option len
        icrc_len                    : 16;

        ipv4_options_blob2___start_off          : 16;
        inner_ipv4_options_blob2___start_off    : 16;
        ipv4_options_blob2___hdr_len            : 16;
        inner_ipv4_options_blob2___hdr_len      : 16;
    }
}

@pragma parser_write_only
@pragma parser_share_ohi icrc_l3 ipv4___start_off ipv6___start_off
@pragma parser_share_ohi icrc_inner_l3 inner_ipv4___start_off inner_ipv6___start_off
metadata parser_ohi_t ohi;

/* tag depths */
#define MPLS_DEPTH 3

#define PARSE_ETHERTYPE                                    \
        ETHERTYPE_VLAN : parse_vlan;                       \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        default: ingress

#define PARSE_ETHERTYPE_MINUS_VLAN                         \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        default: ingress

header cap_phv_intr_global_t capri_intrinsic;
header cap_phv_intr_p4_t    capri_p4_intrinsic;
header cap_phv_intr_rxdma_t capri_rxdma_intrinsic;
header cap_phv_intr_txdma_t capri_txdma_intrinsic;
header tm_replication_data_t tm_replication_data;

// dummy headers that are extracted and thrown away (rxdma span copies)
header cap_phv_intr_rxdma_t              e2e_capri_rxdma_intrinsic;
header p4_to_p4plus_classic_nic_header_t e2e_p4_to_p4plus_classic_nic;
header p4_to_p4plus_ip_addr_t            e2e_p4_to_p4plus_classic_nic_ip;

@pragma pa_header_union egress p4_to_p4plus_roce_eth
header ethernet_t ethernet;
header recirc_header_t recirc_header;
header llc_header_t llc_header;
header snap_header_t snap_header;
@pragma pa_header_union ingress p4plus_to_p4_vlan
@pragma pa_header_union egress p4_to_p4plus_roce_vlan
header vlan_tag_t vlan_tag;
header mpls_t mpls[MPLS_DEPTH];

@pragma pa_header_union egress ipv6 p4_to_p4plus_roce_ipv6
@pragma pa_header_union ingress ipv6
header ipv4_t ipv4;
header ipv6_t ipv6;
header icmp_t icmp;
header ah_t ah;
header esp_t esp;

header udp_t udp;
// udp payload is treated as header when parsing udp trailer options
@pragma hdr_len parser_metadata.l4_len
header udp_payload_t udp_payload;

@pragma pa_header_union xgress vxlan_gpe genv nvgre mpls[0]
header vxlan_t vxlan;
header vxlan_gpe_t vxlan_gpe;
header genv_t genv;

header roce_bth_t roce_bth;
header roce_deth_t roce_deth;
header roce_deth_immdt_t roce_deth_immdt;
header icrc_t icrc;

header gre_t gre;
header nvgre_t nvgre;
header gre_opt_seq_t gre_opt_seq;
header erspan_t2_t erspan_t2;
header erspan_t3_t erspan_t3;
header erspan_t3_opt_t erspan_t3_opt;

@pragma pa_header_union ingress inner_udp icmp
header tcp_t tcp;

header icmp_echo_req_reply_t icmp_echo;

// TCP options
@pragma hdr_len parser_metadata.parse_tcp_counter
header tcp_options_blob_t tcp_options_blob;
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
@pragma no_ohi egress
header tcp_option_sack_perm_t tcp_option_sack_perm;
@pragma pa_header_union egress tcp_option_two_sack tcp_option_three_sack tcp_option_four_sack
@pragma no_ohi egress
header tcp_option_one_sack_t tcp_option_one_sack;
@pragma no_ohi egress
header tcp_option_two_sack_t tcp_option_two_sack;
@pragma no_ohi egress
header tcp_option_three_sack_t tcp_option_three_sack;
@pragma no_ohi egress
header tcp_option_four_sack_t tcp_option_four_sack;
@pragma no_ohi xgress
header tcp_option_timestamp_t tcp_option_timestamp;

// UDP options
@pragma no_ohi xgress
header udp_opt_eol_t udp_opt_eol;
@pragma no_ohi xgress
header udp_opt_nop_t udp_opt_nop;
@pragma no_ohi xgress
header udp_opt_ocs_t udp_opt_ocs;
@pragma no_ohi xgress
header udp_opt_mss_t udp_opt_mss;
@pragma no_ohi xgress
header udp_opt_timestamp_t udp_opt_timestamp;
header udp_opt_unknown_t udp_opt_unknown;

// IPv4 Options
@pragma hdr_len parser_metadata.ip_options_len
header ipv4_options_blob_t ipv4_options_blob;
header ipv4_options_blob_t ipv4_options_blob2;
header ipv4_option_eol_t ipv4_option_eol;
header ipv4_option_nop_t ipv4_option_nop;
header ipv4_option_generic_t ipv4_option_generic;

// Inner IPv4 Options
@pragma hdr_len parser_metadata.inner_ip_options_len
header ipv4_options_blob_t inner_ipv4_options_blob;
header ipv4_options_blob_t inner_ipv4_options_blob2;
header ipv4_option_eol_t inner_ipv4_option_eol;
header ipv4_option_nop_t inner_ipv4_option_nop;
header ipv4_option_generic_t inner_ipv4_option_generic;

header ethernet_t inner_ethernet;
@pragma pa_header_union xgress inner_ipv6
header ipv4_t inner_ipv4;
header ipv6_t inner_ipv6;
header udp_t inner_udp;

// IPv6 extension headers
@pragma hdr_len parser_metadata.ip_options_len
header ipv6_options_blob_t ipv6_options_blob;
header ipv6_extn_generic_t v6_generic;
header ipv6_extn_frag_t v6_fragment;

// Inner IPv6 extension headers
@pragma hdr_len parser_metadata.inner_ip_options_len
header ipv6_options_blob_t inner_ipv6_options_blob;
header ipv6_extn_generic_t inner_v6_generic;
header ipv6_extn_frag_t inner_v6_fragment;

// name 'capri_i2e_metadata' has a special meaning
header capri_i2e_metadata_t capri_i2e_metadata;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_roce.roce_opt_ts_value   udp_opt_timestamp.ts_value
@pragma pa_field_union egress p4_to_p4plus_roce.roce_opt_ts_echo    udp_opt_timestamp.ts_echo
@pragma pa_field_union egress p4_to_p4plus_roce.roce_opt_mss        udp_opt_mss.mss
@pragma pa_field_union egress p4_to_p4plus_roce.roce_opt_pad        tcp.srcPort
@pragma pa_field_union egress p4_to_p4plus_roce.roce_int_recirc_hdr tcp.dstPort
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
header vlan_tag_t p4_to_p4plus_roce_vlan;
@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.pad0           tcp_option_four_sack.first_le
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.pad1           tcp_option_four_sack.first_re
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.pad2           tcp_option_four_sack.second_le
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.pad3           tcp_option_four_sack.second_re
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.pad4           tcp_option_four_sack.third_le
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.version        ipv4.version
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.ihl            ipv4.ihl
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.diffserv       ipv4.diffserv
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.totalLen       ipv4.totalLen
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.identification ipv4.identification
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.flags          ipv4.flags
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.fragOffset     ipv4.fragOffset
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.ttl            ipv4.ttl
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.protocol       ipv4.protocol
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.hdrChecksum    ipv4.hdrChecksum
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.srcAddr        ipv4.srcAddr
@pragma pa_field_union egress p4_to_p4plus_roce_ipv4.dstAddr        ipv4.dstAddr
header roce_ipv4_t p4_to_p4plus_roce_ipv4;
@pragma synthetic_header
header ipv6_t p4_to_p4plus_roce_ipv6;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_classic_nic_ip.ip_sa        ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_classic_nic_ip.ip_da        ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_classic_nic_ip;
@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_classic_nic_inner_ip.ip_sa  inner_ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_classic_nic_inner_ip.ip_da  inner_ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_classic_nic_inner_ip;

@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_cpu_ip.ip_sa             ipv6.srcAddr
@pragma pa_field_union egress p4_to_p4plus_cpu_ip.ip_da             ipv6.dstAddr
header p4_to_p4plus_ip_addr_t p4_to_p4plus_cpu_ip;
@pragma synthetic_header
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_flags    tcp.flags
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_seqNo    tcp.seqNo
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_ackNo    tcp.ackNo
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_window   tcp.window
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_mss      tcp_option_mss.value
@pragma pa_field_union egress p4_to_p4plus_cpu_tcp_pkt.tcp_ws       tcp_option_ws.value
header p4_to_p4plus_cpu_tcp_pkt_t p4_to_p4plus_cpu_tcp_pkt;

@pragma pa_header_union egress p4_to_p4plus_p4pt p4_to_p4plus_ipsec p4_to_p4plus_cpu_pkt p4_to_p4plus_mirror
header p4_to_p4plus_classic_nic_header_t p4_to_p4plus_classic_nic;
header p4_to_p4plus_p4pt_header_t p4_to_p4plus_p4pt;
header p4_to_p4plus_ipsec_header_t p4_to_p4plus_ipsec;
header p4_to_p4plus_cpu_header_t p4_to_p4plus_cpu;
header p4_to_p4plus_cpu_pkt_t p4_to_p4plus_cpu_pkt;
header p4_to_p4plus_mirror_t p4_to_p4plus_mirror;

header p4plus_to_p4_s1_t p4plus_to_p4;
header p4plus_to_p4_s2_t p4plus_to_p4_vlan;

parser start {
    extract(capri_intrinsic);
    set_metadata(control_metadata.tm_iport, capri_intrinsic.tm_iport + 0);
    set_metadata(qos_metadata.qos_class_id, capri_intrinsic.tm_oq);
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
        TM_INSTANCE_TYPE_MULTICAST : parse_tm_replication_data;
        TM_INSTANCE_TYPE_SPAN : parse_span_copy_set;
        TM_INSTANCE_TYPE_SPAN_AND_DROP : parse_span_copy_set;
        default : parse_i2e_metadata1;
    }
}

@pragma xgress egress
parser parse_egress_to_egress {
    extract(capri_p4_intrinsic);
    return select(capri_intrinsic.tm_oport) {
        TM_PORT_DMA : parse_egress_to_egress_rxdma;
        default : parse_egress_to_egress_common;
    }
}

@pragma xgress egress
@pragma no_extract
parser parse_egress_to_egress_rxdma {
    extract(e2e_capri_rxdma_intrinsic);
    extract(e2e_p4_to_p4plus_classic_nic);
    extract(e2e_p4_to_p4plus_classic_nic_ip);
    return parse_egress_to_egress_common;
}

@pragma xgress egress
parser parse_egress_to_egress_common {
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
    extract(capri_p4_intrinsic);
    return parse_i2e_metadata;
}
@pragma xgress egress
parser parse_i2e_metadata {
    extract(capri_i2e_metadata);
    return select(capri_intrinsic.tm_instance_type) {
        TM_INSTANCE_TYPE_SPAN : parse_ethernet_span_copy;
        TM_INSTANCE_TYPE_SPAN_AND_DROP : parse_ethernet_span_copy;
        default : parse_ethernet;
    }
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_span_copy_set {
    // This state is created as span_copy is used as predicate (goes into
    // flit0) while i2e_metadata can go into flit1 which causes problem
    // (go back in flits) for parser
    set_metadata(control_metadata.span_copy, 1);
    return parse_i2e_metadata1;
}

@pragma xgress egress
@pragma allow_set_meta control_metadata.span_copy
parser parse_span_copy {
    set_metadata(control_metadata.span_copy, 1);
    return parse_ethernet_span_copy;
}

@pragma xgress egress
@pragma capture_payload_offset
parser parse_ethernet_span_copy {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_VLAN : parse_vlan_span_copy;
        default : ingress;
    }
}

@pragma xgress egress
@pragma dont_capture_payload_offset
parser parse_vlan_span_copy {
    extract(vlan_tag);
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
    extract(p4_to_p4plus_cpu_tcp_pkt);
    extract(p4_to_p4plus_p4pt);
    extract(p4_to_p4plus_mirror);
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
    extract(p4plus_to_p4_vlan);
    set_metadata(ohi.gso_start, p4plus_to_p4.gso_start + 0);
    set_metadata(ohi.gso_offset, p4plus_to_p4.gso_offset + 0);
    return select(p4plus_to_p4.gso_valid, p4plus_to_p4.flow_index_valid) {
        0x1:        parse_p4plus_to_p4_flow_index_valid;
        0x2:        parse_gso;
        0x3:        parse_p4plus_to_p4_flow_index_valid_gso_valid;
        default:    parse_ethernet;
    }
}

@pragma generic_checksum_start capri_gso_csum.gso_checksum
parser parse_p4plus_to_p4_flow_index_valid_gso_valid {
    set_metadata(control_metadata.skip_flow_lkp, 1);
    return parse_ethernet;
}

parser parse_p4plus_to_p4_flow_index_valid {
    set_metadata(control_metadata.skip_flow_lkp, 1);
    return parse_ethernet;
}

@pragma generic_checksum_start capri_gso_csum.gso_checksum
parser parse_gso {
    return parse_ethernet;
}

parser parse_nic {
    set_metadata(control_metadata.uplink, 1);
    return parse_ethernet;
}

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
    extract(capri_p4_intrinsic);
    return parse_i2e_metadata;
}

parser parse_ethernet {
    set_metadata(control_metadata.parser_outer_eth_offset, current+0);
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
    return select(current(23,1)) {
        0 : ingress;
        1 : parse_mpls_bos;
    }
}

parser parse_mpls_bos {
    extract(mpls[0]);
    return select(current(32, 4)) {
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

@pragma checksum hdr_len_expr ohi.ipv4_options_blob___hdr_len + 20
@pragma checksum verify_len ohi.l4_len
@pragma checksum update_len capri_deparser_len.ipv4_hdr_len
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
    roce_bth.reserved1;
}
@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc verify_len ohi.icrc_len
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
    roce_bth.reserved1;
}

@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc verify_len ohi.icrc_len
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
    roce_bth.reserved1;
}
@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc verify_len ohi.icrc_len
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
    roce_bth.reserved1;
}

@pragma icrc update_len capri_deparser_len.icrc_payload_len
@pragma icrc verify_len ohi.icrc_len
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

    verify inner_ipv4_roce_icrc if (valid(roce_bth));
    verify inner_ipv6_roce_icrc if (valid(roce_bth));
}

parser parse_ipv4_frag {
    extract(ipv4);
    set_metadata(l3_metadata.ip_frag, 1);
    return ingress;
}

parser parse_base_ipv4 {
    extract(ipv4);
    set_metadata(parser_metadata.l4_trailer, ipv4.totalLen - (ipv4.ihl << 2));
    set_metadata(parser_metadata.ip_options_len,
                 parser_metadata.ip_options_len + 0);
    return select(ipv4.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP  : parse_ipv4_tcp;
        IP_PROTO_UDP  : parse_udp;
        IP_PROTO_GRE  : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP : parse_ipsec_esp;
        default: ingress;
    }
}

@pragma no_extract
parser parse_ipv4_option_eol {
    extract(ipv4_option_eol);
    set_metadata(parser_metadata.ip_options_len,
                 parser_metadata.ip_options_len - 1);
     return parse_ipv4_options;
}

@pragma no_extract
parser parse_ipv4_option_nop {
    extract(ipv4_option_nop);
    set_metadata(parser_metadata.ip_options_len,
                 parser_metadata.ip_options_len - 1);
    return parse_ipv4_options;
}

@pragma no_extract
parser parse_ipv4_option_generic {
    extract(ipv4_option_generic);
    set_metadata(parser_metadata.ip_options_len,
                 parser_metadata.ip_options_len - ipv4_option_generic.len);
    return parse_ipv4_options;
}

@pragma header_ordering ipv4_option_generic ipv4_option_nop ipv4_option_eol
parser parse_ipv4_options {
    return select(parser_metadata.ip_options_len, current(0, 8)) {
        0x0000 mask 0xff00  : parse_ipv4_after_options;
        0x0000 mask 0x00ff  : parse_ipv4_option_eol;
        0x0001 mask 0x00ff  : parse_ipv4_option_nop;
        default             : parse_ipv4_option_generic;
    }
}

@pragma dont_advance_packet
parser parse_ipv4_options_blob {
    set_metadata(ohi.inner_ipv4_options_blob2___start_off, current + 0);
    set_metadata(ohi.inner_ipv4_options_blob2___hdr_len,
                 parser_metadata.ip_options_len - 1);
    extract(ipv4_options_blob);
    set_metadata(l3_metadata.ip_option_seen, 1);
    return select(current(0, 8)) {
        default : parse_ipv4_options;
        0xff mask 0x00 : parse_ipv4_options_blob_deparse;
    }
}

@pragma xgress egress
@pragma deparse_only
parser parse_ipv4_options_blob_deparse {
    extract(ipv4_options_blob2);
    return parse_ipv4_options;
}

parser parse_ipv4_with_options {
    extract(ipv4);
    set_metadata(parser_metadata.l4_trailer, ipv4.totalLen - (ipv4.ihl << 2));
    return parse_ipv4_options_blob;
}

parser parse_ipv4_after_options {
    return select(ipv4.protocol) {
        IP_PROTO_ICMP : parse_icmp;
        IP_PROTO_TCP  : parse_ipv4_tcp;
        IP_PROTO_UDP  : parse_udp;
        IP_PROTO_GRE  : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP : parse_ipsec_esp;
        default: ingress;
    }
}

@pragma packet_len_check outer_ipv4 len gt ohi.l3_len
@pragma packet_len_check outer_ipv4 start ohi.ipv4___start_off
parser parse_ipv4 {
    set_metadata(parser_metadata.ip_options_len, (current(4,4) << 2) - 20);
    set_metadata(ohi.ipv4_options_blob___hdr_len, (current(4,4) << 2) - 20);
    set_metadata(ohi.l3_len, current(16,16) + 0);
    set_metadata(ohi.l4_len, current(16,16) + 0);
    return select(current(48,16), current(0,8)) {
        0x000045 mask 0x3fffff : parse_base_ipv4;
        0x000044 mask 0x3fffff : ingress;
        0x000040 mask 0x3ffffc : ingress;
        0x000040 mask 0x3ffff0 : parse_ipv4_with_options;
        0x000000 mask 0x3fff00 : parse_base_ipv4;
        default : parse_ipv4_frag;
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

parser parse_ipv6_tcp {
    set_metadata(ohi.l4_len, parser_metadata.l4_trailer -
                 parser_metadata.ip_options_len + 20);
    return parse_tcp_ipv6;
}

parser parse_ipv4_tcp {
    set_metadata(ohi.l4_len, parser_metadata.l4_trailer + 0);
    return parse_tcp_ipv4;
}

parser parse_inner_ipv6_tcp {
    set_metadata(ohi.inner_l4_len, parser_metadata.l4_trailer + 0);
    return parse_tcp_ipv6;
}

parser parse_inner_ipv4_tcp {
    set_metadata(ohi.inner_l4_len, parser_metadata.l4_trailer + 0);
    return parse_tcp_ipv4;
}

parser parse_v6_generic_ext_hdr {
    set_metadata(parser_metadata.ipv6_nextHdr, current(0, 8) + 0);
    return parse_v6_generic_ext_hdr2;
}

@pragma no_extract
parser parse_v6_generic_ext_hdr2 {
    extract(v6_generic);
    set_metadata(l3_metadata.ipv6_ulp, parser_metadata.ipv6_nextHdr);
    set_metadata(parser_metadata.ip_options_len,
                 parser_metadata.ip_options_len + (v6_generic.len << 3) + 8);
    set_metadata(parser_metadata.ipv6_nextHdr, parser_metadata.ipv6_nextHdr + 0);
    return parse_ipv6_extn_hdrs;
}

parser parse_v6_fragment_hdr {
    set_metadata(l3_metadata.ip_frag, 1);
    set_metadata(parser_metadata.ipv6_nextHdr, current(0, 8) + 0);
    return parse_v6_fragment_hdr2;
}

@pragma no_extract
parser parse_v6_fragment_hdr2 {
    extract(v6_fragment);
    set_metadata(l3_metadata.ipv6_ulp, parser_metadata.ipv6_nextHdr);
    // options len + 8 - 20
    set_metadata(ohi.ipv6_options_blob___hdr_len,
                 parser_metadata.ip_options_len -12);
    set_metadata(l3_metadata.ipv6_options_len,
                 parser_metadata.ip_options_len - 12);
    return ingress;
}

@pragma dont_advance_packet
parser parse_ipv6_option_blob {
    set_metadata(ohi.ipv6_options_blob___hdr_len,
                 parser_metadata.ip_options_len + 0);
    set_metadata(l3_metadata.ipv6_options_len, parser_metadata.ip_options_len);
    // ipv6_option_blob header uses ip_options_len as variable header len
    // setup the ip_options_len = 0 and keep updating the len as options are
    // parsed.
    // Note1: that options are parsed using 'no_extract' state i.e. they are not
    // individually extracted to phv/ohi (but hv bits will be set)
    // Note2: ip_options_len value used to setup ohi slot is previous value (before init)
    // make if common for v4 and v6 and save mux inst XXXX
    // For icrc len calculation, 48 bytes (40byte v6 header and 8 bytes of 1's) need to be added.
    /// However here add only 20bytes because in parse_udp another 28 bytes are added.
    set_metadata(parser_metadata.ip_options_len, 20);
    extract(ipv6_options_blob);
    return parse_ipv6_extn_hdrs;
}

parser parse_ipv6_ulp {
    set_metadata(l3_metadata.ipv6_ulp, parser_metadata.ipv6_nextHdr);
    // update the header len of the ipv6_options_blob header
    // must use expression to update ohi variable.
    // Because ip_options_len has extra 20 bytes (set in ipv6_option_blob
    // state), blob header length should be 20 bytes less.
    set_metadata(ohi.ipv6_options_blob___hdr_len,
                 parser_metadata.ip_options_len - 20);
    set_metadata(l3_metadata.ipv6_options_len,
                 parser_metadata.ip_options_len - 20);
    return select(parser_metadata.ipv6_nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_ipv6_tcp;
        IP_PROTO_UDP : parse_udp;
        IP_PROTO_GRE : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP : parse_ipsec_esp;
        default: ingress;
    }
}

@pragma header_ordering v6_generic
parser parse_ipv6_extn_hdrs {
    set_metadata(l3_metadata.ip_option_seen, 1);
    return select(parser_metadata.ipv6_nextHdr) {
        IPV6_PROTO_EXTN_HOPBYHOP :  parse_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_ROUTING_HDR : parse_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_FRAGMENT_HDR : parse_v6_fragment_hdr;
        IPV6_PROTO_EXTN_DEST_OPT_HDR : parse_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_MOBILITY_HDR : parse_v6_generic_ext_hdr;
        default: parse_ipv6_ulp;
    }
}

@pragma packet_len_check outer_ipv6 len gt ohi.l3_len + 40
@pragma packet_len_check outer_ipv6 start ohi.ipv6___start_off
parser parse_ipv6 {
    extract(ipv6);
    set_metadata(ohi.ipv6___start_off, current + 0);
    set_metadata(parser_metadata.ipv6_nextHdr, latest.nextHdr);
    set_metadata(parser_metadata.l4_trailer, ipv6.payloadLen);
    set_metadata(parser_metadata.ip_options_len, ipv6.payloadLen);
    set_metadata(l3_metadata.ipv6_ulp, latest.nextHdr);
    set_metadata(ohi.l4_len, ipv6.payloadLen + 0);
    set_metadata(ohi.l3_len, ipv6.payloadLen + 0);
    return select(parser_metadata.ipv6_nextHdr) {
        IPV6_PROTO_EXTN_HOPBYHOP:  parse_ipv6_option_blob;
        IPV6_PROTO_EXTN_ROUTING_HDR : parse_ipv6_option_blob;
        IPV6_PROTO_EXTN_FRAGMENT_HDR : parse_ipv6_option_blob;
        IPV6_PROTO_EXTN_DEST_OPT_HDR : parse_ipv6_option_blob;
        IPV6_PROTO_EXTN_MOBILITY_HDR : parse_ipv6_option_blob;
        default: parse_ipv6_ulp_no_options;
    }
}

parser parse_ipv6_ulp_no_options {
    set_metadata(parser_metadata.ip_options_len, 20);
    return select(parser_metadata.ipv6_nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_ipv6_tcp;
        IP_PROTO_UDP : parse_udp;
        IP_PROTO_GRE : parse_gre;
        IP_PROTO_IPV4 : parse_ipv4_in_ip;
        IP_PROTO_IPV6 : parse_ipv6_in_ip;
        IP_PROTO_IPSEC_AH : parse_ipsec_ah;
        IP_PROTO_IPSEC_ESP : parse_ipsec_esp;
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
    extract(icmp);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.typeCode);
    return select(latest.typeCode) {
        ICMP6_ECHO_REQ_TYPE_CODE : parse_icmp_echo_req_reply;
        ICMP6_ECHO_REPLY_TYPE_CODE : parse_icmp_echo_req_reply;
        default: ingress;
    }
}

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
    capri_deparser_len.inner_l4_payload_len;
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
@pragma checksum verify_len ohi.l4_len
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
    capri_deparser_len.inner_l4_payload_len;
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
@pragma checksum verify_len ohi.l4_len
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
    update ipv4_tcp_checksum if (valid(ipv4));
    update ipv6_tcp_checksum if (valid(ipv6));
}

@pragma capture_payload_offset
parser parse_tcp_ipv6 {
    extract(tcp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.srcPort);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.dstPort);
    set_metadata(parser_metadata.parse_tcp_counter, (tcp.dataOffset << 2) - 20);
    return select(parser_metadata.parse_tcp_counter) {
        0 : ingress;
        0x80 mask 0x80: parse_tcp_option_error;
        default : parse_tcp_options_blob;
    }
}

@pragma capture_payload_offset
parser parse_tcp_ipv4 {
    extract(tcp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.srcPort);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.dstPort);
    set_metadata(parser_metadata.parse_tcp_counter, (tcp.dataOffset << 2) - 20);
    return select(parser_metadata.parse_tcp_counter) {
        0 : ingress;
        0x80 mask 0x80: parse_tcp_option_error;
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
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_two_sack {
    extract(tcp_option_two_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 18);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_three_sack {
    extract(tcp_option_three_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 26);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_four_sack {
    extract(tcp_option_four_sack);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 34);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_EOL {
    extract(tcp_option_eol);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 1);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_NOP {
    extract(tcp_option_nop);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 1);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
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
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_ws {
    extract(tcp_option_ws);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 3);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_option_sack_perm {
    extract(tcp_option_sack_perm);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 2);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_timestamp {
    extract(tcp_option_timestamp);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 10);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

parser parse_tcp_unknown_option {
    extract(tcp_option_unknown);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - tcp_option_unknown.optLength);
    return select(parser_metadata.parse_tcp_counter) {
        0x00 : ingress;
        default : parse_tcp_options;
    }
}

@pragma dont_capture_payload_offset
parser parse_tcp_option_error {
    set_metadata(control_metadata.parse_tcp_option_error, 1);
    return ingress;
}

@pragma dont_capture_payload_offset
parser parse_tcp_option_error2 {
    set_metadata(control_metadata.parse_tcp_option_error, 1);
    return ingress;
}

@pragma header_ordering tcp_option_mss tcp_option_ws tcp_option_sack_perm tcp_option_one_sack tcp_option_two_sack tcp_option_three_sack tcp_option_four_sack tcp_option_timestamp tcp_option_unknown tcp_option_nop tcp_option_nop_1 tcp_option_eol
parser parse_tcp_options {
    return select(parser_metadata.parse_tcp_counter, current(0, 8)) {
        0x0000 mask 0xff00 : ingress;
        0x8000 mask 0x8000 : parse_tcp_option_error;
        0x0000 mask 0x00ff : parse_tcp_option_EOL;
        0x0001 mask 0x00ff : parse_tcp_option_NOP;
        default : parse_tcp_multibyte_options;
        0x0001 mask 0x0000 : parse_tcp_deparse_options;
    }
}

parser parse_tcp_multibyte_options {
    return select(current(0, 16)) {
        0x0204 mask 0xffff : parse_tcp_option_mss;
        0x0200 mask 0xff00 : parse_tcp_option_error2;
        0x0303 mask 0xffff : parse_tcp_option_ws;
        0x0300 mask 0xff00 : parse_tcp_option_error2;
        0x0402 mask 0xffff : parse_tcp_option_sack_perm;
        0x0400 mask 0xff00 : parse_tcp_option_error2;
        0x050a mask 0xffff : parse_tcp_option_one_sack;
        0x0512 mask 0xffff : parse_tcp_option_two_sack;
        0x051a mask 0xffff : parse_tcp_option_three_sack;
        0x0522 mask 0xffff : parse_tcp_option_four_sack;
        0x0500 mask 0xff00 : parse_tcp_option_error2;
        0x080a mask 0xffff : parse_tcp_timestamp;
        0x0800 mask 0xff00 : parse_tcp_option_error2;
        0x0000 mask 0x00ff : parse_tcp_option_error2;
        default: parse_tcp_unknown_option;
    }
}

parser parse_roce_v2_pre {
    set_metadata(parser_metadata.l4_trailer,
                 parser_metadata.l4_trailer - parser_metadata.ip_options_len);
    // subtract 24 (8B UDP header, 12B BTH, 4B iCRC)
    set_metadata(parser_metadata.l4_len, parser_metadata.ip_options_len - 24);
    return parse_roce_v2;
}

parser parse_roce_v2 {
    extract(roce_bth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len+0);
    return select(latest.opCode) {
        0x64 : parse_roce_deth;
        0x65 : parse_roce_deth_immdt;
        default : parse_udp_payload;
    }
}

parser parse_roce_deth {
    extract(roce_deth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-8);
    return select(latest.reserved) {
        default : parse_udp_payload;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

parser parse_roce_deth_immdt {
    extract(roce_deth_immdt);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-12);
    return select(latest.reserved) {
        default : parse_udp_payload;
        0x1 mask 0x0 : parse_roce_eth;
    }
}

@pragma xgress egress
parser parse_udp_payload {
    // dummy set_metadata to make ncc compiler reserve a register (wka)
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len + 0);
    set_metadata(ohi.udp_opt_len, parser_metadata.l4_trailer + 0);
    extract(udp_payload);
    return select(parser_metadata.l4_trailer) {
        0x0000 mask 0xffff: ingress;
        default : parse_udp_trailer;
    }
}

@pragma xgress egress
parser parse_udp_trailer {
    extract(icrc);
    return parse_udp_options;
}

@pragma xgress egress
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

@pragma xgress egress
parser parse_udp_option_eol {
    extract(udp_opt_eol);
    return ingress;
}

@pragma xgress egress
parser parse_udp_option_nop {
    extract(udp_opt_nop);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 1);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

@pragma xgress egress
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

@pragma xgress egress
parser parse_udp_option_mss {
    extract(udp_opt_mss);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 4);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

@pragma xgress egress
parser parse_udp_option_timestamp {
    extract(udp_opt_timestamp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - 10);
    return select(parser_metadata.l4_trailer) {
        0 : ingress;
        default : parse_udp_options;
    }
}

@pragma xgress egress
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

@pragma xgress egress
@pragma deparse_only
parser parse_roce_eth {
    extract(p4_to_p4plus_roce_eth);
    extract(p4_to_p4plus_roce_vlan);
    extract(p4_to_p4plus_roce_ipv4);
    extract(p4_to_p4plus_roce_ipv6);
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
@pragma checksum verify_len  ohi.l4_len
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
@pragma checksum verify_len ohi.l4_len
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
    update ipv4_udp_checksum if (valid(ipv4));
    update ipv6_udp_checksum if (valid(ipv6));
}

parser parse_udp {
    extract(udp);
    set_metadata(ohi.icrc_len, parser_metadata.ip_options_len + udp.len + 28);
    set_metadata(ohi.l4_len, udp.len + 0);
    set_metadata(parser_metadata.ip_options_len, udp.len);
    return select(latest.dstPort) {
        UDP_PORT_VXLAN : parse_vxlan;
        UDP_PORT_GENV : parse_geneve;
        UDP_PORT_VXLAN_GPE : parse_vxlan_gpe;
        UDP_PORT_ROCE_V2: parse_roce_v2_pre;
        //UDP_PORT_NATT : parse_ipsec_esp;
        UDP_PORT_MPLS : parse_mpls;
        default: ingress;
    }
}

parser parse_gre {
    extract(gre);
    return select(latest.proto) {
        default : ingress;
        0x1 mask 0x0 : deparse_gre;
    }
}

@pragma xgress egress
@pragma deparse_only
parser deparse_gre {
    extract(gre_opt_seq);
    extract(erspan_t2);
    extract(erspan_t3);
    extract(erspan_t3_opt);
    return parse_inner_ethernet;
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

@pragma checksum hdr_len_expr ohi.inner_ipv4_options_blob___hdr_len + 20
@pragma checksum verify_len ohi.inner_l4_len
@pragma checksum update_len capri_deparser_len.inner_ipv4_hdr_len
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

parser parse_inner_ipv4_frag {
    extract(inner_ipv4);
    set_metadata(l3_metadata.inner_ip_frag, 1);
    return ingress;
}

parser parse_base_inner_ipv4 {
    extract(inner_ipv4);
    set_metadata(parser_metadata.l4_trailer, inner_ipv4.totalLen - (inner_ipv4.ihl << 2));
    set_metadata(parser_metadata.inner_ip_options_len,
                 parser_metadata.inner_ip_options_len + 0);
    return select(inner_ipv4.protocol) {
        IP_PROTO_ICMP: parse_icmp;
        IP_PROTO_TCP: parse_inner_ipv4_tcp;
        IP_PROTO_UDP: parse_inner_udp;
        default: ingress;
    }
}

@pragma no_extract
parser parse_inner_ipv4_option_eol {
     extract(inner_ipv4_option_eol);
     set_metadata(parser_metadata.inner_ip_options_len,
                  parser_metadata.inner_ip_options_len - 1);
     return parse_inner_ipv4_options;
}

@pragma no_extract
parser parse_inner_ipv4_option_nop {
     extract(inner_ipv4_option_nop);
     set_metadata(parser_metadata.inner_ip_options_len,
                  parser_metadata.inner_ip_options_len - 1);
     return parse_inner_ipv4_options;
}

@pragma no_extract
parser parse_inner_ipv4_option_generic {
    extract(inner_ipv4_option_generic);
    set_metadata(parser_metadata.inner_ip_options_len,
                 parser_metadata.inner_ip_options_len - inner_ipv4_option_generic.len);
    return parse_inner_ipv4_options;
}

@pragma header_ordering inner_ipv4_option_generic inner_ipv4_option_nop inner_ipv4_option_eol
parser parse_inner_ipv4_options {
    return select(parser_metadata.inner_ip_options_len, current(0, 8)) {
        0x0000 mask 0xff00  : parse_inner_ipv4_after_options;
        0x0000 mask 0x00ff  : parse_inner_ipv4_option_eol;
        0x0001 mask 0x00ff  : parse_inner_ipv4_option_nop;
        default             : parse_inner_ipv4_option_generic;
    }
}

@pragma dont_advance_packet
parser parse_inner_ipv4_options_blob {
    set_metadata(ohi.ipv4_options_blob2___start_off, current + 0);
    set_metadata(ohi.ipv4_options_blob2___hdr_len,
                 parser_metadata.inner_ip_options_len - 1);
    extract(inner_ipv4_options_blob);
    set_metadata(l3_metadata.inner_ip_option_seen, 1);
    return select(current(0, 8)) {
        default : parse_inner_ipv4_options;
        0xFF mask 0x00 : parse_inner_ipv4_options_blob_deparse;
    }
}

@pragma xgress egress
@pragma deparse_only
parser parse_inner_ipv4_options_blob_deparse {
    extract(inner_ipv4_options_blob2);
    return parse_inner_ipv4_options;
}

parser parse_inner_ipv4_with_options {
    extract(inner_ipv4);
    set_metadata(parser_metadata.l4_trailer, inner_ipv4.totalLen - (inner_ipv4.ihl << 2));
    return parse_inner_ipv4_options_blob;
}

parser parse_inner_ipv4_after_options {
    return select(inner_ipv4.protocol) {
        IP_PROTO_ICMP: parse_icmp;
        IP_PROTO_TCP: parse_inner_ipv4_tcp;
        IP_PROTO_UDP: parse_inner_udp;
        default: ingress;
    }
}

@pragma packet_len_check inner_ipv4 len gt ohi.inner_l3_len
@pragma packet_len_check inner_ipv4 start ohi.inner_ipv4___start_off
parser parse_inner_ipv4 {
    set_metadata(parser_metadata.inner_ip_options_len, (current(4,4) << 2) - 20);
    set_metadata(ohi.inner_ipv4_options_blob___hdr_len, (current(4,4) << 2) - 20);
    set_metadata(ohi.inner_l3_len, current(16,16) + 0);
    set_metadata(ohi.inner_l4_len, current(16,16) + 0);
    return select(current(48,16), current(0,8)) {
        0x000045 mask 0x3fffff : parse_base_inner_ipv4;
        0x000044 mask 0x3fffff : ingress;
        0x000040 mask 0x3ffffc : ingress;
        0x000040 mask 0x3ffff0 : parse_inner_ipv4_with_options;
        0x000000 mask 0x3fff00 : parse_base_inner_ipv4;
        default : parse_inner_ipv4_frag;
    }
}

field_list inner_ipv4_tcp_checksum_list {
    inner_ipv4.srcAddr;
    inner_ipv4.dstAddr;
    8'0;
    inner_ipv4.protocol;
    capri_deparser_len.inner_l4_payload_len;
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
@pragma checksum verify_len ohi.inner_l4_len
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
    capri_deparser_len.inner_l4_payload_len;
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
@pragma checksum verify_len ohi.inner_l4_len
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
    update inner_ipv4_tcp_checksum if (valid(inner_ipv4));
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
@pragma checksum verify_len ohi.inner_l4_len
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
@pragma checksum verify_len ohi.inner_l4_len
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
    update inner_ipv4_udp_checksum if (valid(inner_ipv4));
    update inner_ipv6_udp_checksum if (valid(inner_ipv6));
}

field_list udp_opt_checksum_list {
    udp_opt_ocs.kind; // First field in UDP options related to csum
    payload; // specify payload keyword as list of options
}

@pragma checksum verify_len ohi.udp_opt_len
@pragma checksum update_len capri_deparser_len.udp_opt_l2_checksum_len
@pragma checksum gress egress
@pragma checksum udp_option
@pragma checksum update_share udp_opt_ocs.chksum, p4_to_p4plus_classic_nic.csum
field_list_calculation udp_opt_checksum {
    input {
        udp_opt_checksum_list;
    }
    algorithm : csum8;
    output_width : 8;
}

calculated_field udp_opt_ocs.chksum {
    verify udp_opt_checksum;
    update udp_opt_checksum;
}

field_list gso_checksum_list {
    p4plus_to_p4.gso_start; // Gso start should be first field in the input list.
                            // 'gso_start' should also be included in write only ohi list.
    payload;
}

@pragma checksum update_len capri_gso_csum.gso_checksum // Specifies csum result location
@pragma checksum gso_checksum_offset p4plus_to_p4.gso_offset // Specifies csum location in packet
@pragma checksum gress ingress
field_list_calculation gso_checksum {
    input {
        gso_checksum_list;
    }
    algorithm : gso; //For GSO checksum, provide algorithm as gso
    output_width : 16;
}
calculated_field capri_gso_csum.gso_checksum {
    update gso_checksum;
}

parser parse_inner_udp {
    extract(inner_udp);
    set_metadata(flow_lkp_metadata.lkp_sport, latest.srcPort);
    set_metadata(flow_lkp_metadata.lkp_dport, latest.dstPort);
    set_metadata(ohi.icrc_len, parser_metadata.inner_ip_options_len + inner_udp.len + 28);
    set_metadata(ohi.inner_l4_len, inner_udp.len + 0);
    set_metadata(parser_metadata.inner_ip_options_len, inner_udp.len);
    return select(latest.dstPort) {
        UDP_PORT_ROCE_V2: parse_inner_roce_v2_pre;
        default:    ingress;
    }
}


parser parse_inner_roce_v2_pre {
    set_metadata(parser_metadata.l4_trailer,
                 parser_metadata.l4_trailer - parser_metadata.inner_ip_options_len);
    // subtract 24 (8B UDP header, 12B BTH, 4B iCRC)
    set_metadata(parser_metadata.l4_len, parser_metadata.inner_ip_options_len - 24);
    return parse_roce_v2;
}

@pragma dont_advance_packet
parser parse_inner_ipv6_option_blob {
    set_metadata(ohi.inner_ipv6_options_blob___hdr_len,
                 parser_metadata.inner_ip_options_len + 0);
    set_metadata(l3_metadata.inner_ipv6_options_len,
                 parser_metadata.inner_ip_options_len);
    // inner_ipv6_option_blob header uses inner_ip_options_len as variable header len
    // setup the inner_ip_options_len = 0 and keep updating the len as options are
    // parsed.
    // Note1: that options are parsed using 'no_extract' state i.e. they are not
    // individually extracted to phv/ohi (but hv bits will be set)
    // Note2: inner_ip_options_len value used to setup ohi slot is previous value (before init)
    // For icrc len calculation, 48 bytes (40byte v6 header and 8 bytes of 1's) need to be added.
    /// However here add only 20bytes because in parse_udp another 28 bytes are added.
    set_metadata(parser_metadata.inner_ip_options_len, 20);
    extract(inner_ipv6_options_blob);
    return parse_inner_ipv6_extn_hdrs;
}

parser parse_inner_ipv6_ulp {
    // update the header len of the inner_ipv6_options_blob header
    // must use expression to update ohi variable.
    // Because ip_options_len has extra 20 bytes (set in ipv6_option_blob
    // state), blob header length should be 20 bytes less.
    set_metadata(ohi.inner_ipv6_options_blob___hdr_len,
                 parser_metadata.inner_ip_options_len - 20);
    set_metadata(l3_metadata.inner_ipv6_options_len,
                 parser_metadata.inner_ip_options_len - 20);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer -
                 parser_metadata.inner_ip_options_len + 20);
    return select(parser_metadata.inner_ipv6_nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_inner_ipv6_tcp;
        IP_PROTO_UDP : parse_inner_udp;
        default: ingress;
    }
}

parser parse_inner_v6_generic_ext_hdr {
    set_metadata(parser_metadata.inner_ipv6_nextHdr, current(0, 8) + 0);
    return parse_inner_v6_generic_ext_hdr2;
}

@pragma no_extract
parser parse_inner_v6_generic_ext_hdr2 {
    extract(inner_v6_generic);
    set_metadata(l3_metadata.inner_ipv6_ulp,
                 parser_metadata.inner_ipv6_nextHdr);
    set_metadata(parser_metadata.inner_ip_options_len,
                 parser_metadata.inner_ip_options_len +
                 (inner_v6_generic.len << 3) + 8);
    set_metadata(parser_metadata.inner_ipv6_nextHdr,
                 parser_metadata.inner_ipv6_nextHdr + 0);
    return parse_inner_ipv6_extn_hdrs;
}

parser parse_inner_v6_fragment_hdr {
    set_metadata(l3_metadata.inner_ip_frag, 1);
    set_metadata(parser_metadata.inner_ipv6_nextHdr, current(0, 8) + 0);
    return parse_inner_v6_fragment_hdr2;
}

@pragma no_extract
parser parse_inner_v6_fragment_hdr2 {
    extract(inner_v6_fragment);
    set_metadata(l3_metadata.inner_ipv6_ulp,
                 parser_metadata.inner_ipv6_nextHdr);
    // options len + 8 - 20
    set_metadata(ohi.inner_ipv6_options_blob___hdr_len,
                 parser_metadata.inner_ip_options_len - 12);
    set_metadata(l3_metadata.inner_ipv6_options_len,
                 parser_metadata.inner_ip_options_len - 12);
    return ingress;
}

@pragma header_ordering inner_v6_generic
parser parse_inner_ipv6_extn_hdrs {
    set_metadata(l3_metadata.inner_ip_option_seen, 1);
    return select(parser_metadata.inner_ipv6_nextHdr) {
        IPV6_PROTO_EXTN_HOPBYHOP :  parse_inner_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_ROUTING_HDR : parse_inner_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_FRAGMENT_HDR : parse_inner_v6_fragment_hdr;
        IPV6_PROTO_EXTN_DEST_OPT_HDR : parse_inner_v6_generic_ext_hdr;
        IPV6_PROTO_EXTN_MOBILITY_HDR : parse_inner_v6_generic_ext_hdr;
        default: parse_inner_ipv6_ulp;
    }
}

@pragma packet_len_check inner_ipv6 len gt ohi.inner_l3_len + 40
@pragma packet_len_check inner_ipv6 start ohi.inner_ipv6___start_off
parser parse_inner_ipv6 {
    extract(inner_ipv6);
    set_metadata(parser_metadata.inner_ipv6_nextHdr, latest.nextHdr);
    set_metadata(flow_lkp_metadata.lkp_src, latest.srcAddr);
    set_metadata(flow_lkp_metadata.lkp_dst, latest.dstAddr);
    set_metadata(ohi.inner_l4_len, inner_ipv6.payloadLen + 0);
    set_metadata(ohi.inner_l3_len, inner_ipv6.payloadLen + 0);
    set_metadata(l3_metadata.inner_ipv6_ulp, latest.nextHdr);
    set_metadata(parser_metadata.l4_trailer, inner_ipv6.payloadLen);
    set_metadata(parser_metadata.inner_ip_options_len, 0);
    return select(parser_metadata.inner_ipv6_nextHdr) {
        IPV6_PROTO_EXTN_HOPBYHOP :  parse_inner_ipv6_option_blob;
        IPV6_PROTO_EXTN_ROUTING_HDR : parse_inner_ipv6_option_blob;
        IPV6_PROTO_EXTN_FRAGMENT_HDR : parse_inner_ipv6_option_blob;
        IPV6_PROTO_EXTN_DEST_OPT_HDR : parse_inner_ipv6_option_blob;
        IPV6_PROTO_EXTN_MOBILITY_HDR : parse_inner_ipv6_option_blob;
        default: parse_inner_ipv6_ulp_no_options;
    }
}

parser parse_inner_ipv6_ulp_no_options {
    set_metadata(parser_metadata.inner_ip_options_len, 20);
    return select(parser_metadata.inner_ipv6_nextHdr) {
        IP_PROTO_ICMPV6 : parse_icmpv6;
        IP_PROTO_TCP : parse_inner_ipv6_tcp;
        IP_PROTO_UDP : parse_inner_udp;
        default: ingress;
    }
}

parser parse_inner_ethernet {
    set_metadata(control_metadata.parser_inner_eth_offset, current+0);
    extract(inner_ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_inner_ipv4;
        ETHERTYPE_IPV6 : parse_inner_ipv6;
        default: ingress;
    }
}

field_list complete_checksum_ipv4_list {
    ipv4.srcAddr;
    payload;
}

field_list complete_checksum_ipv6_list {
    ipv6.srcAddr;
    payload;
}

@pragma checksum update_len capri_deparser_len.udp_opt_l2_checksum_len
//Share checksum engine among udp-option csum computation and l2_checksum
//as one is used in packet towards uplink and other in packet towards host.
@pragma checksum update_share udp_opt_ocs.chksum, p4_to_p4plus_classic_nic.csum
field_list_calculation complete_checksum_ipv4 {
    input {
        complete_checksum_ipv4_list;
    }
    algorithm : l2_complete_csum; // Used to indicate L2 Complete Csum
    output_width : 16;
}

@pragma checksum update_len capri_deparser_len.udp_opt_l2_checksum_len
//Share checksum engine among udp-option csum computation and l2_checksum
//as one is used in packet towards uplink and other in packet towards host.
@pragma checksum update_share udp_opt_ocs.chksum, p4_to_p4plus_classic_nic.csum
field_list_calculation complete_checksum_ipv6 {
    input {
        complete_checksum_ipv6_list;
    }
    algorithm : l2_complete_csum; // Used to indicate L2 Complete Csum
    output_width : 16;
}


field_list complete_checksum_vlan {
    vlan_tag.pcp;
    payload;
}
@pragma checksum update_len capri_deparser_len.udp_opt_l2_checksum_len
//Share checksum engine among udp-option csum computation and l2_checksum
//as one is used in packet towards uplink and other in packet towards host.
@pragma checksum update_share udp_opt_ocs.chksum, p4_to_p4plus_classic_nic.csum
field_list_calculation complete_checksum_vlan {
    input {
        complete_checksum_vlan;
    }
    algorithm : l2_complete_csum; // Used to indicate L2 Complete Csum
    output_width : 16;
}

calculated_field p4_to_p4plus_classic_nic.csum {
    update complete_checksum_vlan if (valid(vlan_tag));
    update complete_checksum_ipv4 if (valid(ipv4));
    update complete_checksum_ipv6 if (valid(ipv6));
}

field_list icmp_checksum_list {
#if 0
    icmp.typeCode;
    icmp.hdrChecksum;
    icmp_echo.identifier;
    icmp_echo.seqNum;
#endif
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum update_share icmp.hdrChecksum inner_udp.checksum
field_list_calculation icmp_checksum {
    input {
        icmp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list icmp_v6_checksum_list {
    ipv6.srcAddr;
    ipv6.dstAddr;
    // Since nextHdr is listed here as pseudo
    // header field, it has to appear at fixed
    // offset within ipv6 header. Hence ipv6
    // options are not supported with icmp pkt.
    ipv6.nextHdr;
    ipv6.payloadLen;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum update_share icmp.hdrChecksum inner_udp.checksum
field_list_calculation icmp_v6_checksum {
    input {
        icmp_v6_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list icmp_inner_v6_checksum_list {
    inner_ipv6.srcAddr;
    inner_ipv6.dstAddr;
    // Since nextHdr is listed here as pseudo
    // header field, it has to appear at fixed
    // offset within ipv6 header. Hence ipv6
    // options are not supported with icmp pkt.
    inner_ipv6.nextHdr;
    inner_ipv6.payloadLen;
    payload;
}

@pragma checksum update_len capri_deparser_len.inner_l4_payload_len
@pragma checksum update_share icmp.hdrChecksum inner_udp.checksum
field_list_calculation icmp_inner_v6_checksum {
    input {
        icmp_inner_v6_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field icmp.hdrChecksum {
    update icmp_checksum if (valid(icmp));
    update icmp_v6_checksum if (valid(icmp));
    update icmp_inner_v6_checksum if (valid(icmp));
}
