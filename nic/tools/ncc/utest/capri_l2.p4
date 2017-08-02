// Test a small parse graph, no loops, no tlv parsing
// used capri intrinsic headers

#include "includes/capri_intrinsic.p4"

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
// Capri specific headers
header cap_phv_intr_global_t capri_intrinsic;                                                       
metadata cap_phv_intr_p4_t capri_p4_intrinsic;                                                      
header ethernet_t ethernet;

parser start {
    extract(capri_intrinsic);
    return select(current(0, 4)) {
        default : parse_ethernet;
    }
}

parser parse_ethernet {
    extract(ethernet);
    return ingress;
}

// ================ control flow ===============
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

control ingress {
    apply(abc);
}

control egress {
}
