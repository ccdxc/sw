header_type apulu_p4i_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        lpm1_enable     : 1;
        lpm2_enable     : 1;
        vnic_info_en    : 1;
        apulu_p4plus    : 1;

        pad0            : 6;
        rx_packet       : 1;
        iptype          : 1;
        vnic_info_key   : 16;
        vpc_id          : 16;
        tag_root        : 40;

        flow_src        : 128;
        flow_sport      : 16;
        flow_dport      : 16;
        flow_proto      : 8;
        flow_dst        : 128;
        service_tag     : 32;
    }
}

header_type apulu_txdma_to_p4e_header_t {
    fields {
        pad             : 5;
        nexthop_type    : 2;
        drop            : 1;
        nexthop_id      : 16;
        meter_id        : 16;
    }
}

header_type apulu_p4_to_arm_header_t {
    fields {
        packet_len          : 16;
        data                : 8;
        flags               : 8;
        ingress_bd_id       : 16;
        flow_hash           : 32;

        l2_1_offset         : 8;
        l3_1_offset         : 8;
        l4_1_offset         : 8;
        l2_2_offset         : 8;
        l3_2_offset         : 8;
        l4_2_offset         : 8;
        payload_offset      : 8;

        lif                 : 16;
        egress_bd_id        : 16;
        service_xlate_id    : 16;
        mapping_xlate_id    : 16;
        tx_meter_id         : 16;
        nexthop_id          : 16;
        pad                 : 5;
        nexthop_type        : 2;
        drop                : 1;
    }
}

header_type apulu_arm_to_p4_header_t {
    fields {
        pad     : 5;
        lif     : 11;
    }
}

header_type apulu_ingress_recirc_header_t {
    fields {
        flow_ohash          : 32;
        local_mapping_ohash : 32;
        pad1                : 6;
        flow_done           : 1;
        local_mapping_done  : 1;
    }
}

header_type apulu_egress_recirc_header_t {
    fields {
        mapping_ohash   : 32;
        pad1            : 6;
        p4_to_arm_valid : 1;
        mapping_done    : 1;
    }
}

header_type apulu_rx_to_tx_header_t {
    fields {
        remote_ip       : 128;// Bytes 0 to 15
        route_base_addr : 40; // Bytes 16 to 20

        sacl_base_addr0 : 40; // Bytes 21 to 25
        sip_classid0    : 10; // Bytes 26[0] to 27[1]
        dip_classid0    : 10; // Bits 27[2] to 28[3]
        pad0            : 4;  // Bits 28[4] to 28[7]
        sport_classid0  : 8;  // Byte 29
        dport_classid0  : 8;  // Byte 30

        sacl_base_addr1 : 40; // Bytes 31 to 35
        sip_classid1    : 10; // Bytes 36[0] to 37[1]
        dip_classid1    : 10; // Bits 37[2] to 38[3]
        pad1            : 4;  // Bits 38[4] to 38[7]
        sport_classid1  : 8;  // Byte 39
        dport_classid1  : 8;  // Byte 40

        sacl_base_addr2 : 40; // Bytes 41 to 45
        sip_classid2    : 10; // Bytes 46[0] to 47[1]
        dip_classid2    : 10; // Bits 47[2] to 48[3]
        pad2            : 4;  // Bits 48[4] to 48[7]
        sport_classid2  : 8;  // Byte 49
        dport_classid2  : 8;  // Byte 50

        sacl_base_addr3 : 40; // Bytes 51 to 55
        sip_classid3    : 10; // Bytes 56[0] to 57[1]
        dip_classid3    : 10; // Bits 57[2] to 58[3]
        pad3            : 4;  // Bits 58[4] to 58[7]
        sport_classid3  : 8;  // Byte 59
        dport_classid3  : 8;  // Byte 60

        pad7            : 24; // Bytes 61 to 63

        /*--------------512b ---------------*/

        sacl_base_addr4 : 40; // Bytes 64 to 68
        sip_classid4    : 10; // Bytes 69[0] to 70[1]
        dip_classid4    : 10; // Bits 70[2] to 71[3]
        pad4            : 4;  // Bits 71[4] to 71[7]
        sport_classid4  : 8;  // Byte 72
        dport_classid4  : 8;  // Byte 73

        sacl_base_addr5 : 40; // Bytes 74 to 78
        sip_classid5    : 10; // Bytes 79[0] to 80[1]
        dip_classid5    : 10; // Bits 80[2] to 81[3]
        pad5            : 4;  // Bits 81[4] to 81[7]
        sport_classid5  : 8;  // Byte 82
        dport_classid5  : 8;  // Byte 83

        vpc_id          : 16; // Bytes 84 and 85
        vnic_id         : 16; // Bytes 86 and 87

        iptype          : 1;  // Bit 88[0]
        rx_packet       : 1;  // Bit 88[1]
        payload_len     : 14; // Bit 88[2] to 89[7]

        dtag_classid    : 8;  // Byte 90
        stag_classid    : 10; // Bits 91[0] to 92[1]
        pad6            : 6;  // Bits 92[2] to 92[7]

        pad8            : 280; // Bytes 93 to 127
    }
}
