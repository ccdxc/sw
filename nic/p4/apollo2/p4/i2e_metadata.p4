header_type capri_i2e_metadata_t {
    fields {
        direction       : 1;
        pad0            : 5;
        local_vnic_tag  : 10;
        pad1            : 6;
        vcn_id          : 10;
        pad2            : 6;
        nexthop_index   : 10;
        pad3            : 6;
        xlate_index     : 10;
        dst             : 128;
    }
}
