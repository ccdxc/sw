header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len       : 16;
        ipv4_0_hdr_len      : 16;
        ipv4_1_hdr_len      : 16;
        ipv4_2_hdr_len      : 16;
        l4_payload_len      : 16;
    }
}

header_type key_metadata_t {
    fields {
        ktype               : 2;
        vlan                : 12;
        tenant_id           : 40;
        src                 : 128;
        dst                 : 128;
        proto               : 8;
        dport               : 16;
        sport               : 16;
        ipv4_src            : 32;
        ipv4_dst            : 32;
        tcp_flags           : 6;
        ingress_port        : 1;
    }
}

header_type control_metadata_t {
    fields {
        forward_to_uplink                   : 1;
        redir_to_rxdma                      : 1;
        skip_flow_lkp                       : 1;
        flow_ohash_lkp                      : 1;
        direction                           : 1;
        parse_tcp_option_error              : 1;
        flow_miss                           : 1;
        launch_v4                           : 1; // Dummy - never set
        p4plus_app_id                       : 8;
        config1_epoch                       : 32;
        config1_idx                         : 16;
        config2_epoch                       : 32;
        config2_idx                         : 16;
        throttle_pps                        : 16;
        throttle_bw                         : 16;
        rx_packet_len                       : 14;
        tx_packet_len                       : 14;
        p4i_drop_reason                     : 32;
        p4e_drop_reason                     : 32;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        ipv4_src            : 32;
        flow_hash           : 11;
        flow_hint           : 18;
        class_id            : 8;
        addr                : 32;
        local_vnic_tag      : 10;
        vpc_id              : 10;
        drop                : 1;
        tcp_seq_num         : 32;
        tcp_ack_num         : 32;
        tcp_win_sz          : 16;
        tcp_win_scale       : 4;
        last_seen_timestamp : 48;
        tcp_flags           : 8;
        session_stats_addr  : 34;
        hint_valid          : 1;
        cpu_flags           : 16;
        nexthop_index       : 12;
        num_nexthops        : 4;
        pad6                : 6;
        update_ip_chksum    : 1;
        update_l4_chksum    : 1;


        //common types
        mac                 : 48;
        ipv4                : 32;

        flow_data_pad       : 7;

        // Session info
        timestamp           : 48;
        config_epoch        : 32;
        config_substrate_src_ip : 32;
        pop_hdr_flag        : 1;
        user_pkt_rewrite_type   : 2;
        user_pkt_rewrite_ip : 128;

        // Session info - substrate encap to switch
        encap_type          : 3;
        smac                : 48;
        dmac                : 48;
        vlan                : 12;
        ip_ttl              : 8;
        ip_saddr            : 32;
        ip_daddr            : 32;
        udp_sport           : 16;
        udp_dport           : 16;
        mpls1_label         : 20;
        mpls2_label         : 20;

        // Counters
        counter_rx          : 64;
        counter_tx          : 64;

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
metadata control_metadata_t     control_metadata;


metadata offset_metadata_t      offset_metadata;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t   capri_deparser_len;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
