header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len   : 16;
        ipv4_0_hdr_len  : 16;
        ipv4_1_hdr_len  : 16;
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
        mapping_ip      : 128;
        mapping_ip2     : 128;
        mapping_port    : 16;
    }
}

header_type vnic_metadata_t {
    fields {
        vnic_id         : 8;
        vpc_id          : 8;
        src_vpc_id      : 16;

        vr_mac          : 48;
        overlay_mac     : 48;
        vlan_id         : 12;
    }
}

header_type control_metadata_t {
    fields {
        p4i_drop_reason     : 32;
        p4e_drop_reason     : 32;
        direction           : 1;
        span_copy           : 1;
        epoch               : 8;
        skip_flow_lkp       : 1;
        flow_ohash_lkp      : 1;
        pipe_id             : 4;
        mirror_session      : 8;
        vlan_strip          : 1;
    }
}

header_type tunnel_metadata_t {
    fields {
        decap_next          : 1;
        tep2_dst            : 128;
    }
}

header_type rewrite_metadata_t {
    fields {
        encap_src_ip        : 128;
        ip                  : 128;
        l4port              : 16;
        pa_mac              : 48;
        nexthop_idx         : 20;
        flags               : 8;
        policer_idx         : 12;
        meter_idx           : 16;
    }
}

header_type nat_metadata_t {
    fields {
        xlate_idx           : 16;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        hint_valid          : 1;
        flow_hash           : 9;
        flow_hint           : 22;
        in_packets          : 64;
        in_bytes            : 64;
        epoch               : 8;
        cpu_flags           : 16;
        timestamp           : 48;
        ip_totallen         : 16;
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
        tag_root_addr       : 40;
        meter_idx           : 16;
        packet_len          : 16;
    }
}

header_type offset_metadata_t {
    fields {
        l2_1                : 8;
        l2_2                : 8;
        l2_3                : 8;
        l3_1                : 8;
        l3_2                : 8;
        l3_3                : 8;
        l4_1                : 8;
        l4_2                : 8;
        l4_3                : 8;
        payload_offset      : 16;
    }
}

metadata key_metadata_t         key_metadata;
metadata vnic_metadata_t        vnic_metadata;
metadata control_metadata_t     control_metadata;
metadata tunnel_metadata_t      tunnel_metadata;
metadata rewrite_metadata_t     rewrite_metadata;
metadata nat_metadata_t         nat_metadata;

@pragma parser_end_offset payload_offset
metadata offset_metadata_t      offset_metadata;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t   capri_deparser_len;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
