header_type apollo_i2e_metadata_t {
    fields {
        dnat_required            : 1;
        pad0                     : 7;
        // TODO: padding local_vnic_tag to 16 bits so as to unionize it with
        // vnic_metadata.local_vnic_tag . Add pragmas to any index tables using
        // this
        local_vnic_tag           : 16;
        pad3                     : 7;
        xlate_index              : 17;
        dst                      : 128;
        pad4                     : 6;
        resource_group           : 10;
        pad5                     : 3;
        traffic_class            : 3;
        classified_policer_index : 10;
        pad6                     : 6;
        agg_policer_index        : 10;
        pad7                     : 4;
        src_slot_id              : 20;
        rvpath_subnet_id         : 16;
        rvpath_overlay_mac       : 48;
    }
}
