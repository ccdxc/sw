header_type apulu_p4i_to_rxdma_header_t {
    fields {
        p4plus_app_id : 4;
        lpm1_enable   : 1;
        lpm2_enable   : 1;
        vnic_info_en  : 1;
        apulu_p4plus  : 1;

        rx_packet     : 1;
        iptype        : 1;
        pad0          : 6;
        vnic_id       : 8;
        vpc_id        : 8;

        flow_src      : 128;
        flow_dst      : 128;
        flow_sport    : 16;
        flow_dport    : 16;
        flow_proto    : 8;

        service_tag   : 32;
        tag_root      : 40;
    }
}

header_type apulu_txdma_to_p4e_header_t {
    fields {
        p4plus_app_id  : 4;
        pad0           : 3;
        drop           : 1;
        mapping_lkp_id : 16;
        nexthop_idx    : 24;
        meter_idx      : 16;
    }
}

header_type apulu_ingress_recirc_header_t {
    fields {
        flow_ohash : 32;
        local_mapping_ohash : 32;
        pad1 : 6;
        flow_done : 1;
        local_mapping_done : 1;
    }
}

header_type apulu_egress_recirc_header_t {
    fields {
        mapping_ohash : 32;
        pad1 : 7;
        mapping_done : 1;
    }
}

header_type apulu_rx_to_tx_header_t {
    fields {
        remote_ip       : 128;// Bytes 0 to 15
        sacl_base_addr  : 40; // Bytes 16 to 20
        route_base_addr : 40; // Bytes 21 to 25
        meter_result    : 10; // Bytes 26 and 27
        sip_classid     : 10; // Bytes 27 and 28
        dip_classid     : 10; // Bytes 28 and 29
        stag_classid    : 10; // Bytes 29 and 30
        dtag_classid    : 8;  // Bytes 31
        sport_classid   : 8;  // Bytes 32
        dport_classid   : 8;  // Bytes 33
        vpc_id          : 8;  // Bytes 34
        vnic_id         : 8;  // Bytes 35
        payload_len     : 14; // Bytes 36 and 37
        iptype          : 1;  // Bytes 37 7b
        rx_packet       : 1;  // Bytes 37 8b
        pad1            : 208; // Bytes 38 to 63
        // Please check the above comment when adding new fields
    }
}
