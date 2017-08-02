// ipv4 option parsing

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

// End of Option List
#define IPV4_OPTION_EOL_VALUE 0x00
header_type ipv4_option_EOL_t {
    fields {
        eol_value : 8;
    }
}

// No operation
#define IPV4_OPTION_NOP_VALUE 0x01
header_type ipv4_option_NOP_t {
    fields {
        nop_value : 8;
    }
}

#define IPV4_OPTION_SECURITY_VALUE 0x82
header_type ipv4_option_security_t {
    fields {
        security_value : 8;
        len : 8;
        security : 72;
    }
}

#define IPV4_OPTION_TIMESTAMP_VALUE 0x44
header_type ipv4_option_timestamp_t {
    fields {
        timestamp_value : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

#define IPV4_OPTION_SECURITY_VALUE 0x83
header_type ipv4_option_lsr_t {
    fields {
        lsr_value : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

#define IPV4_OPTION_SECURITY_VALUE 0x89
header_type ipv4_option_ssr_t {
    fields {
        ssr_value : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

#define IPV4_OPTION_SECURITY_VALUE 0x87
header_type ipv4_option_rr_t {
    fields {
        rr_value : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
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

header_type l3_metadata_t {
    fields {
        parse_ipv4_counter   : 8;
    }
}

header ethernet_t ethernet;
header fabric_header_t fabric_header;
header vlan_tag_t vlan_tag[2];
@pragma pa_header_union xgress ipv6
header ipv4_t ipv4;
header ipv6_t ipv6;
header udp_t udp;
header tcp_t tcp;

// IPV4 Options
header ipv4_option_EOL_t ipv4_option_EOL;
header ipv4_option_NOP_t ipv4_option_NOP;
header ipv4_option_security_t ipv4_option_security;
header ipv4_option_timestamp_t ipv4_option_timestamp;
header ipv4_option_lsr_t ipv4_option_lsr;
header ipv4_option_ssr_t ipv4_option_ssr;
header ipv4_option_rr_t ipv4_option_rr;

@pragma pa_parser_local
metadata l3_metadata_t l3_metadata;

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

parser parse_ipv4_option_EOL {
     extract(ipv4_option_EOL);
     set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter -1);
     return parse_ipv4_options;
}

parser parse_ipv4_option_NOP {
     extract(ipv4_option_NOP);
     set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter -1);
     return parse_ipv4_options;
}

parser parse_ipv4_option_security {
    extract(ipv4_option_security);
    set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter - 11);
    return parse_ipv4_options;
}

parser parse_ipv4_option_timestamp {
    extract(ipv4_option_timestamp);
    set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter - ipv4_option_timestamp.len);
    return parse_ipv4_options;
}
 
parser parse_ipv4_option_lsr{
    extract(ipv4_option_lsr);
    set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter - ipv4_option_lsr.len);
    return parse_ipv4_options;
}
 
parser parse_ipv4_option_ssr{
    extract(ipv4_option_ssr);
    set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter - ipv4_option_ssr.len);
    return parse_ipv4_options;
}
 
parser parse_ipv4_option_rr{
    extract(ipv4_option_rr);
    set_metadata(l3_metadata.parse_ipv4_counter, l3_metadata.parse_ipv4_counter - ipv4_option_rr.len);
    return parse_ipv4_options;
}

@pragma header_ordering ipv4_option_security ipv4_option_timestamp ipv4_option_lsr ipv4_option_ssr ipv4_option_rr ipv4_option_NOP ipv4_option_EOL
parser parse_ipv4_options {
    return select(l3_metadata.parse_ipv4_counter, current(0, 8)) {
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

parser parse_ipv4 {
    extract(ipv4);
    set_metadata(l3_metadata.parse_ipv4_counter, (ipv4.ihl << 2) - 20);
    return select(l3_metadata.parse_ipv4_counter) {
        0x0 : parse_base_ipv4;
        default : parse_ipv4_options;
    }
    
}

parser parse_base_ipv4 {
    return select(ipv4.fragOffset, ipv4.protocol) {
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
