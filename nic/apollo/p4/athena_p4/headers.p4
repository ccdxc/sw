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

header_type arp_rarp_t {
    fields {
        hwType : 16;
        protoType : 16;
        hwAddrLen : 8;
        protoAddrLen : 8;
        opcode : 16;
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
        typeCode : 16;
        hdrChecksum : 16;
    }
}

header_type icmp_echo_t {
    fields {
        identifier : 16;
        seqNum : 16;
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

header_type tcp_options_blob_t {
    fields {
        hdr_len : 8;
        data : *;
    }
    length : hdr_len;
    max_length : 40;
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

header_type tcp_option_unknown_t {
    fields {
        optType : 8;
        optLength : 8;
        data : *;
    }
    length : optLength;
    max_length : 40;
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
        cos : 3;
        bso : 2;
        truncated : 1;
        span_id : 10;
        timestamp : 32;
        sgt : 16;
        pdu : 1;
        frame_type : 5;
        hw_id : 6;
        direction : 1;
        granularity : 2;
        options : 1;
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

/*
 * The MPLS label has been split into 2 parts
 * to be able to setup metadata directly in the parser
 */
header_type mpls_t {
    fields {
        label_b20_b4            : 16;
        label_b3_b0             : 4;
        exp                     : 3;
        bos                     : 1;
        ttl                     : 8;
    }
}

header_type ingress_recirc_header_t {
    fields {
        flow_ohash              : 32;
        dnat_ohash              : 32;

        pad1                    : 5;
        direction               : 1;
        flow_done               : 1;
        dnat_done               : 1;
    }
}

header_type p4i_to_p4e_header_t {
    fields {
        index                   : 22;
        index_type              : 1;
        pad0                    : 1;
        packet_len              : 16;
        flow_hash               : 32;
        flow_miss               : 1;
        direction               : 1;
        update_checksum         : 1;
        pad1                    : 5;
    }
}

header_type p4_to_arm_header_t {
    fields {
        packet_len              : 16;
        flags                   : 16;

        local_vnic_tag          : 16;
        flow_hash               : 32;

        l2_1_offset             : 8;
        l3_1_offset             : 8;
        l4_1_offset             : 8;
        l2_2_offset             : 8;
        l3_2_offset             : 8;
        l4_2_offset             : 8;
        payload_offset          : 8;
    }
}

header_type p4_to_rxdma_header_t {
    fields {
        p4plus_app_id           : 4;
        table0_valid            : 1;
        table1_valid            : 1;
        table2_valid            : 1;
        table3_valid            : 1;
        udp_flow_lkp_continue   : 1;
        sacl_bypass             : 1;
        sacl_result             : 2;    // (sf,sl) encoded value
        udp_flow_lkp_result     : 2;
        sacl_base_addr          : 34;
        local_vnic_tag          : 16;

        udp_flow_hash_lkp       : 1;    // Must never be set
        udp_queue_enable        : 1;    // True = subject packet to udp flow
                                        // queueing, could be either flow miss
                                        // or flow_state == queueing

        pad                     : 4;
        direction               : 1;
        udp_queue_drain         : 1;

        udp_oflow_index         : 24;
        udp_flow_qid            : 8;    // qid - useful when drain is set

        flow_src                : 128;
        flow_sport              : 16;
        flow_proto              : 8;
        flow_dst                : 128;
        flow_dport              : 16;
        flow_ktype              : 4;

        udp_queue_delete        : 1;
        udp_flow_hit            : 1;    // flow hit, flow_state == queuing
        udp_q_counter           : 10;   // packets received while flow entry
                                        // is in 'queuing' state, 0 indicates
                                        // flow miss
    }
}
