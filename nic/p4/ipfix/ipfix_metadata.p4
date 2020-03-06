header_type ipfix_metadata_t {
    fields {
        flow_index : 32;
        session_index : 32;
        export_id : 8;
        flow_type : 4;
        export_en : 4;
        flow_role : 1;
        scan_complete : 1;
        pad : 12;
        qstate_addr : 34;
    }
}

header_type ipfix_flow_hash_metadata_t {
    fields {
        next_record_offset : 16;
        flow_hash_table_type : 8;
        flow_hash_index_next : 32;
        exported_stats_addr : 34;
    }
}

header_type ipfix_exported_stats_metadata_t {
    fields {
        pkts : 64;
        byts : 64;
    }
}

header_type ipfix_s5_metadata_t {
    fields {
        doorbell1_data : 64;
        doorbell2_data : 64;
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

header_type ipfix_record_ipv4_t {
    fields {
        set_id : 16;
        len : 16;
        vrf : 32;
        ip_sa : 32;
        ip_da : 32;
        proto : 8;
        sport : 16;
        dport : 16;
    }
}

header_type ipfix_record_ipv6_t {
    fields {
        set_id : 16;
        len : 16;
        vrf : 32;
        ip_sa : 128;
        ip_da : 128;
        proto : 8;
        sport : 16;
        dport : 16;
        pad   : 8;
    }
}

header_type ipfix_record_nonip_t {
    fields {
        set_id : 16;
        len : 16;
        vrf : 32;
        mac_sa : 48;
        mac_da : 48;
        ether_type : 16;
    }
}

header_type ipfix_record_ip_t {
    fields {
        flow_state_index : 24;
        role : 8;
        ttl : 8;
        icmp_type_code : 16;
        tcp_seq_num : 32;
        tcp_ack_num : 32;
        tcp_win_sz : 16;
        tcp_win_scale : 8;
        tcp_state : 8;
        tcp_win_mss : 16;
        tcp_exceptions : 32;
        flit_pad : 16;
        tcp_rtt : 32;
    }
}

header_type ipfix_record_common_t {
    fields {
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
        flit_pad : 32;
        start_timestamp : 64;
        last_seen_timestamp : 64;
        drop_vector : 64;
    }
}

header_type ipfix_pad_t {
    fields {
        pad : 32;
    }
}

header_type ipfix_scratch_metadata_t {
    fields {
        scan_complete : 1;
        flow_role : 1;
        flow_type : 4;
        export_id : 4;
        export_en : 4;
        flow_index : 32;
        session_index : 32;
        flow_hash_table_type : 8;
        flow_hash_index_next : 32;
        qstate_addr : 34;
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

        pktaddr : 64;
        pktsize : 16;
        seq_no : 32;
        domain_id : 32;
        ipfix_hdr_offset : 16;
        next_record_offset : 16;

        flow_hash_table_type : 8;
        flow_hash_index_next : 32;
        flow_hash_index_max : 32;
        flow_hash_overflow_index_max : 32;

        export_time : 32;
    }
}
