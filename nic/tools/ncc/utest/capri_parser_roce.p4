// Test program for roce pre-parser hardware
#include "includes/capri_intrinsic.p4"


header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type vlan_tag_t {
    fields {
        pcp : 3;
        dei : 1;
        vid : 12;
        etherType : 16;
    }
}

header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

header_type udp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        len : 16;
        checksum : 16;
    }
}

header_type ipv6_t {
    fields {
        version : 4;
        trafficClass : 8;
        flowLabel : 20;
        payloadLen : 16;
        nextHdr : 8;
        hopLimit : 8;
        srcAddr : 128;
        dstAddr : 128;
    }
}
// pre-parser is expected to check for udp options, so no need to extract udp_payload.
// extracting udp_payload will need more ohi slots i.e. extra state.
header_type udp_payload_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : (hdr_len);
    max_length : 65535;
}

header_type roce_bth_t {
    fields {
        opCode : 8;
        se : 1;
        m : 1;
        padCnt : 2;
        tVer : 4;
        pKey : 16;
        fr : 1;
        br : 1;
        reserved1 : 6;
        destQP : 24;
        ack : 1;
        reserved2 : 7;
        psn : 24;
    }
}

header_type roce_deth_t {
    fields {
        queueKey : 32;
        reserved : 8;
        srcQP : 24;
    }
}

header_type roce_deth_immdt_t {
    fields {
        queueKey : 32;
        reserved : 8;
        srcQP : 24;
        immDt : 32;
    }
}

header_type vxlan_t {
    fields {
        flags : 8;
        reserved : 24;
        vni : 24;
        reserved2 : 8;
    }
}

// Capri specific headers
header cap_phv_intr_global_t capri_intrinsic;
metadata cap_phv_intr_p4_t capri_p4_intrinsic;

header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len           : 16;
        l4_payload_len          : 16;
        inner_l4_payload_len    : 16;
        icrc_payload_len        : 16;
    }
}

header_type parser_metadata_t {
    fields {
        l4_len                          : 16;
        inner_l4_len                    : 16;
        icrc_len                        : 16;
        icrc                            : 32;
    }
}
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

header_type parser_ohi_t {
    fields {
        ipv4___start_off            : 16;
        ipv6___start_off            : 16;
        udp___start_off             : 16;
        inner_ipv4___start_off      : 16;
        inner_ipv6___start_off      : 16;
        inner_udp___start_off       : 16;
        l4_len                      : 16;
        inner_l4_len                : 16;
        icrc_len                    : 16;
        ipv4___hdr_len              : 16;
        inner_ipv4___hdr_len        : 16;
    }
}

@pragma parser_write_only
@pragma parser_share_ohi icrc_l3 ipv4___start_off ipv6___start_off
@pragma parser_share_ohi icrc_inner_l3 inner_ipv4___start_off inner_ipv6___start_off
metadata parser_ohi_t ohi;

header ethernet_t ethernet;
header vlan_tag_t vlan;
@pragma no_ohi ingress
header ipv4_t ipv4;
@pragma no_ohi ingress
header ipv6_t ipv6;
@pragma no_ohi ingress
header ipv6_t inner_ipv6;
@pragma no_ohi xgress
header udp_t udp;
@pragma no_ohi xgress
header roce_bth_t roce_bth;
@pragma no_ohi xgress
header roce_deth_t roce_deth;
@pragma no_ohi ingress
header roce_deth_immdt_t roce_deth_immdt;
@pragma no_ohi ingress
header vxlan_t vxlan;

header ethernet_t inner_ethernet;
@pragma no_ohi ingress
header ipv4_t inner_ipv4;
header udp_t inner_udp;
@pragma no_ohi ingress
header roce_bth_t inner_roce_bth;
@pragma no_ohi ingress
header roce_deth_t inner_roce_deth;
@pragma no_ohi ingress
header roce_deth_immdt_t inner_roce_deth_immdt;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t capri_deparser_len;

#define ETHERTYPE_VLAN         0x8100
#define ETHERTYPE_IPV4         0x0800
#define ETHERTYPE_IPV6         0x86dd

parser start {
    extract(capri_intrinsic);
    return select(capri_intrinsic.csum_err) {
        0 : parse_ethernet;
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
        // tunneled roce
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
        default : ingress;
    }
}


parser start_ipv4_bth_deth {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth_deth;
}
parser start_vlan_ipv4_bth_deth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth_deth;
}
parser start_ipv6_bth_deth {
    extract(ethernet);
    extract(ipv6);
    return parse_bth_deth;
}
parser start_vlan_ipv6_bth_deth {
    extract(ethernet);
    extract(vlan);
    extract(ipv6);
    return parse_bth_deth;
}
parser start_ipv4_bth_deth_immdt {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv4_bth_deth_immdt {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth_deth_immdt;
}
parser start_ipv6_bth_deth_immdt {
    extract(ethernet);
    extract(ipv6);
    return parse_bth_deth_immdt;
}
parser start_vlan_ipv6_bth_deth_immdt {
    extract(ethernet);
    extract(vlan);
    extract(ipv6);
    return parse_bth_deth_immdt;
}
parser start_ipv4_bth {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth;
}
parser start_vlan_ipv4_bth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_bth;
}
parser start_ipv6_bth {
    extract(ethernet);
    extract(ipv6);
    return parse_bth;
}
parser start_vlan_ipv6_bth {
    extract(ethernet);
    extract(vlan);
    extract(ipv6);
    return parse_bth;
}

parser parse_bth_deth {
    extract(udp);
    extract(roce_bth);
    extract(roce_deth);
    set_metadata(ohi.l4_len, udp.len + 0);
    return ingress;
}
parser parse_bth_deth_immdt {
    extract(udp);
    extract(roce_bth);
    extract(roce_deth_immdt);
    set_metadata(ohi.l4_len, udp.len + 0);
    return ingress;
}
parser parse_bth {
    extract(udp);
    extract(roce_bth);
    set_metadata(ohi.l4_len, udp.len + 0);
    return ingress;
}

// tunneled roce
parser parse_inner_bth_deth {
    extract(inner_udp);
    extract(inner_roce_bth);
    extract(inner_roce_deth);
    set_metadata(ohi.inner_l4_len, inner_udp.len + 0);
    return ingress;
}
parser parse_inner_bth_deth_immdt {
    extract(inner_udp);
    extract(inner_roce_bth);
    extract(inner_roce_deth_immdt);
    set_metadata(ohi.inner_l4_len, inner_udp.len + 0);
    return ingress;
}
parser parse_inner_bth {
    extract(inner_udp);
    extract(inner_roce_bth);
    set_metadata(ohi.inner_l4_len, inner_udp.len + 0);
    return ingress;
}

parser start_outer_vxlan_ipv4_bth_deth {
    extract (ethernet);
    extract (ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth_deth;
}
parser start_outer_vxlan_ipv6_bth_deth {
    extract(ethernet);
    extract(ipv4);
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv6_bth_deth;
}
parser start_outer_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv4_bth_deth_immdt {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth_deth_immdt;
}
parser start_outer_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vlan_vxlan_ipv6_bth_deth_immdt {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv6_bth_deth_immdt;
}
parser start_outer_vxlan_ipv4_bth {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vlan_vxlan_ipv4_bth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv4_bth;
}
parser start_outer_vxlan_ipv6_bth {
    extract(ethernet);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv6_bth;
}
parser start_outer_vlan_vxlan_ipv6_bth {
    extract(ethernet);
    extract(vlan);
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return parse_vxlan_ipv6_bth;
}

parser parse_vxlan_ipv4_bth_deth {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv4_bth_deth_split;
}
parser parse_vxlan_ipv4_bth_deth_split {
    extract(inner_ipv4);
    set_metadata(ohi.inner_ipv4___hdr_len, (inner_ipv4.ihl << 2));
    return parse_inner_bth_deth;
}
parser parse_vxlan_ipv6_bth_deth {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv6_bth_deth_split;
}
parser parse_vxlan_ipv6_bth_deth_split {
    extract(inner_ipv6);
    return parse_inner_bth_deth;
}
parser parse_vxlan_ipv4_bth_deth_immdt {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv4_bth_deth_immdt_split;
}

parser parse_vxlan_ipv4_bth_deth_immdt_split {
    extract(inner_ipv4);
    set_metadata(ohi.inner_ipv4___hdr_len, (inner_ipv4.ihl << 2));
    return parse_inner_bth_deth_immdt;
}
parser parse_vxlan_ipv6_bth_deth_immdt {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv6_bth_deth_immdt_split;
}
parser parse_vxlan_ipv6_bth_deth_immdt_split {
    extract(inner_ipv6);
    return parse_inner_bth_deth_immdt;
}
parser parse_vxlan_ipv4_bth {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv4_bth_split;
}
parser parse_vxlan_ipv4_bth_split {
    extract(inner_ipv4);
    set_metadata(ohi.inner_ipv4___hdr_len, (inner_ipv4.ihl << 2));
    return parse_inner_bth;
}
parser parse_vxlan_ipv6_bth {
    extract(udp);
    extract(vxlan);
    extract(inner_ethernet);
    set_metadata(ohi.l4_len, udp.len + 0);
    return parse_vxlan_ipv6_bth_split;
}
parser parse_vxlan_ipv6_bth_split {
    extract(inner_ipv6);
    return parse_inner_bth;
}


// Non preparsed roce
parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4: parse_ipv4;
        ETHERTYPE_IPV6: parse_ipv6;
        default : ingress;
    }
}

#define IP_PROTOCOLS_UDP               17
parser parse_ipv4 {
    extract(ipv4);
    set_metadata(ohi.ipv4___hdr_len, (ipv4.ihl << 2));
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_ipv6 {
    extract(ipv6);
    return select(latest.nextHdr) {
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_udp {
    extract(udp);
    set_metadata(ohi.l4_len, udp.len + 0);
    return ingress;
}

action nop() {
}
table abc {
    reads {
        capri_p4_intrinsic.frame_size:exact;
    }
    actions {
        nop;
    }
}


// checksum
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

@pragma checksum hdr_len_expr ohi.ipv4___hdr_len + 0
@pragma checksum verify_len ohi.l4_len
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

@pragma checksum hdr_len_expr ohi.inner_ipv4___hdr_len + 0
@pragma checksum verify_len ohi.inner_l4_len
field_list_calculation inner_ipv4_checksum {
    input {
        inner_ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field inner_ipv4.hdrChecksum  {
    verify inner_ipv4_checksum;
    update inner_ipv4_checksum;
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
@pragma checksum verify_len  ohi.inner_l4_len
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
    inner_roce_bth.reserved1;
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

//For icrc, specify invariant crc fields.
field_list inner_ipv6_icrc_list {
    inner_ipv6.trafficClass;
    inner_ipv6.flowLabel;
    inner_ipv6.hopLimit;
    inner_udp.checksum;
    inner_roce_bth.reserved1;
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

    verify ipv4_roce_icrc if (valid(roce_bth));
    update ipv4_roce_icrc if (valid(roce_bth));
    verify ipv6_roce_icrc if (valid(roce_bth));
    update ipv6_roce_icrc if (valid(roce_bth));

    verify inner_ipv4_roce_icrc if (valid(inner_roce_bth));
    update inner_ipv4_roce_icrc if (valid(inner_roce_bth));
    verify inner_ipv6_roce_icrc if (valid(inner_roce_bth));
    update inner_ipv6_roce_icrc if (valid(inner_roce_bth));

}

control ingress {
    apply(abc);
}

control egress {
}

