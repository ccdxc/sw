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
        // TODO: padding local_vnic_tag to 16 bits so as to unionize it with
        // vnic_metadata.local_vnic_tag . Add pragmas to any index tables using
        // this
        local_vnic_tag      : 16;
        // TODO: padding vcn_id to 16 bits so as to unionize it with
        // vnic_metadata.vcn_id . Add pragmas to any index tables using
        // this
        vcn_id              : 16;
        skip_src_dst_check  : 1;
        vr_mac              : 48;
        overlay_mac         : 48;
        vlan_id             : 12;
        src_slot_id         : 20;
    }
}

header_type rvpath_metadata_t {
    fields {
        tep_idx     : 10;
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
        encap_type              : 1;
        tep_index               : 10;
        dst_slot_id             : 20;
        mytep_ip                : 32;
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

header_type nat_metadata_t {
    fields {
        snat_required       : 1;
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
        flow_hint           : 21;
        in_packets          : 64;
        in_bytes            : 64;
        class_id            : 8;
        addr                : 32;
        local_vnic_tag      : 10;
        vcn_id              : 10;
        drop                : 1;
        mytep_ip            : 32;
        mytep_macsa         : 48;
        overlay_mac         : 48;

        subnet_id           : 16;
        snat                : 1;
        dnat                : 1;
        encap_type          : 1;

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

metadata key_metadata_t         key_metadata;
metadata vnic_metadata_t        vnic_metadata;
metadata rvpath_metadata_t      rvpath_metadata;
metadata control_metadata_t     control_metadata;
metadata rewrite_metadata_t     rewrite_metadata;
metadata tunnel_metadata_t      tunnel_metadata;
metadata policer_metadata_t     policer_metadata;
metadata nat_metadata_t         nat_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
