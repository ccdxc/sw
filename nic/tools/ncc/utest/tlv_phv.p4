// Use header fields in control flow to force extract instructions
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
        options : *;
    }
    length : ihl << 2;
    max_length : 32;    // XXX keep it small to avoid assert in ncc
}

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
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

header_type parser_metadata_t {
    fields {
        parse_tcp_counter   : 8;
        options_length      : 8;
        protocol            : 8;
    }
}
header_type tcp_NOP_t {
    fields {
        optType : 8;
    }
}

header_type tcp_option_X_t {
    fields {
        opt_type : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 32;
}

header_type tcp_option_sack_t {
    fields {
        opt_type : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 32;
}


header ethernet_t ethernet;
header ipv4_t ipv4;
header tcp_t tcp;
header tcp_NOP_t tcp_opt_NOP;
header tcp_option_X_t tcp_option_X;
header tcp_option_sack_t tcp_option_sack;

@pragma pa_parser_local
metadata parser_metadata_t parser_metadata;

#define ETHERTYPE_VLAN         0x8100
#define ETHERTYPE_IPV4         0x0800

parser start {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4: parse_ipv4;
        default : ingress;
    }
}

#define IP_PROTOCOLS_TCP               6
#define IP_PROTOCOLS_UDP               17

parser parse_ipv4 {
    // extraction of ipv4 header needs to steps
    // 1. extract 20B to phv
    // 2. create OHI entry (offset = ipv4.options, len = (ipv4.ihl << 2) - 20)
    extract(ipv4);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTOCOLS_TCP : parse_tcp;
        default: ingress;
    }
}

parser parse_tcp {
    extract(tcp);
    set_metadata(parser_metadata.parse_tcp_counter, (tcp.dataOffset << 2) - 20);
    return select(parser_metadata.parse_tcp_counter) {
        0x0 : ingress;
        default : parse_tcp_options;
    }
}

@pragma header_ordering tcp_option_X tcp_option_sack tcp_opt_NOP
parser parse_tcp_options {
    return select(parser_metadata.parse_tcp_counter, current(0, 8)) {
        0x0000 mask 0xff00: parse_tcp_opts_done;
        0x0001 mask 0x00ff: parse_tcp_option_NOP;
        0x0006 mask 0x00ff: parse_tcp_option_X;
        0x0005 mask 0x00ff: parse_tcp_option_sack;
        default: ingress;
    }
}

parser parse_tcp_option_NOP {
    extract(tcp_opt_NOP);
    return parse_tcp_options;
}

parser parse_tcp_option_X {
    // extraction of optionX
    extract(tcp_option_X);
    set_metadata(parser_metadata.parse_tcp_counter, \
        parser_metadata.parse_tcp_counter - tcp_option_X.len);
    // Need pkt_data as operand when updating the tlv register
    // set_metadata(parser_metadata.options_length, parser_metadata.options_length + tcp_option_X.len);
    return parse_tcp_options;
}

parser parse_tcp_option_sack {
    extract(tcp_option_sack);
    set_metadata(parser_metadata.parse_tcp_counter, \
            parser_metadata.parse_tcp_counter - tcp_option_sack.len);
    // Need pkt_data as operand when updating the tlv register
    // options_length calculation requires an additional mux_inst, hw only has 3
    // usage - 1. offset inst, 2. extraction, 3. counter update
    //set_metadata(parser_metadata.options_length, parser_metadata.options_length + tcp_option_sack.len);
    return parse_tcp_options;
}

parser parse_tcp_opts_done {
    return ingress;
}

// Tables
action nop() {}

table x {
    reads {
        ethernet.dstAddr : exact;
        ethernet.srcAddr : exact;
        ethernet.etherType : exact;
        ipv4.srcAddr : exact;
        ipv4.dstAddr : exact;
        tcp.srcPort : exact;
        tcp.dstPort : exact;
        tcp_option_sack.opt_type : exact;
        tcp_option_X.opt_type : exact;
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
// QQ : deparser may need to allow to drop options in OHI - this can be used when dropping unknown options ??
