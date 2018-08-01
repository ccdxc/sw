header_type key_metadata_t {
    fields {
        ktype   : 4;
        src     : 128;
        dst     : 128;
        proto   : 8;
        dport   : 16;
        sport   : 16;
    }
}

header_type vnic_metadata_t {
    fields {
        local_vnic_tag      : 10;
        vcn_id              : 10;
        subnet_id           : 16;
        skip_src_dst_check  : 1;
        vr_mac              : 48;
        overlay_mac         : 48;
        vlan_id             : 12;
        src_slot            : 20;
    }
}

header_type rvpath_metadata_t {
    fields {
        subnet_id   : 16;
        overlay_mac : 48;
        tep_idx     : 12;
        src_slot_id : 20;
    }
}

header_type tunnel_metadata_t {
    fields {
        tunnel_type : 8;
    }
}

header_type control_metadata_t {
    fields {
        mapping_lkp_addr                : 128;
        ingress_vnic                    : 12;
        egress_vnic                     : 12;
        local_ip_mapping_ohash_lkp      : 1;
        remote_vnic_mapping_ohash_lkp   : 1;
        skip_flow_lkp                   : 1;
        flow_ohash_lkp                  : 1;
        flow_index                      : 23;
        span_copy                       : 1;
        direction                       : 1;
        drop_reason                     : 32;
    }
}

header_type rewrite_metadata_t {
    fields {
        tunnel_terminate        : 1;
        egress_tunnel_terminate : 1;
        nexthop_index           : 10;
        tunnel_rewrite_index    : 10;
    }
}

header_type policer_metadata_t {
    fields {
        resource_group                  : 10;
        traffic_class                   : 3;
        agg_policer_index               : 10;
        agg_policer_color               : 1;
        agg_packet_len                  : 16;
        classified_policer_index        : 10;
        classified_policer_color        : 1;
        classified_packet_len           : 16;
    }
}

header_type slacl_metadata_t {
    fields {
        bypass      : 1;
        base_addr   : 34;
        addr1       : 34;
        addr2       : 34;
        ip_15_00    : 16;
        ip_31_16    : 16;
    }
}

header_type lpm_metadata_t {
    fields {
        bypass      : 1;
        addr        : 34;
        base_addr   : 34;
    }
}

header_type nat_metadata_t {
    fields {
        xlate_index : 10;
        dnat        : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        flag                : 1;
        use_epoch1          : 1;
        epoch               : 32;
        local_ip_hash       : 15;
        local_ip_hint       : 14;
        remote_vnic_hash    : 11;
        remote_vnic_hint    : 18;
        ipv4_src            : 32;
        flow_hash           : 8;
        flow_hint           : 18;
        in_packets          : 64;
        in_bytes            : 64;
        class_id            : 8;
        addr                : 32;
        vcn_id              : 10;
        drop                : 1;
        mytep_ip            : 32;
        macsa               : 48;

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

metadata key_metadata_t         key_metadata;
metadata vnic_metadata_t        vnic_metadata;
metadata rvpath_metadata_t      rvpath_metadata;
metadata control_metadata_t     control_metadata;
metadata rewrite_metadata_t     rewrite_metadata;
metadata tunnel_metadata_t      tunnel_metadata;
metadata policer_metadata_t     policer_metadata;
metadata slacl_metadata_t       slacl_metadata;
metadata lpm_metadata_t         lpm_metadata;
metadata nat_metadata_t         nat_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
