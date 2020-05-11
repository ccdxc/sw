header_type apulu_i2e_metadata_t {
    fields {
        mapping_lkp_addr    : 128;
        entropy_hash        : 32;
        mapping_lkp_type    : 2;
        flow_role           : 1;
        session_id          : 21;
        vnic_id             : 16;
        bd_id               : 16;
        vpc_id              : 16;
        mapping_lkp_id      : 16;
        nexthop_id          : 16;
        xlate_id            : 16;
        meter_enabled       : 1;
        rx_packet           : 1;
        nexthop_type        : 2;
        mapping_bypass      : 1;
        update_checksum     : 1;
        copp_policer_id     : 10;
        priority            : 5;
        src_lif             : 11;
        mirror_session      : 8;
        pad0                : 5;
        skip_stats_update   : 1;
        tx_policer_id       : 10;
    }
}
