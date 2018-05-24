header_type flow_lkp_metadata_t {
    fields {
        ethernet_dst_1           : 48;
        ethernet_src_1           : 48;
        ethernet_type_1          : 16;
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
        tunnel_type_1            : 8;

        mpls0_label              : 20;
        mpls0_exp                : 3;
        mpls0_bos                : 1;
        mpls0_ttl                : 8;
        mpls1_label              : 20;
        mpls1_exp                : 3;
        mpls1_bos                : 1;
        mpls1_ttl                : 8;
        mpls2_label              : 20;
        mpls2_exp                : 3;
        mpls2_bos                : 1;
        mpls2_ttl                : 8;

        ethernet_dst_2           : 48;
        ethernet_src_2           : 48;
        ethernet_type_2          : 16;
        ctag_2                   : 12;
        ip_dscp_2                : 8;
        ip_proto_2               : 8;
        ip_ttl_2                 : 8;
        ip_src_2                 : 128;
        ip_dst_2                 : 128;
        l4_sport_2               : 16;
        l4_dport_2               : 16;
        tcp_flags_2              : 8;

        custom_field_1           : 32;
        custom_field_2           : 32;
        custom_field_3           : 32;

        overflow_hash            : 32;
    }
}

header_type tunnel_metadata_t {
    fields {
        tunnel_type_1 : 8;
        tunnel_vni_1  : 24;
    }
}

header_type l4_metadata_t {
    fields {
        l4_sport_1     : 16;
        l4_dport_1     : 16;
        l4_sport_2     : 16;
        l4_dport_2     : 16;
    }
}

header_type flow_act_metadata_t {
    fields {
        prio                      : 8;
        flow_index                : 16;
        parent_policer_index      : 13;
        child_policer_index       : 13;
        tcam_prio                 : 8;
        tcam_index                : 10;
        tcam_flow_index           : 20;
        tcam_parent_policer_index : 13;
        tcam_child_policer_index  : 13;
        tcam_lif                  : 11;
        tcam_tm_oport             : 4;
        overflow_lkp              : 1;
    }
}

header_type control_metadata_t {
    fields {
        update_checksum_1   : 1;
        update_checksum_2   : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        last_seen_timestamp : 48;
        flow_packets        : 64;
        flow_bytes          : 64;
        rewrite_fields      : 8;
        payload_len         : 16;
        flag                : 1;
        match_fields        : 32;
        hdr_bits            : 32;
        in_packets          : 64;
        in_bytes            : 64;
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
metadata control_metadata_t    control_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t    scratch_metadata;
