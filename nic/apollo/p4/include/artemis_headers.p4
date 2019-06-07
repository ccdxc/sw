header_type artemis_p4_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        lpm1_enable     : 1;
        lpm2_enable     : 1;
        vnic_info_en    : 1;
        cps_path_en     : 1;
        aging_enable    : 1;
        
        pad0            : 5;
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

header_type artemis_p4_to_rxdma_header2_t {
    fields {
        service_xlate_idx   : 16;
        pa_or_ca_xlate_idx  : 16;
        public_xlate_idx    : 16;
        pad0                : 48;
    }
}

header_type artemis_rx_to_tx_header_t {
    fields {
        remote_ip       : 128;// Bytes 0 to 15
        sacl_base_addr  : 40; // Bytes 16 to 20
        route_base_addr : 40; // Bytes 21 to 25
        sip_classid     : 10; // Bytes 26 and 27
        dip_classid     : 10; // Bytes 27 and 28
        tag_classid     : 10; // Bytes 28 and 29
        meter_result    : 10; // Bytes 29 and 30
        dport_classid   : 8;  // Byte 31
        sport_classid   : 8;  // Byte 32
        vnic_id         : 8;  // Byte 33
        tag2_classid    : 8;  // Byte 34
    }
}

