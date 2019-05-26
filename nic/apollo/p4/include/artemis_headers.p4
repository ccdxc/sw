header_type artemis_p4_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        table0_valid    : 1;
        table1_valid    : 1;
        table2_valid    : 1;
        table3_valid    : 1;

        pad0            : 6;
        sacl_bypass     : 1;
        direction       : 1;
        vnic_id         : 8;
        service_tag     : 32;

        flow_src        : 128;
        flow_sport      : 16;
        flow_proto      : 8;
        flow_dst        : 128;
        flow_dport      : 16;
        flow_ktype      : 4;
        padn            : 4;
    }
}
