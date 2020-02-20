header_type p4_to_p4plus_ipsec_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        flow_id             : 24;
        ipsec_payload_start : 16;
        ipsec_payload_end   : 16;
        l4_protocol         : 8;
        ip_hdr_size         : 8;
        seq_no              : 32;
        spi                 : 32;
    }
}

header_type p4_to_p4plus_p4pt_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        p4pt_idx            : 24;
        payload_len         : 16;
        pad                 : 6;
        flow_role           : 1;
        flow_dir            : 1;
    }
}

header_type p4_to_p4plus_roce_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        //Parsed UDP options valid flags
        roce_opt_ts_valid   : 1;
        roce_opt_mss_valid  : 1;
        rdma_hdr_len        : 6;  // copied directly from p4 rdma table
        raw_flags           : 16; // copied directly from p4 rdma table
        ecn                 : 2;
        payload_len         : 14;
        //Parsed UDP options
        roce_opt_ts_value   : 32;
        roce_opt_ts_echo    : 32;
        roce_opt_mss        : 16;
        roce_opt_pad        : 16;
        roce_int_recirc_hdr : 16;
    }
}

header_type p4_to_p4plus_tcp_proxy_base_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        from_ooq_txdma      : 1;
        pad                 : 1;
        ecn                 : 2;
        timestamp_valid     : 1;
        num_sack_blocks     : 3;
        payload_len         : 16;
        srcPort             : 16;
        dstPort             : 16;
        seqNo               : 32;
        ackNo               : 32;
        dataOffset          : 4;
        res                 : 4;
        flags               : 8;
        window              : 16;
        urgentPtr           : 16;
        ts                  : 32;
        prev_echo_ts        : 32;
    }
}

header_type p4_to_p4plus_tcp_proxy_sack_header_t {
    fields {
        start_seq0          : 32;
        end_seq0            : 32;
        start_seq1          : 32;
        end_seq1            : 32;
        start_seq2          : 32;
        end_seq2            : 32;
        start_seq3          : 32;
        end_seq3            : 32;
    }
}

header_type p4_to_p4plus_classic_nic_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        flow_id             : 24;
        vlan_pcp            : 3;
        vlan_dei            : 1;
        vlan_vid            : 12;
        packet_len          : 16;
        csum                : 16;
        csum_ip_bad         : 1;
        csum_ip_ok          : 1;
        csum_udp_bad        : 1;
        csum_udp_ok         : 1;
        csum_tcp_bad        : 1;
        csum_tcp_ok         : 1;
        vlan_valid          : 1;
        pad                 : 1;
        l2_pkt_type         : 2;
        pkt_type            : 6;
        l4_sport            : 16;
        l4_dport            : 16;
    }
}

header_type p4_to_p4plus_ip_addr_t {
    fields {
        ip_sa               : 128;
        ip_da               : 128;
    }
}

header_type p4_to_p4plus_cpu_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        ip_proto            : 8;
        l4_sport            : 16;
        l4_dport            : 16;
        packet_type         : 2;
        packet_len          : 14;
    }
}

/*
 * flags bits:
 *  0       : vlan valid
 *  1       : ipv4 valid
 *  2       : ipv6 valid
 *  3       : ip options present
 *  4       : tcp options present
 *  5       : from ipsec app
 *  6       : tunnel terminated
 *  7     : unused
 */
header_type p4_to_p4plus_cpu_pkt_t {
    fields {
        src_lif             : 16;

        lif                 : 16;
        qtype               : 8;
        qid                 : 32;

        lkp_vrf             : 16;

        src_lport           : 16;

        src_app_id          : 4;
        lkp_dir             : 1;
        lkp_inst            : 1;
        lkp_type            : 2;

        flags               : 8;

        // offsets
        l2_offset           : 16;
        l3_offset           : 16;
        l4_offset           : 16;
        payload_offset      : 16;

        // qos
        src_tm_iq           : 5;
        pad_1               : 3;

        // flow hash
        flow_hash           : 32;
    }
}

header_type p4_to_p4plus_cpu_tcp_pkt_t {
    fields {
        tcp_flags           : 8;
        tcp_seqNo           : 32;
        tcp_ackNo           : 32;
        tcp_window          : 16;
        tcp_options         : 8;
        tcp_mss             : 16;
        tcp_ws              : 8;
    }
}

header_type p4_to_p4plus_mirror_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        payload_len         : 16;

        session_id          : 8;
        direction           : 8;
        lif                 : 16;
        timestamp           : 64;
        capture_len         : 16;
        original_len        : 16;
    }
}

/*
 * flag bits:
 *  0 : update ip id
 *  1 : update ip len
 *  2 : update tcp seq number
 *  3 : update udp len
 *  4 : insert vlan header
 *  5 : compute outer checksums
 *  6 : compute inner checksums
 *  7 : lkp_inst
 */
header_type p4plus_to_p4_header_t {
    fields {
        p4plus_app_id           : 4;
        table0_valid            : 1;
        table1_valid            : 1;
        table2_valid            : 1;
        table3_valid            : 1;
        flow_index              : 24;
        flags                   : 8;
        udp_opt_bytes           : 8;
        dst_lport               : 11;
        dst_lport_valid         : 1;
        pad1                    : 1;
        tso_last_segment        : 1;
        tso_first_segment       : 1;
        tso_valid               : 1;
        ip_id_delta             : 16;
        tcp_seq_delta           : 32;
        gso_start               : 14;
        compute_inner_ip_csum   : 1;
        compute_ip_csum         : 1;
        gso_offset              : 14;
        flow_index_valid        : 1;
        gso_valid               : 1;
        vlan_tag                : 16;
    }
}

header_type p4plus_to_p4_s1_t {
    fields {
        p4plus_app_id           : 4;
        pad                     : 4;
        flow_index              : 24;
        lkp_inst                : 1;
        compute_inner_l4_csum   : 1;
        compute_l4_csum         : 1;
        insert_vlan_tag         : 1;
        update_udp_len          : 1;
        update_tcp_seq_no       : 1;
        update_ip_len           : 1;
        update_ip_id            : 1;
        udp_opt_bytes           : 8;
        dst_lport               : 11;
        dst_lport_valid         : 1;
        pad1                    : 1;
        tso_last_segment        : 1;
        tso_first_segment       : 1;
        tso_valid               : 1;
        ip_id_delta             : 16;
        tcp_seq_delta           : 32;
        gso_start               : 14;
        compute_inner_ip_csum   : 1;
        compute_ip_csum         : 1;
        gso_offset              : 14;
        flow_index_valid        : 1;
        gso_valid               : 1;
    }
}

header_type p4plus_to_p4_s2_t {
    fields {
        pcp                     : 3;
        dei                     : 1;
        vid                     : 12;
    }
}
