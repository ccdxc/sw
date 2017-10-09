header_type ipfix_metadata_t {
    fields {
        flow_index : 32;
        session_index : 32;
        flow_role : 1;
        qstate_addr : 34;
    }
}

header_type ipfix_t0_metadata_t {
    fields {
        eindex : 32;
    }
}

header_type ipfix_ipv4_udp_header_t {
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
        srcPort : 16;
        dstPort : 16;
        len : 16;
        checksum : 16;
    }
}

header_type ipfix_ipv6_udp_header_t {
    fields {
        version : 4;
        trafficClass : 8;
        flowLabel : 20;
        payloadLen : 16;
        nextHdr : 8;
        hopLimit : 8;
        srcAddr : 128;
        dstAddr : 128;
        srcPort : 16;
        dstPort : 16;
        len : 16;
        checksum : 16;
    }
}

header_type ipfix_record_header_t {
    fields {
        version : 16;
        len : 16;
        export_time : 32;
        seq_num : 32;
        domain_id : 32;
    }
}

header_type ipfix_record_common_t {
    fields {
        set_id : 16;
        len : 16;
        vrf : 32;
        flow_id : 32;
        egress_interface : 16;
        egress_interface_type : 16;
        permit_packets : 64;
        permit_bytes : 64;
        delta_permit_packets : 32;
        delta_permit_bytes : 32;
        drop_packets : 64;
        drop_bytes : 64;
        delta_drop_packets : 32;
        delta_drop_bytes : 32;
        start_timestamp : 64;
        last_seen_timestamp : 64;
        drop_vector : 64;
    }
}

header_type ipfix_record_ipv4_t {
    fields {
        ip_sa : 32;
        ip_da : 32;
    }
}

header_type ipfix_record_ipv6_t {
    fields {
        ip_sa : 128;
        ip_da : 128;
    }
}

header_type ipfix_record_ip_t {
    fields {
        flow_state_index : 24;
        proto : 8;
        role : 8;
        sport : 16;
        dport : 16;
        icmp_type_code : 16;
        tcp_seq_num : 32;
        tcp_ack_num : 32;
        tcp_win_sz : 16;
        tcp_win_scale : 8;
        tcp_state : 8;
        tcp_win_mss : 16;
        tcp_exceptions : 32;
        tcp_rtt : 32;
        ttl : 8;
    }
}

header_type ipfix_record_nonip_t {
    fields {
        mac_sa : 48;
        mac_da : 48;
        ether_type : 16;
    }
}

header_type ipfix_pad_t {
    fields {
        pad : 104;
    }
}

header_type ipfix_scratch_metadata_t {
    fields {
        flow_role : 1;
        flow_index : 32;
        session_index : 32;
        qstate_addr : 32;
        counter64 : 64;
    }
}

header_type ipfix_qstate_metadata_t {
    fields {
        pc : 8;
        rsvd : 8;
        cos_a : 4;
        cos_b : 4;
        cos_sel : 8;
        eval_last : 8;
        host_rings : 4;
        total_rings : 4;
        pid : 16;
        pindex : 16;
        cindex : 16;
        eindex : 32;
        pktaddr : 64;
        pktsize : 16;
    }
}
