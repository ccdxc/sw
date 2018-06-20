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

header_type lpm_metadata_t {
    fields {
        done        : 1;
        addr        : 34;
        base_addr   : 34;
    }
}

header_type tunnel_metadata_t {
    fields {
        tunnel_type : 8;
        tunnel_vni  : 24;
    }
}

header_type control_metadata_t {
    fields {
        ingress_vnic            : 12;
        egress_vnic             : 12;
        subnet_id               : 16;
        ep_mapping_ohash_lkp    : 1;
        source_guard_ohash_lkp  : 1;
        skip_policy_lkp         : 1;
        policy_ohash_lkp        : 1;
        policy_index            : 23;
        span_copy               : 1;
        direction               : 1;
        drop_reason             : 32;
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
        resource_group1 : 10;
        resource_group2 : 10;
    }
}

header_type slacl_metadata_t {
    fields {
        base_addr   : 34;
        addr1       : 34;
        addr2       : 34;
        ip_15_00    : 16;
        ip_31_16    : 16;
        class_id0   : 14;
        class_id1   : 16;
        class_id2   : 16;
        drop        : 1;
        stats_index : 20;
    }
}

header_type scratch_metadata_t {
    fields {
        flag        : 1;
        use_epoch1  : 1;
        epoch       : 4;
        ep_hash     : 11;
        ep_hint     : 18;
        ip_src      : 128;
        policy_hash : 8;
        policy_hint : 18;
        in_packets  : 64;
        in_bytes    : 64;
        lpm_data    : 15;
        class_id    : 8;
        rule_id     : 8;
    }
}

metadata key_metadata_t         key_metadata;
metadata lpm_metadata_t         lpm_metadata;
metadata slacl_metadata_t       slacl_metadata;
metadata control_metadata_t     control_metadata;
metadata rewrite_metadata_t     rewrite_metadata;
metadata tunnel_metadata_t      tunnel_metadata;
metadata policer_metadata_t     policer_metadata;

@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;
