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
        parse_tcp_counter   : 8;
        options_length      : 8;
        protocol            : 8;
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
header tcp_t tcp;
header tcp_option_eol_t tcp_option_eol;
header tcp_option_nop_t tcp_option_nop;
header tcp_option_mss_t tcp_option_mss;
header tcp_option_ws_t tcp_option_ws;
header tcp_option_sack_perm_t tcp_option_sack_perm;
header tcp_option_one_sack_t tcp_option_one_sack;
header tcp_option_two_sack_t tcp_option_two_sack;
header tcp_option_three_sack_t tcp_option_three_sack;
header tcp_option_four_sack_t tcp_option_four_sack;
header tcp_option_timestamp_t tcp_option_timestamp;
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
    set_metadata(parser_metadata.protocol, latest.protocol);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTOCOLS_TCP : parse_tcp;
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_ipv6 {
    extract(ipv6);
    return select(latest.nextHdr) {
        IP_PROTOCOLS_TCP : parse_tcp;
        IP_PROTOCOLS_UDP : parse_udp;
        default: ingress;
    }
}

parser parse_udp {
    extract(udp);
    return ingress;
}

parser parse_tcp {
    extract(tcp);
    set_metadata(parser_metadata.parse_tcp_counter, (tcp.dataOffset << 2) - 20);
    return select(tcp.dataOffset) {
        0x5 : ingress;
        default : parse_tcp_options;
    }
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
        default : ingress;
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

parser parse_tcp_option_mss {
    extract(tcp_option_mss);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - tcp_option_mss.optLength);
    set_metadata(parser_metadata.options_length,
                 parser_metadata.options_length+tcp_option_mss.optLength);
    return parse_tcp_options;
}

parser parse_tcp_option_ws {
    extract(tcp_option_ws);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - tcp_option_ws.optLength);
    set_metadata(parser_metadata.options_length,
                 parser_metadata.options_length + tcp_option_ws.optLength);
    return parse_tcp_options;
}

parser parse_tcp_option_sack_perm {
    extract(tcp_option_sack_perm);
    set_metadata(parser_metadata.parse_tcp_counter,
                 parser_metadata.parse_tcp_counter - 2);
    set_metadata(parser_metadata.options_length, parser_metadata.options_length + 2);
    return parse_tcp_options;
}

parser parse_tcp_timestamp {
    extract(tcp_option_timestamp);
    set_metadata(parser_metadata.parse_tcp_counter, parser_metadata.parse_tcp_counter - tcp_option_timestamp.optLength);
    set_metadata(parser_metadata.options_length, parser_metadata.options_length + tcp_option_timestamp.optLength);
    return parse_tcp_options;
}

@pragma header_ordering tcp_option_mss tcp_option_ws tcp_option_sack_perm tcp_option_one_sack tcp_option_two_sack tcp_option_three_sack tcp_option_four_sack tcp_option_timestamp tcp_option_nop tcp_option_eol
parser parse_tcp_options {
    return select(parser_metadata.parse_tcp_counter, current(0, 8)) {
        0x0000 mask 0xff00 : parse_tcp_opts_done;
        0x0000 mask 0x00ff: parse_tcp_option_EOL;
        0x0001 mask 0x00ff: parse_tcp_option_NOP;
        0x0002 mask 0x00ff: parse_tcp_option_mss;
        0x0003 mask 0x00ff: parse_tcp_option_ws;
        0x0004 mask 0x00ff: parse_tcp_option_sack_perm;
        0x0005 mask 0x00ff: parse_tcp_option_sack;
        0x0008 mask 0x00ff: parse_tcp_timestamp;
        default: ingress;
    }
}

parser parse_tcp_opts_done {
    // capture the options len
    set_metadata(l4_metadata.l4_opt_len, parser_metadata.options_length);
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
