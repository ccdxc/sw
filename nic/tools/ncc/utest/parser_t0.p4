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


header ethernet_t ethernet;
header fabric_header_t fabric_header;
header vlan_tag_t vlan_tag[2];
@pragma pa_field_union ingress ipv4.dstAddr ipv6.dstAddr
@pragma pa_field_union ingress ipv4.srcAddr ipv6.srcAddr
@pragma pa_field_union ingress ipv6.srcAddr meta.lkp_srcAddr
header ipv4_t ipv4;
header ipv6_t ipv6;
@pragma pa_header_union xgress tcp
header udp_t udp;
header tcp_t tcp;

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
    // just playing with mask for testing
    return select(latest.fragOffset, latest.protocol, latest.ihl) {
        0x60 mask 0x1FFFFF0 : parse_tcp;
        0x115 mask 0x1FFFFFF : parse_udp;
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
