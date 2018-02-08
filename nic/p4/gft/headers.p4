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

header_type icmp_t {
    fields {
        icmp_type : 8;
        icmp_code : 8;
        hdrChecksum : 16;
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

header_type udp_opt_ocs_t {
    fields {
        kind :      8;
        chksum :    8;
    }
}

header_type udp_opt_timestamp_t {
    fields {
        kind : 8;
        len  : 8;
        ts_value : 32;
        ts_echo : 32;
    }
}

header_type udp_opt_mss_t {
    fields {
        kind :  8;
        len :  8;
        mss :  16;
    }
}

header_type udp_opt_nop_t {
    fields {
        kind : 8;
    }
}

header_type udp_opt_eol_t {
    fields {
        kind : 8;
    }
}

header_type udp_opt_unknown_t {
    fields {
        kind : 8;
        len : 8;
        data : *;
    }
    length : len;
    max_length : 40;
}

header_type gre_t {
    fields {
        C : 1;
        R : 1;
        K : 1;
        S : 1;
        s : 1;
        recurse : 3;
        flags : 5;
        ver : 3;
        proto : 16;
    }
}

header_type erspan_header_t3_t {
    fields {
        version : 4;
        vlan : 12;
        priority : 6;
        span_id : 10;
        timestamp : 32;
        sgt       : 16;
        ft_d_other: 16;
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

header_type icrc_t {
    fields {
        value : 32;
    }
}
