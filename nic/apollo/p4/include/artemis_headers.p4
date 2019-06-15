header_type artemis_predicate_header_t {
    fields {
        pad0            : 7;
        direction       : 1;
    }
}

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

header_type artemis_p4_to_rxdma_header3_t {
    fields {
        flow_hash           : 32;
        parent_hint_index   : 22;
        flow_nrecircs       : 3;
        parent_hint_slot    : 3;
        parent_is_hint      : 1;
        ipaf                : 1;
        pad0                : 2;
        pad1                : 64;
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
        payload_len     : 14; // Byte 36 to 37-6b
        iptype          : 1;  // Byte 37 - 1b
        pad0            : 1;  // Byte 37 - 1b
        // Please check the above comment when adding new fields
    }
}

// Session Info hints that CPA path passes to the FTL Assist programs
// Session Info is derived from the configured policies for the flow
// being investigated
header_type session_info_hint_t {
    fields {
        iflow_tcp_state : 4 ;
        iflow_tcp_seq_num : 32 ;
        iflow_tcp_ack_num : 32 ;
        iflow_tcp_win_sz : 16 ;
        iflow_tcp_win_scale : 4 ;
        rflow_tcp_state : 4 ;
        rflow_tcp_seq_num : 32 ;
        rflow_tcp_ack_num : 32 ;
        rflow_tcp_win_sz : 16 ;
        rflow_tcp_win_scale : 4 ;

        // Fields set by CPS
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
        __pad_to_512b    : 67;
    }
}


// iFlow Info Hints - for now nothing, but later filled by FTL assist program
header_type iflow_info_hint_t {
    fields {
         pad : 512;
    }
}

// rFlow Info Hints - for now nothing, but later filled by FTL assist program
header_type rflow_info_hint_t {
    fields {
         pad : 512;
    }
}

// Packet to arm will contain artemis_txdma_to_arm_session_info_header_t + 
// 2 * artemis_txdma_to_arm_flow_header_t (iflow and rflow)
// In addition p4 adds p4_to_arm_header_t (even in the final approach?)

header_type artemis_txdma_to_arm_session_info_header_t {
    fields {
        action_                 : 8; // add or delete
        packet_len              : 16;
        session_index           : 32;  // delete only
        session_info            : 512; // insert only
    }
}

header_type artemis_txdma_to_arm_flow_header_t {
    fields {
        flow_hash               : 32;
        ipaf                    : 1; // v4 or v6
        parent_is_hint          : 1;
        parent_hint_slot        : 3; // 0 is invalid
                                     // 1-4 = hint slots
                                     // 5 = more
        nrecircs                : 3;
        parent_index            : 32; // parent index if 'parent_is_hint'
        leaf_index              : 32; // for delete
        parent_flow_entry       : 512;
        leaf_flow_entry         : 512;
    }
}

header_type cps_blob_t {
    fields {
        hdr_len         : 8;
        data            : *;
    }
    length : hdr_len;
    max_length : 65535;
}
