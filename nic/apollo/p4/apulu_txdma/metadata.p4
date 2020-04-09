header_type txdma_predicate_metadata_t {
    fields {
        p4plus_app_id       : 4;
        rfc_enable          : 1;
        pass_two            : 1;
        lpm1_enable         : 1;
        cps_path_en         : 1;
    }
}

header_type txdma_control_metadata_t {
    fields {
        pktdesc_addr1       : 40;
        pktdesc_addr2       : 40;
        rxdma_cindex_addr   : 40;
        rfc_table_addr      : 40;
        lpm1_base_addr      : 40;
        lpm1_next_addr      : 40;
        lpm1_key            : 128;

        payload_addr        : 40;
        cindex              : 16;

        rfc_index           : 20;
        rfc_p1_classid      : 10;
        stag_classid        : 10;
        dtag_classid        : 10;
        recirc_count        : 12;
        dnat_en             : 1;
        st_enable           : 1;

        stag_count          : 4;
        dtag_count          : 4;
        root_count          : 4;
        rule_priority       : 11;
        pad0                : 1;
    }
}

header_type scratch_metadata_t {
    fields {
        field1          :  1;
        field2          :  2;
        field3          :  3;
        field4          :  4;
        field6          :  6;
        field7          :  7;
        field8          :  8;
        field10         : 10;
        field16         : 16;
        field20         : 20;
        field22         : 22;
        field32         : 32;
        field40         : 40;
        field48         : 48;
        field80         : 80;
        field64         : 64;
        field112        : 112;
        field128        : 128;
        field512        : 512;
        meter_result    : 10;
        payload_addr    : 40;
        payload_len     : 14;
        qid             : 24;
        lif             : 11;
        // TMP: Fileds added temporarily
        pad0                : 4;
        flag                : 1;
        hint_valid          : 1;
        mapping_hash        : 11;
        mapping_hint        : 18;
        epoch               : 8;
        flow_hash           : 9;
        flow_hint           : 23;
        session_index       : 23;
        flow_role           : 1;
    }
}

/*
    NOTE: Unfortunately there is no mechanism to enforce that the key formats are the same across
    multiple pipelines, P4I and TXDMA in this case.
    The following key formats have been manually created to match the key generated
    by the Apulu P4 program. Any changes in the keys needs to be reflected here.
*/

// key_ipv4_metadata_t is exactly 160 bits and can be unionized
// with common_t3_s2s
header_type key_ipv4_metadata_t {
    fields {
        flow_ohash      : 32;
        flow_ohash_lkp  : 1;
        _pad0           : 7;
        ipv4_dst        : 32;
        proto           : 8;
        ipv4_src        : 32;
        dport           : 16;
        sport           : 16;
        vpc_id          : 8;

        // I
        epoch           : 8;
    }
}

header_type key_ipv4_metadata_part2_t {
    fields {
        flow_lkp_type   : 8;
        num_flow_lkps   : 8;
    }
}

header_type key_metadata_t {
    fields {
        flow_ohash      : 32;
        flow_ohash_lkp  : 1;
        _pad0           : 7;
        proto           : 8;
        src             : 128;
        dst             : 128;
        dport           : 16;
        sport           : 16;
        vpc_id          : 8;
        ktype           : 2;
        _pad1           : 6;
        _pad2           : 32;

        // I
        epoch           : 8;
        flow_lkp_type   : 8;
        num_flow_lkps   : 8;

        _pad3           : 104;
    }
}

// break key_metadata_t into parts so it can be unionized

// to be unionized with common_t3_s2s (160 bits)
header_type key_metadata_part1_t {
    fields {
        flow_ohash      : 32;
        flow_ohash_lkp  : 1;
        _pad0           : 7;
        proto           : 8;
        src             : 112;
    }
}

// to be unionized with txdma_common_pad (96 bits)
header_type key_metadata_part2_t {
    fields {
        src             : 16;
        dst             : 80;
    }
}

header_type key_metadata_part3_t {
    fields {
        dst             : 48;
        dport           : 16;
        sport           : 16;
        vpc_id          : 8;
        ktype           : 2;
        _pad1           : 6;
        _pad2           : 32;

        // I
        epoch           : 8;
        flow_lkp_type   : 8;
        num_flow_lkps   : 8;

        _pad3           : 104;
    }
}

header_type txdma_to_arm_flow_t {
    fields {
        entry           : 512;
    }
}

// PHV instantiation
@pragma dont_trim
@pragma pa_header_union ingress app_header
metadata txdma_predicate_metadata_t txdma_predicate;

@pragma dont_trim
@pragma pa_header_union ingress to_stage_1
metadata doorbell_data_t    doorbell_data;

@pragma pa_align 512
@pragma dont_trim
metadata apulu_txdma_to_p4e_header_t txdma_to_p4e;

@pragma dont_trim
metadata txdma_control_metadata_t txdma_control;

@pragma pa_align 512
@pragma dont_trim
metadata apulu_rx_to_tx_header_t  rx_to_tx_hdr;

@pragma pa_align 128
@pragma dont_trim
metadata dma_cmd_phv2pkt_t intrinsic_dma;       // dma cmd 1
@pragma dont_trim
metadata dma_cmd_mem2pkt_t payload_dma;         // dma cmd 2
@pragma dont_trim
metadata dma_cmd_phv2mem_t rxdma_ci_update;     // dma cmd 3
@pragma dont_trim
metadata dma_cmd_phv2mem_t doorbell_ci_update;  // dma cmd 4

// Scratch metadata
@pragma scratch_metadata
metadata scratch_metadata_t     scratch_metadata;

@pragma scratch_metadata
metadata qstate_hdr_t           scratch_qstate_hdr;

@pragma scratch_metadata
metadata qstate_info_t          scratch_qstate_info;
