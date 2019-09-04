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
        lkp_id              : 16;
        src                 : 128;
        dst                 : 128;
        proto               : 8;
        dport               : 16;
        sport               : 16;
        ipv4_src            : 32;
        ipv4_dst            : 32;
        l2_dst              : 48;
    }
}

header_type vnic_metadata_t {
    fields {
        local_vnic_tag      : 16;
        vpc_id              : 16;
        skip_src_dst_check  : 1;
        vr_mac              : 48;
        overlay_mac         : 48;
        vlan_id             : 12;
        src_slot_id         : 24;
    }
}

header_type control_metadata_t {
    fields {
        mapping_lkp_addr                    : 128;
        local_l2_mapping_lkp_addr           : 48;
        local_mapping_ohash_lkp             : 1;
        remote_vnic_mapping_rx_ohash_lkp    : 1;
        remote_vnic_mapping_tx_ohash_lkp    : 1;
        skip_flow_lkp                       : 1;
        flow_ohash_lkp                      : 1;
        session_index                       : 24;
        flow_role                           : 1;
        span_copy                           : 1;
        direction                           : 1;
        p4i_drop_reason                     : 32;
        p4e_drop_reason                     : 32;
        sacl_v6addr                         : 40;
        lpm_v6addr                          : 40;
        p4plus_app_id                       : 8;
        vlan_strip                          : 1;
        local_switching                     : 1;
        mirror_session                      : 8;
        tunneled_packet                     : 1;
        fastpath                            : 1;
        mode                                : 2;
        parse_tcp_option_error              : 1;
    }
}

header_type rewrite_metadata_t {
    fields {
        encap_type          : 1;
        dst_slot_id_valid   : 1;
        tep_index           : 10;
        dst_slot_id         : 24;
        mytep_ip            : 32;
        src_slot_id         : 20;
        nexthop_index       : 12;
    }
}

header_type policer_metadata_t {
    fields {
        resource_group              : 10;
        traffic_class               : 3;
        agg_policer_index           : 10;
        agg_policer_color           : 1;
        agg_packet_len              : 16;
        classified_policer_index    : 10;
        classified_policer_color    : 1;
        classified_packet_len       : 16;
    }
}

header_type nat_metadata_t {
    fields {
        snat_required       : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        use_epoch1          : 1;
        epoch_valid         : 1;
        epoch               : 32;
        local_ip_hash       : 14;
        local_ip_hint       : 15;
        vnic_mapping_hash   : 11;
        vnic_mapping_hint   : 18;
        ipv4_src            : 32;
        flow_hash           : 9;
        flow_hint           : 22;
        in_packets          : 64;
        in_bytes            : 64;
        class_id            : 8;
        addr                : 32;
        local_vnic_tag      : 10;
        vpc_id              : 10;
        drop                : 1;
        mytep_ip            : 32;
        mytep_macsa         : 48;
        overlay_mac         : 48;
        drop_stats_pad      : 88;
        drop_stats_pkts     : 40;
        tcp_state           : 4;
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

        subnet_id           : 16;
        snat                : 1;
        dnat                : 1;
        encap_type          : 1;
        ip_totallen         : 16;
        packet_len          : 16;

        //common types
        mac                 : 48;
        ipv4                : 32;

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
        policer_packets     : 4;
        policer_bytes       : 18;
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
metadata policer_metadata_t     policer_metadata;
metadata nat_metadata_t         nat_metadata;

@pragma parser_end_offset payload_offset
metadata offset_metadata_t      offset_metadata;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t   capri_deparser_len;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
