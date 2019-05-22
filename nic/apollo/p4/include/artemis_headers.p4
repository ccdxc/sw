header_type artemis_p4_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        table0_valid    : 1;
        table1_valid    : 1;
        table2_valid    : 1;
        table3_valid    : 1;

        pad0            : 5;
        direction       : 1;
        sacl_base_addr  : 34;
        pad1            : 6;
        lpm_base_addr   : 34;
        pad2            : 6;
        meter_base_addr : 34;
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

header_type artemis_p4_to_txdma_header_t {
    fields {
        p4plus_app_id   : 4;
        pad0            : 2;
        lpm_addr        : 34;
        pad2            : 2;
        payload_len     : 14;
        lpm_dst         : 128;
        vpc_id          : 8;
    }
}
