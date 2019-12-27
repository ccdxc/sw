header_type apulu_i2e_metadata_t {
    fields {
        mapping_lkp_addr    : 128;
        entropy_hash        : 32;
        mapping_lkp_type    : 2;
        flow_role           : 1;
        session_index       : 21;
        vnic_id             : 16;
        bd_id               : 16;
        vpc_id              : 16;
        mapping_lkp_id      : 16;
        nexthop_id          : 16;
        xlate_id            : 16;
        pad0                : 3;
        rx_packet           : 1;
        nexthop_type        : 2;
        mapping_bypass      : 1;
        update_checksum     : 1;
        pad1                : 6;
        copp_policer_id     : 10;
    }
}
