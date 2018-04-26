
header_type parser_ohi_t {
    fields {
        ipv4___start_off            : 16;
        ipv6___start_off            : 16;
        udp___start_off             : 16;
        tcp___start_off             : 16;
        l4_len                      : 16;
    }
}


@pragma parser_write_only
metadata parser_ohi_t ohi;


header_type deparser_len_t {
    fields {
        trunc_pkt_len   : 16;
        l4_payload_len  : 16;
    }
}


header_type parser_metadata_t {
    fields {
        ipv4hdr_len                     : 16;
    }
}

@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;


header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
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

header_type meta_t {
    fields {
        tcpLength : 16;
    } 
}

header_type parser_meta_t {
    fields {
        ipv4HdrLen : 16;
        totalLen : 16;
    }
}

header ethernet_t ethernet;
header ipv4_t ipv4;
header ipv6_t ipv6;
header tcp_t tcp;
header udp_t udp;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata deparser_len_t capri_deparser_len_hdr;
@pragma pa_parser_local
metadata meta_t meta;
@pragma pa_parser_local
metadata parser_meta_t parsermeta;

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

@pragma checksum hdr_len_expr parser_metadata.ipv4hdr_len + 20
@pragma checksum verify_len ohi.l4_len
field_list_calculation ipv4_checksum {
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.hdrChecksum {
    verify ipv4_checksum if (valid(ipv4));
    update ipv4_checksum if (valid(ipv4));
}


field_list udp_checksum_list {
    ipv4.srcAddr;
    ipv4.dstAddr;
    8'0;
    ipv4.protocol;
    udp.srcPort;
    udp.dstPort;
    udp.len;
    payload;
}

@pragma checksum update_len capri_deparser_len_hdr.l4_payload_len
@pragma checksum verify_len ohi.l4_len
field_list_calculation ipv4_udp_checksum {
    input {
        udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}


field_list v6_udp_checksum_list {
    ipv6.srcAddr;
    ipv6.dstAddr;
    8'0;
    ipv6.nextHdr;
    udp.srcPort;
    udp.dstPort;
    payload;
}

@pragma checksum update_len capri_deparser_len_hdr.l4_payload_len
@pragma checksum verify_len ohi.l4_len
field_list_calculation ipv6_udp_checksum {
    input {
        v6_udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}



calculated_field udp.checksum {
    verify ipv4_udp_checksum if (valid(udp));
    verify ipv6_udp_checksum if (valid(udp));
    update ipv4_udp_checksum if (valid(udp));
    update ipv6_udp_checksum if (valid(udp));
}


field_list tcp_checksum_list {
    ipv4.srcAddr;
    ipv4.dstAddr;
    8'0;
    ipv4.protocol;
    meta.tcpLength;
    tcp.srcPort;
    tcp.dstPort;
    tcp.seqNo;
    tcp.ackNo;
    tcp.dataOffset;
    tcp.res;
    tcp.flags;
    tcp.window;
    tcp.checksum;
    tcp.urgentPtr;
    payload;
}

@pragma checksum update_len capri_deparser_len_hdr.l4_payload_len
@pragma checksum verify_len ohi.l4_len
field_list_calculation ipv4_tcp_checksum {
    input {
        tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

field_list v6_tcp_checksum_list {
    ipv6.srcAddr;
    ipv6.dstAddr;
    8'0;
    ipv6.nextHdr;
    meta.tcpLength;
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

@pragma checksum update_len capri_deparser_len_hdr.l4_payload_len
@pragma checksum verify_len ohi.l4_len
field_list_calculation ipv6_tcp_checksum {
    input {
        v6_tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}


calculated_field tcp.checksum {
    verify ipv4_tcp_checksum if (valid(tcp));
    verify ipv6_tcp_checksum if (valid(tcp));
    update ipv4_tcp_checksum if (valid(tcp));
    update ipv6_tcp_checksum if (valid(tcp));
}


#define ETHERTYPE_VLAN         0x8100
#define ETHERTYPE_IPV4         0x0800
#define ETHERTYPE_IPV6         0x86dd

parser start {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4: parse_ipv4;
        ETHERTYPE_IPV6: parse_ipv6;
        default : ingress;
    }
}

#define IP_PROTOCOLS_TCP               6
#define IP_PROTOCOLS_UDP               17

parser parse_ipv4 {
    extract(ipv4);
    set_metadata(parser_metadata.ipv4hdr_len, (ipv4.ihl << 2) - 20);
    return select(latest.protocol) {
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

parser parse_tcp {
    extract(tcp);
    return ingress;
}

parser parse_udp {
    extract(udp);
    return ingress;
}

control ingress {
}

control egress {
}
