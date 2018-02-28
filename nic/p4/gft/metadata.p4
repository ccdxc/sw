header_type flow_lkp_metadata_t {
    fields {
        ethernet_dst_1           : 48;
        ethernet_src_1           : 48;
        ethernet_type_1          : 16;
        pad_1                    : 4;
        ctag_1                   : 12;
        ip_dscp_1                : 8;
        ip_proto_1               : 8;
        ip_ttl_1                 : 8;
        ip_src_1                 : 128;
        ip_dst_1                 : 128;
        l4_sport_1               : 16;
        l4_dport_1               : 16;
        tcp_flags_1              : 8;
        tenant_id_1              : 24;
        gre_proto_1              : 16;

        ethernet_dst_2           : 48;
        ethernet_src_2           : 48;
        ethernet_type_2          : 16;
        pad_2                    : 4;
        ctag_2                   : 12;
        ip_dscp_2                : 8;
        ip_proto_2               : 8;
        ip_ttl_2                 : 8;
        ip_src_2                 : 128;
        ip_dst_2                 : 128;
        l4_sport_2               : 16;
        l4_dport_2               : 16;
        tcp_flags_2              : 8;
        tenant_id_2              : 24;
        gre_proto_2              : 16;

        ethernet_dst_3           : 48;
        ethernet_src_3           : 48;
        ethernet_type_3          : 16;
        pad_3                    : 4;
        ctag_3                   : 12;
        ip_dscp_3                : 8;
        ip_proto_3               : 8;
        ip_ttl_3                 : 8;
        ip_src_3                 : 128;
        ip_dst_3                 : 128;
        l4_sport_3               : 16;
        l4_dport_3               : 16;
        tcp_flags_3              : 8;
        tenant_id_3              : 24;
        gre_proto_3              : 16;

        tx_pad                   : 72;
        overflow_hash            : 32;
    }
}

header_type tunnel_metadata_t {
    fields {
        tunnel_type_1 : 8;
        tunnel_vni_1  : 24;
        tunnel_type_2 : 8;
        tunnel_vni_2  : 24;
        tunnel_type_3 : 8;
        tunnel_vni_3  : 24;
    }
}

header_type l4_metadata_t {
    fields {
        l4_sport_1     : 16;
        l4_dport_1     : 16;
        l4_sport_2     : 16;
        l4_dport_2     : 16;
        l4_sport_3     : 16;
        l4_dport_3     : 16;
    }
}

header_type flow_act_metadata_t {
    fields {
        //flow_index        : 20;
        flow_index          : 16;
        policer_index       : 14;
        overflow_lkp        : 1;
        tx_ethernet_dst     : 48;
    }
}

header_type tcam_entry_t {
    fields {
        inactive : 1;
    }
}

header_type roce_metadata_t {
    fields {
        roce_valid          : 1;
        rdma_enabled        : 1;
        udp_len             : 16;
        ecn                 : 2;
    }
}

header_type control_metadata_t {
    fields {
        update_checksum_1   : 1;
        update_checksum_2   : 1;
        update_checksum_3   : 1;
        skip_flow_lkp       : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        last_seen_timestamp : 48;
        flow_packets        : 64;
        flow_bytes          : 64;
        packet_len          : 16;
        flag                : 1;
        match_fields        : 32;
        hdr_bits            : 24;
        num_packets         : 64;
        num_bytes           : 64;
        hash                : 12;
        hint                : 16;
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

metadata cap_phv_intr_p4_t     capri_p4_intrinsic;
metadata tunnel_metadata_t     tunnel_metadata;
metadata l4_metadata_t         l4_metadata;
metadata flow_lkp_metadata_t   flow_lkp_metadata;
metadata flow_act_metadata_t   flow_action_metadata;
metadata roce_metadata_t       roce_metadata;
metadata control_metadata_t    control_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t    scratch_metadata;
