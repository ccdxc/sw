header_type apollo_i2e_metadata_t {
    fields {
        dst                         : 128;
        local_vnic_tag              : 16;
        rvpath_subnet_id            : 16;
        rvpath_overlay_mac          : 48;
        entropy_hash                : 16;
        traffic_class               : 3;
        dnat_required               : 1;
        src_slot_id                 : 20;
        pad0                        : 7;
        xlate_index                 : 17;
        pad1                        : 6;
        resource_group              : 10;
        pad2                        : 6;
        classified_policer_index    : 10;
        pad4                        : 6;
        agg_policer_index           : 10;
    }
}
