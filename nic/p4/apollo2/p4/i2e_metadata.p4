header_type apollo_i2e_metadata_t {
    fields {
        direction                : 1;
        pad0                     : 5;
        local_vnic_tag           : 10;
        pad1                     : 6;
        vcn_id                   : 10;
        pad2                     : 6;
        nexthop_index            : 10;
        pad3                     : 6;
        xlate_index              : 10;
        dst                      : 128;
        pad4                     : 6;
        resource_group           : 10;
        pad5                     : 3;
        traffic_class            : 3;
        classified_policer_index : 10;
        pad6                     : 6;
        agg_policer_index        : 10;
    }
}
