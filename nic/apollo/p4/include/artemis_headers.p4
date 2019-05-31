header_type artemis_p4_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        lpm1_enable     : 1;
        lpm2_enable     : 1;
        vnic_info_en    : 1;
        cps_path_en     : 1;

        pad0            : 6;
        iptype          : 1;
        direction       : 1;
        vnic_id         : 8;
        tag_root        : 40;

        pad1            : 24;

        flow_src        : 128;
        flow_sport      : 16;
        flow_dport      : 16;
        flow_proto      : 8;
        flow_dst        : 128;
        service_tag     : 32;
    }
}
