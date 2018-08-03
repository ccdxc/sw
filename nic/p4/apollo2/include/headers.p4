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

header_type mpls_t {
    fields {
        label : 20;
        exp : 3;
        bos : 1;
        ttl : 8;
    }
}

header_type service_header_t {
    fields {
        local_ip_mapping_ohash : 32;
        flow_ohash : 32;

        epoch : 32;

        pad1 : 5;
        local_ip_mapping_done : 1;
        flow_done : 1;
    }
}

header_type egress_service_header_t {
    fields {
        remote_vnic_mapping_ohash : 32;

        remote_vnic_mapping_done : 1;
    }
}

header_type p4_to_rxdma_header_t {
    fields {
        p4plus_app_id       : 4;
        slacl_bypass        : 1;
        to_arm              : 1;
        slacl_base_addr     : 34;

        direction           : 1;    // ???
        udp_flow_hash_lkp   : 1;    // Must never be set
        udp_queue_bypass    : 1;    // False = subject packet to udp flow queueing, could be either flow miss or flow_state == queueing
        udp_queue_drain     : 1;
        udp_queue_delete    : 1;
        udp_flow_hit        : 1;    // flow hit, flow_state == queuing 
        slacl_addr1         : 34;

        pad0                : 6;
        slacl_addr2         : 34;

        slacl_ip_15_00      : 16;
        ip_proto            : 8;
        slacl_ip_31_16      : 16;
        l4_sport            : 16;
        l4_dport            : 16;

        ingress_vnic        : 12;
        egress_vnic         : 12;


        pad1                : 6;
        udp_q_counter       : 10;   // packets received while flow entry is in 'queuing' state, 0 indicates flow miss

        udp_oflow_index     : 32;
        udp_flow_qid        : 8;    // qid - useful when drain is set

        pad2                : 4;
        udp_flow_key        : 300;
    }
}

header_type p4_to_arm_header_t {
    fields {
        packet_type         : 2;
        packet_len          : 14;
    }
}

header_type p4_to_txdma_header_t {
    fields {
        p4plus_app_id   : 4;
        lpm_bypass      : 1;
        pad0            : 1;
        lpm_addr        : 34;
        pad1            : 6;
        lpm_base_addr   : 34;
        lpm_dst         : 128;
        pad2            : 6;
        vcn_id          : 10;
    }
}

header_type txdma_to_p4i_header_t {
    fields {
        p4plus_app_id   : 4;
        pad0            : 4;
        udp_first_pkt   : 1;
        bypass_rxdma    : 1;
    }
}

header_type txdma_to_p4e_header_t {
    fields {
        p4plus_app_id   : 4;
        pad0            : 4;
        pad1            : 6;
        nexthop_index   : 10;
        pad2            : 6;
        vcn_id          : 10;
    }
}

header_type arm_to_txdma_header_t {
    fields {
        udp_first_pkt : 1;
    }
}
