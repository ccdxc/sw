header_type artemis_p4_to_rxdma_header_t {
    fields {
        p4plus_app_id   : 4;
        lpm1_enable     : 1;
        lpm2_enable     : 1;
        vnic_info_en    : 1;
        cps_path_en     : 1;

        pad0            : 5;
        aging_enable    : 1;
        iptype          : 1;
        direction       : 1;
        vnic_id         : 8;
        tag_root        : 40;

        vpc_id          : 8;
        pad1            : 16;

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

// When added new fields, please make sure to update
// DMA command in ASM file to include this new field:
// apollo/asm/artemis_rxdma/txdma_enqueue.asm
// And in txdma files where it is DMA'd back
header_type artemis_rx_to_tx_header_t {
    fields {
        remote_ip       : 128;// Bytes 0 to 15
        sacl_base_addr  : 40; // Bytes 16 to 20
        route_base_addr : 40; // Bytes 21 to 25
        meter_result    : 10; // Bytes 26 and 27
        sip_classid     : 10; // Bytes 27 and 28
        dip_classid     : 10; // Bytes 28 and 29
        stag_classid    : 10; // Bytes 29 and 30
        dtag_classid    : 8;  // Byte 31
        sport_classid   : 8;  // Byte 32
        dport_classid   : 8;  // Byte 33
        vpc_id          : 8;  // Byte 34
        vnic_id         : 8;  // Byte 35
        iptype          : 1;  // Byte 36 - 1b
        pad0            : 7;  // Byte 36 - 7b
        // Please check the above comment when adding new fields
    }
}

// Session Info hints that CPA path passes to the FTL Assist programs
// Session Info is derived from the configured policies for the flow
// being investigated
header_type session_info_hint_t {  // Total 272b = 34B
    fields {
        tx_dst_ip        : 128;
        tx_dst_l4port    : 16;  // No need to fill
        nexthop_idx      : 20;

        //tx_rewrite_flags : 8;
        tx_rewrite_flags_unused : 2;
        tx_rewrite_flags_encap  : 1;
        tx_rewrite_flags_dst_ip : 1;
        tx_rewrite_flags_dport  : 1; // Always set to zero
        tx_rewrite_flags_src_ip : 2;
        tx_rewrite_flags_dmac   : 1;  // Always set to rewrite dmac

        //rx_rewrite_flags : 8;
        rx_rewrite_flags_unused : 2;
        rx_rewrite_flags_dst_ip : 2;
        rx_rewrite_flags_sport  : 1; // Always set to zero
        rx_rewrite_flags_src_ip : 2;
        rx_rewrite_flags_smac   : 1;

        tx_policer_idx   : 12;  // TODO-KSM: Not sure what to fill
        rx_policer_idx   : 12;  // TODO-KSM: Not sure what to fill
        meter_idx        : 16;
        timestamp        : 48;  // No need to fill this but kept for easy copy/DMA
        drop             : 1;
        pad0             : 3;
    }
}
