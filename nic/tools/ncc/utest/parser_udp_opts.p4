// Test a small parse graph, no loops, no tlv parsing
// separate egress start state
// some header fields are used in M+A for phv allocation

/* Ether types */
#define ETHERTYPE_VLAN         0x8100
#define ETHERTYPE_QINQ         0x9100
#define ETHERTYPE_MPLS         0x8847
#define ETHERTYPE_IPV4         0x0800
#define ETHERTYPE_IPV6         0x86dd
#define ETHERTYPE_ARP          0x0806
#define ETHERTYPE_RARP         0x8035
#define ETHERTYPE_ETHERNET     0x6558
#define ETHERTYPE_FABRIC       0x910F

#define IP_PROTOCOLS_TCP               6
#define IP_PROTOCOLS_UDP               17

#define UDP_PORT_ROCE_V2               4791

#define PARSE_ETHERTYPE                                    \
        ETHERTYPE_VLAN : parse_vlan;                       \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        default: ingress

#define PARSE_ETHERTYPE_MINUS_VLAN                         \
        ETHERTYPE_IPV4 : parse_ipv4;                       \
        ETHERTYPE_IPV6 : parse_ipv6;                       \
        default: ingress

// Headers
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
        cfi : 1;
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
#if 0
        options : *;
#endif
    }
#if 0
    length : ihl * 4;
    max_length : 60;
#endif
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
header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 4;
        flags : 8;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
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

header_type udp_payload_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : (hdr_len);
    max_length : 65535;
}

header_type fabric_header_t {
    fields {
        bypass : 1;
        reserved : 7;
        ingressPort: 16;
        ingressIfindex : 16;
        ingressBd : 16;
        reasonCode : 16;
    }
}
header_type fabric_payload_header_t {
    fields {
        etherType : 16;
    }
}

header_type tcp_option_eol_t {
    fields {
        optType : 8;
    }
}

header_type tcp_option_nop_t {
    fields {
        optType : 8;
    }
}

header_type tcp_option_mss_t {
    fields {
        optType : 8;
        optLength : 8;
        value : 16;
    }
}

header_type tcp_option_ws_t {
    fields {
        optType : 8;
        optLength : 8;
        value : 8;
    }
}

header_type tcp_option_sack_perm_t {
    fields {
        optType: 8;
        optLength : 8;
        value : 16;
    }
}

header_type tcp_option_timestamp_t {
    fields {
        optType : 8;
        optLength: 8;
        ts : 32;
        prev_echo_ts : 32;
    }
}

header_type tcp_option_one_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
    }
}

header_type tcp_option_two_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
    }
}

header_type tcp_option_three_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
        third_le : 32;
        third_re : 32;
    }
}

header_type tcp_option_four_sack_t {
    fields {
        optType : 8;
        optLength : 8;
        first_le : 32;
        first_re : 32;
        second_le : 32;
        second_re : 32;
        third_le : 32;
        third_re : 32;
        fourth_le : 32;
        fourth_re : 32;
    }
}

header_type parser_metadata_t {
    fields {
        l3_payload_len      : 16;
        l4_len              : 16;
        l4_trailer          : 16;
        options_length      : 8;
    }
}

header_type l4_metadata_t {
    fields {
        l4_opt_len :8;
    }
}

header ethernet_t ethernet;
header fabric_header_t fabric_header;
header vlan_tag_t vlan_tag[2];
@pragma pa_header_union egress ipv6
header ipv4_t ipv4;
header ipv6_t ipv6;
header udp_t udp;
@pragma hdr_len parser_metadata.l4_len
header udp_payload_t udp_payload;
header tcp_t tcp;
@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;
metadata l4_metadata_t l4_metadata;

parser start {
    extract(ethernet);
    return select(latest.etherType) {
        PARSE_ETHERTYPE;
    }
}

parser parse_vlan {
    extract(vlan_tag[0]);
    return select(latest.etherType) {
        PARSE_ETHERTYPE_MINUS_VLAN;
    }
}

parser parse_ipv4 {
    extract(ipv4);
    set_metadata(parser_metadata.l4_trailer, ipv4.totalLen - (ipv4.ihl << 2));
    //set_metadata(parser_metadata.l4_trailer, current(16,16) - (current(4,4) << 2));
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTOCOLS_TCP : parse_tcp;
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_ipv6 {
    extract(ipv6);
    set_metadata(parser_metadata.l4_trailer, ipv6.payloadLen);
    return select(latest.nextHdr) {
        IP_PROTOCOLS_TCP : parse_tcp;
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_udp {
    extract(udp);
    set_metadata(parser_metadata.l4_trailer, parser_metadata.l4_trailer - udp.len);
    set_metadata(parser_metadata.l4_len, udp.len);
    return select(latest.dstPort) {
        UDP_PORT_ROCE_V2: parse_roce_v2;
        default: ingress;
    }
#if 0
    return select (parser_metadata.l4_trailer) {
        0x0000 : ingress;
        //0x8000 mask 0x8000 : ingress_error;
        default : parse_udp_payload;
    }
#endif
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
header roce_bth_t roce_bth;
header roce_deth_t roce_deth;
header roce_deth_immdt_t roce_deth_immdt;
parser parse_roce_v2 {
    extract(roce_bth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-12);
    return select(latest.opCode) {
        0x64 : parse_roce_deth;
        //0x65 : parse_roce_deth_immdt;
        default : parse_trailer;
    }
}

parser parse_roce_deth {
    extract(roce_deth);
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len-8);
    return select(latest.reserved) {
        default : parse_trailer;
        //0x1 mask 0x0 : parse_roce_eth;
    }
}

parser parse_trailer {
    return select(parser_metadata.l4_trailer) {
        0x0000 mask 0xffff: ingress;
        //0x8000 mask 0x8000: ingress_error;
        default : parse_udp_payload;
    }
}

parser parse_udp_payload {
    set_metadata(parser_metadata.l4_len, parser_metadata.l4_len + 0);
    extract(udp_payload);
    return parse_udp_options;
}

parser parse_udp_options {
    // XXX
    return ingress;
}

parser parse_tcp {
    extract(tcp);
    return ingress;
}

// Tables
action nop() {}

table x {
    reads {
        ethernet.dstAddr : exact;
        udp.srcPort : exact;
        tcp.dstPort : exact;
        ipv4.srcAddr : exact;
        ipv6.dstAddr : exact;
        l4_metadata.l4_opt_len : exact;
    }
    actions {
        nop;
    }
}

control ingress {
    apply(x);
}

control egress {
}
