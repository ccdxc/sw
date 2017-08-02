// use large header extractions to force state splitting
// use set meta from each header

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

header_type hw_header_t {
    fields {
        iport : 20;
        pad_0 : 12;
        x : 128;
        y : 128;
        z : 128;
        t : 32;
    }
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

header_type vlan_t {
    fields {
        prio : 4;
        tag : 12;
        etype : 16;
    }
}

header hw_header_t hw_hdr;
header ethernet_t ethernet;
header ipv4_t ipv4;
header vlan_t vlan;

header_type meta_t {
    fields {
        pad_0 : 12;
        da : 48;
    }
}
metadata meta_t meta;

#define ETHERTYPE_VLAN         0x8100
#define ETHERTYPE_IPV4         0x0800

parser start {
    return hw_start;
}

parser hw_start {
    extract(hw_hdr);
    extract(ethernet);
    set_metadata(meta.pad_0, hw_hdr.pad_0);
    set_metadata(meta.da, ethernet.dstAddr);
    return select(hw_hdr.iport, ethernet.etherType) {
        ETHERTYPE_IPV4: parse_ipv4;
        ETHERTYPE_VLAN: parse_vlan;
        default : ingress;
    }
}

#define IP_PROTOCOLS_TCP               6
#define IP_PROTOCOLS_UDP               17

parser parse_vlan {
    extract(vlan);
    return ingress;
}

parser parse_ipv4 {
    extract(ipv4);
    return select(latest.fragOffset, latest.protocol) {
        IP_PROTOCOLS_TCP : parse_tcp;
        default: ingress;
    }
}

parser parse_tcp {
    return ingress;
}

// Tables
action nop() {}

table x {
    reads {
        ethernet.dstAddr : exact;
        meta.da : exact;
        meta.pad_0 : exact;
        ethernet.etherType : exact;
        ipv4.srcAddr : exact;
        ipv4.dstAddr : exact;
        hw_hdr.iport : exact;
        hw_hdr.z : exact;
        hw_hdr.t : exact;
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
