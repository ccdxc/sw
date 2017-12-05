header_type cap_phv_intr_global_t {
    fields {
        tm_iport            : 4;
        tm_oport            : 4;
        tm_iq               : 5;
        lif                 : 11;
        timestamp           : 48;
        tm_span_session     : 8;
        tm_replicate_ptr    : 16;
        tm_replicate_en     : 1;
        tm_cpu              : 1;
        tm_q_depth          : 14;
        drop                : 1;
        bypass              : 1;
        hw_error            : 1;
        tm_oq               : 5;
        debug_trace         : 1;
        csum_err            : 5;
        error_bits          : 6;
        tm_instance_type    : 4;
    }
}

header_type cap_phv_intr_p4_t {
    fields {
        crc_err             : 1;
        len_err             : 4;
        recirc_count        : 3;
        parser_err          : 1;
        p4_pad              : 1;
        frame_size          : 14;
        no_data             : 1;
        recirc              : 1;
        packet_len          : 14;
    }
}

header_type cap_phv_intr_txdma_t {
    fields {
        qid                 : 24;
        dma_cmd_ptr         : 6;
        qstate_addr         : 34;
        qtype               : 3;
        txdma_rsv           : 5;
    }
}

header_type cap_phv_intr_rxdma_t {
    fields {
        qid                 : 24;
        dma_cmd_ptr         : 6;
        qstate_addr         : 34;
        qtype               : 3;
        rx_splitter_offset  : 10;
        rxdma_rsv           : 3;
    }
}

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
        flow_hash           : 32;
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
        roce_opt_ts_vld     : 1;
        roce_opt_msss_vld   : 1;
        rdma_hdr_len        : 6;  // copied directly from p4 rdma table
        raw_flags           : 16; // copied directly from p4 rdma table
        ecn                 : 2;
        payload_len         : 14;
        //Parsed UDP options
        roce_opt_timestamp  : 64;
        roce_opt_new        : 32;
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
        gft_flow_id         : 24;
        num_sack_blocks     : 8;
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
        tcp_pad1            : 24;
        prev_echo_ts        : 32;
        tcp_pad             : 24;
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

/*
 * header flags :
 *  0       : ipv4
 *  1       : ipv4 tcp
 *  2       : ipv4 udp
 *  3       : ipv6
 *  4       : ipv6 tcp
 *  5       : ipv6 udp
 *  [6-11]  : unused
 */

/*
 * flags :
 *  0       : vlan valid
 *  [1-3]   : unused
 */

/*
 * checksum flags:
 *  0       : checksum verified by hardware
 *  1       : l3 checksum ok
 *  2       : l4 checksum ok
 *  3       : inner checksum verified by hardware
 *  4       : inner l3 checksum ok
 *  5       : inner l4 checksum ok
 *  [6-7]   : unused
 */
header_type p4_to_p4plus_classic_nic_header_t {
    fields {
        p4plus_app_id       : 4;
        table0_valid        : 1;
        table1_valid        : 1;
        table2_valid        : 1;
        table3_valid        : 1;
        header_flags        : 12;
        flags               : 4;
        checksum_flags      : 8;
        l4_checksum         : 16;
        vlan_pcp            : 3;
        vlan_dei            : 1;
        vlan_vid            : 12;
        packet_len          : 16;
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
        packet_len          : 16;
        flow_hash           : 32;
    }
}

/*
 * flags bits:
 *  0       : vlan valid
 *  1       : ipv4 valid
 *  2       : ipv6 valid
 *  3       : ip options present
 *  4       : tcp options present
 *  5-7     : unused
 */
header_type p4_to_p4plus_cpu_pkt_t {
    fields {
        src_lif             : 16;

        lif                 : 16;
        qtype               : 8;
        qid                 : 32;

        lkp_vrf             : 16;

        pad                 : 2;
        lkp_dir             : 1;
        lkp_inst            : 1;
        lkp_type            : 4;

        flags               : 8;

        // offsets
        l2_offset           : 16;
        l3_offset           : 16;
        l4_offset           : 16;
        payload_offset      : 16;

        // tcp
        tcp_flags           : 8;
        tcp_seqNo           : 32;
        tcp_AckNo           : 32;
        tcp_window          : 16;
        tcp_options         : 8;
        tcp_mss             : 16;
        tcp_ws              : 8;
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
        p4plus_app_id       : 4;
        pad                 : 4;
        flags               : 8;
        ip_id_delta         : 16;
        tcp_seq_delta       : 32;
        vlan_tag            : 16;
#ifdef GSO_CSUM
        gso_start           : 14; // Should be adjusted to include size of all intrinsic hdrs
                                  // that appear before L2 hdr
        gso_offset          : 14; // should be adjusted to include size of all intrinsic hdrs
                                  // that appear before L2 hdr
        byte_align_pad      : 3;
        gso_valid           : 1;
#endif
    }
}


/*
 * flag bits:
 *  0 : forwarding and rewrite complete, just enqueue the packet
 *  1 : perform forwarding lookup, no rewrites
 *  2 : perform forwarding and rewrites
 *  3 : update stats
 */
header_type cpu_to_p4_header_t {
    fields {
        p4plus_app_id       : 4;
        flags               : 4;
        src_lif             : 11;
        dst_lif             : 11;
        iclass              : 4;
        oclass              : 4;
        dest_type           : 2;
        dest                : 16;
        oqueue              : 5;
    }
}
