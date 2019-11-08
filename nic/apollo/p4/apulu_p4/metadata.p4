header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len   : 16;
        ipv4_0_hdr_len  : 16;
        ipv4_1_hdr_len  : 16;
        l4_payload_len  : 16;
        l2_checksum_len : 16;
    }
}

header_type key_metadata_t {
    fields {
        ktype           : 2;
        src             : 128;
        dst             : 128;
        proto           : 8;
        sport           : 16;
        dport           : 16;
        ipv4_src        : 32;
        ipv4_dst        : 32;

        parsed_sport    : 16;
        parsed_dport    : 16;

        local_mapping_lkp_type  : 2;
        local_mapping_lkp_id    : 16;
        local_mapping_lkp_addr  : 128;
    }
}

header_type vnic_metadata_t {
    fields {
        vnic_id         : 16;
        bd_id           : 16;
        egress_bd_id    : 16;
        vpc_id          : 16;
    }
}

header_type control_metadata_t {
    fields {
        p4i_drop_reason         : 32;
        p4e_drop_reason         : 32;
        rx_packet               : 1;
        tunneled_packet         : 1;
        flow_miss               : 1;
        flow_miss_redirect      : 1;
        pinned_lif              : 11;
        pinned_qtype            : 3;
        pinned_qid              : 24;
        local_mapping_ohash_lkp : 1;
        flow_ohash_lkp          : 1;
        mapping_ohash_lkp       : 1;
        epoch                   : 8;
        span_copy               : 1;
        update_checksum         : 1;
        egress_bypass           : 1;
        l2_enabled              : 1;
        learn_enabled           : 1;
        device_profile_id       : 4;
        to_device_ip            : 1;
        copp_policer_valid      : 1;
        apply_nat               : 1;
        apply_nat2              : 1;
    }
}

header_type rewrite_metadata_t {
    fields {
        flags               : 16;
        xlate_id            : 16;
        xlate_id2           : 16;
        policer_id          : 16;
        dmaci               : 48;
        device_ipv4_addr    : 32;
        device_ipv6_addr    : 128;
        nexthop_type        : 2;
        vrmac               : 48;
        vni                 : 24;
        ip_type             : 1;
        tunnel_dmaci        : 48;
        tunnel_vni          : 24;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        hint_valid          : 1;
        epoch               : 8;
        local_mapping_hash  : 15;
        local_mapping_hint  : 14;
        mapping_hash        : 10;
        mapping_hint        : 19;
        flow_hash           : 11;
        flow_hint           : 18;
        in_packets          : 64;
        in_bytes            : 64;
        drop_stats_pad      : 88;
        drop_stats_pkts     : 40;
        mac                 : 48;
        pad12               : 12;
        tcp_state           : 4;
        tcp_seq_num         : 32;
        tcp_ack_num         : 32;
        tcp_win_sz          : 16;
        tcp_win_scale       : 4;
        timestamp           : 48;
        tcp_flags           : 8;
        session_stats_addr  : 34;
        num_nexthops        : 4;
        nexthop_id          : 16;
        ip_totallen         : 16;
        packet_len          : 16;
        ipv4_addr           : 32;
        ipv6_addr           : 128;
        update_ip_chksum    : 1;
        update_l4_chksum    : 1;
        cpu_flags           : 16;

        // policer
        policer_valid       : 1;
        policer_pkt_rate    : 1;
        policer_rlimit_en   : 1;
        policer_rlimit_prof : 2;
        policer_color_aware : 1;
        policer_rsvd        : 1;
        policer_axi_wr_pend : 1;
        policer_burst       : 40;
        policer_rate        : 40;
        policer_tbkt        : 40;
    }
}

header_type offset_metadata_t {
    fields {
        l2_1                : 8;
        l2_2                : 8;
        l3_1                : 8;
        l3_2                : 8;
        l4_1                : 8;
        l4_2                : 8;
        payload_offset      : 16;
    }
}

metadata key_metadata_t         key_metadata;
metadata vnic_metadata_t        vnic_metadata;
metadata control_metadata_t     control_metadata;
metadata rewrite_metadata_t     rewrite_metadata;

@pragma parser_end_offset payload_offset
metadata offset_metadata_t      offset_metadata;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t   capri_deparser_len;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
