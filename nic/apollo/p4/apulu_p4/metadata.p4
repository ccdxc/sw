header_type capri_deparser_len_t {
    fields {
        trunc_pkt_len   : 16;
        ipv4_0_hdr_len  : 16;
        ipv4_1_hdr_len  : 16;
        l4_payload_len  : 16;
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
    }
}

header_type vnic_metadata_t {
    fields {
        vnic_id         : 8;
        vpc_id          : 8;
        vlan_id         : 12;
    }
}

header_type control_metadata_t {
    fields {
        p4i_drop_reason     : 32;
        p4e_drop_reason     : 32;
        span_copy           : 1;
        update_checksum     : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        hint_valid          : 1;
        flow_hash           : 0;
        flow_hint           : 0;
        in_packets          : 64;
        in_bytes            : 64;
        drop_stats_pad      : 88;
        drop_stats_pkts     : 40;
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

@pragma parser_end_offset payload_offset
metadata offset_metadata_t      offset_metadata;

@pragma deparser_variable_length_header
@pragma dont_trim
metadata capri_deparser_len_t   capri_deparser_len;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
